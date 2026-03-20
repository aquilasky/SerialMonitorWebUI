#include "WebServer.h"
#include "SerialPort.h"
#include "ConfigManager.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <iomanip>
#include <fstream>

namespace SerialMonitor {

WebServer* WebServer::instance = nullptr;

int webSocketConnectHandler(const struct mg_connection* conn, void* userData) {
    (void)conn;
    (void)userData;
    return 0;
}

void webSocketReadyHandler(struct mg_connection* conn, void* userData) {
    auto* server = static_cast<WebServer*>(userData);
    if (server) {
        std::lock_guard<std::mutex> lock(server->clientMutex);
        server->clients.push_back(conn);
    }
}

int webSocketDataHandler(struct mg_connection* conn, int bits, char* data, size_t size, void* userData) {
    (void)bits;
    auto* server = static_cast<WebServer*>(userData);
    if (!server || !data) return 0;

    if (size > 0) {
        data[size] = '\0';
        std::string message(data);

        std::stringstream ss(message);
        std::string cmd;
        ss >> cmd;

        std::map<std::string, std::string> params;
        std::string param;
        while (ss >> param) {
            size_t eqPos = param.find('=');
            if (eqPos != std::string::npos) {
                std::string key = param.substr(0, eqPos);
                std::string value = param.substr(eqPos + 1);
                params[key] = value;
            }
        }

        std::string response = server->handleApi(cmd, params);
        std::string wsResponse = "\x81" + response;

        mg_write(conn, wsResponse.c_str(), wsResponse.length());
    }
    return 1;
}

void webSocketCloseHandler(const struct mg_connection* conn, void* userData) {
    auto* server = static_cast<WebServer*>(userData);
    if (server) {
        std::lock_guard<std::mutex> lock(server->clientMutex);
        server->clients.erase(
            std::remove(server->clients.begin(), server->clients.end(), const_cast<struct mg_connection*>(conn)),
            server->clients.end()
        );
    }
}

WebServer::WebServer(int port)
    : ctx(nullptr)
    , serverPort(port)
    , running(false) {
    instance = this;
    memset(&callbacks, 0, sizeof(callbacks));
}

WebServer::~WebServer() {
    stop();
}

bool WebServer::start() {
    const char* options[] = {
        "document_root",
        "./webui",
        "listening_ports",
        std::to_string(serverPort).c_str(),
        "request_timeout_ms",
        "30000",
        "enable_directory_listing",
        "no",
        nullptr
    };

    callbacks.begin_request = [](struct mg_connection* conn) -> int {
        return handleRequest(conn);
    };

    ctx = mg_start(&callbacks, this, options);

    if (!ctx) {
        std::cerr << "Failed to start web server on port " << serverPort << std::endl;
        return false;
    }

    mg_set_websocket_handler(ctx, "/ws",
        webSocketConnectHandler,
        webSocketReadyHandler,
        webSocketDataHandler,
        webSocketCloseHandler,
        this);

    running = true;
    std::cout << "Web server started on port " << serverPort << std::endl;
    return true;
}

void WebServer::stop() {
    if (!running) return;

    running = false;

    if (ctx) {
        mg_stop(ctx);
        ctx = nullptr;
    }

    std::cout << "Web server stopped" << std::endl;
}

void WebServer::setSerialPort(std::shared_ptr<SerialPort> port) {
    serialPort = port;
}

void WebServer::broadcastData(const std::string& data) {
    std::lock_guard<std::mutex> lock(clientMutex);

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    char timeStr[32] = {0};
    struct tm localtmBuf;
    if (localtime_s(&localtmBuf, &time) == 0) {
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &localtmBuf);
    }

    std::string jsonData = "{\"type\":\"data\",\"timestamp\":\"" + std::string(timeStr) + "\",\"data\":\"";

    for (char c : data) {
        if (c == '"') jsonData += "\\\"";
        else if (c == '\\') jsonData += "\\\\";
        else if (c == '\n') jsonData += "\\n";
        else if (c == '\r') jsonData += "\\r";
        else jsonData += c;
    }
    jsonData += "\"}";

    std::string wsData = "\x81" + jsonData;

    for (auto conn : clients) {
        mg_write(conn, wsData.c_str(), wsData.length());
    }
}

int WebServer::handleRequest(struct mg_connection* conn) {
    const mg_request_info* req = mg_get_request_info(conn);
    if (!req) return 0;

    WebServer* server = static_cast<WebServer*>(req->user_data);
    if (!server) return 0;

    std::string uri(req->request_uri ? req->request_uri : "/");

    if (uri == "/" || uri == "/index.html") {
        std::string indexContent = server->serveIndex();
        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n",
            indexContent.size());
        mg_write(conn, indexContent.c_str(), indexContent.size());
        return 1;
    }

    std::string apiPath;
    std::map<std::string, std::string> params;
    if (server->isApiRequest(uri, apiPath, params)) {
        std::string response = server->handleApi(apiPath, params);
        std::string jsonResponse = "{\"success\":true,\"data\":" + response + "}";

        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n",
            jsonResponse.size());
        mg_write(conn, jsonResponse.c_str(), jsonResponse.size());
        return 1;
    }

    std::string staticContent = server->serveStatic(uri);
    if (!staticContent.empty()) {
        std::string contentType = "application/octet-stream";
        if (endsWith(uri, ".html")) contentType = "text/html; charset=utf-8";
        else if (endsWith(uri, ".css")) contentType = "text/css; charset=utf-8";
        else if (endsWith(uri, ".js")) contentType = "application/javascript; charset=utf-8";
        else if (endsWith(uri, ".json")) contentType = "application/json";
        else if (endsWith(uri, ".png")) contentType = "image/png";
        else if (endsWith(uri, ".ico")) contentType = "image/x-icon";

        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n",
            contentType.c_str(), staticContent.size());
        mg_write(conn, staticContent.c_str(), staticContent.size());
        return 1;
    }

    std::string notFound = "{\"error\":\"Not found\"}";
    mg_printf(conn,
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n\r\n",
        notFound.size());
    mg_write(conn, notFound.c_str(), notFound.size());
    return 1;
}

bool WebServer::endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() >= suffix.length()) {
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    }
    return false;
}

bool WebServer::isApiRequest(const std::string& uri, std::string& path, std::map<std::string, std::string>& params) {
    if (uri.rfind("/api/", 0) == 0) {
        path = uri.substr(5);
        size_t queryStart = path.find('?');
        if (queryStart != std::string::npos) {
            std::string query = path.substr(queryStart + 1);
            path = path.substr(0, queryStart);

            std::stringstream ss(query);
            std::string pair;
            while (std::getline(ss, pair, '&')) {
                size_t eqPos = pair.find('=');
                if (eqPos != std::string::npos) {
                    std::string key = urlDecode(pair.substr(0, eqPos));
                    std::string value = urlDecode(pair.substr(eqPos + 1));
                    params[key] = value;
                }
            }
        }
        return true;
    }
    return false;
}

std::string WebServer::handleApi(const std::string& path, const std::map<std::string, std::string>& params) {
    if (path == "ports") {
        return apiListPorts();
    } else if (path == "connect") {
        return apiConnect(params);
    } else if (path == "disconnect") {
        return apiDisconnect();
    } else if (path == "send") {
        return apiSend(params);
    } else if (path == "config") {
        if (params.empty()) {
            return apiGetConfig();
        } else {
            return apiSaveConfig(params);
        }
    } else if (path == "status") {
        return apiGetStatus();
    }
    return "{\"error\":\"Unknown command\"}";
}

std::string WebServer::apiListPorts() {
    auto ports = SerialPort::listPorts();
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < ports.size(); ++i) {
        if (i > 0) ss << ",";
        ss << "{\"name\":\"" << ports[i].name << "\","
           << "\"description\":\"" << ports[i].description << "\","
           << "\"hardwareId\":\"" << ports[i].hardwareId << "\"}";
    }
    ss << "]";
    return ss.str();
}

std::string WebServer::apiConnect(const std::map<std::string, std::string>& params) {
    if (!serialPort) {
        return "{\"success\":false,\"error\":\"Serial port not initialized\"}";
    }

    SerialConfig config;
    config.port = params.at("port");
    config.baudrate = std::stoi(params.at("baudrate"));
    config.dataBits = std::stoi(params.at("databits"));
    config.parity = params.at("parity") == "odd" ? Parity::Odd :
                     params.at("parity") == "even" ? Parity::Even : Parity::None;
    config.stopBits = params.at("stopbits") == "2" ? StopBits::Two : StopBits::One;
    config.flowControl = params.at("flowcontrol") == "hardware" ? FlowControl::Hardware :
                         params.at("flowcontrol") == "software" ? FlowControl::Software : FlowControl::None;

    auto self = shared_from_this();
    serialPort->setDataReceivedCallback([this, self](const uint8_t* data, size_t length) {
        std::string str(reinterpret_cast<const char*>(data), length);
        broadcastData(str);
    });

    if (serialPort->open(config)) {
        return "{\"success\":true}";
    } else {
        return "{\"success\":false,\"error\":\"Failed to open port\"}";
    }
}

std::string WebServer::apiDisconnect() {
    if (serialPort) {
        serialPort->close();
    }
    return "{\"success\":true}";
}

std::string WebServer::apiSend(const std::map<std::string, std::string>& params) {
    if (!serialPort || !serialPort->isOpen()) {
        return "{\"success\":false,\"error\":\"Port not connected\"}";
    }

    std::string data = params.at("data");
    bool hexMode = params.at("hexMode") == "true";

    if (hexMode) {
        std::vector<uint8_t> bytes;
        std::stringstream ss(data);
        std::string byteStr;
        while (ss >> byteStr) {
            try {
                bytes.push_back(static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16)));
            } catch (...) {}
        }
        if (!bytes.empty()) {
            serialPort->write(bytes.data(), bytes.size());
        }
    } else {
        serialPort->write(data);
    }

    return "{\"success\":true}";
}

std::string WebServer::apiGetConfig() {
    auto& cfg = ConfigManager::getInstance().getConfig();
    std::stringstream ss;
    ss << "{"
       << "\"port\":\"" << cfg.connection.port << "\","
       << "\"baudrate\":" << cfg.connection.baudrate << ","
       << "\"databits\":" << cfg.connection.databits << ","
       << "\"parity\":\"" << cfg.connection.parity << "\","
       << "\"stopbits\":" << cfg.connection.stopbits << ","
       << "\"flowcontrol\":\"" << cfg.connection.flowcontrol << "\","
       << "\"format\":\"" << cfg.display.format << "\","
       << "\"encoding\":\"" << cfg.display.encoding << "\","
       << "\"timestamp\":" << (cfg.display.timestamp ? "true" : "false") << ","
       << "\"autoscroll\":" << (cfg.display.autoscroll ? "true" : "false") << ","
       << "\"lineEnding\":\"" << cfg.sending.lineEnding << "\","
       << "\"hexMode\":" << (cfg.sending.hexMode ? "true" : "false") << ","
       << "\"repeatEnabled\":" << (cfg.sending.repeatEnabled ? "true" : "false") << ","
       << "\"repeatInterval\":" << cfg.sending.repeatInterval << ","
       << "\"theme\":\"" << cfg.ui.theme << "\","
       << "\"language\":\"" << cfg.ui.language << "\""
       << "}";
    return ss.str();
}

std::string WebServer::apiSaveConfig(const std::map<std::string, std::string>& params) {
    auto& cfg = ConfigManager::getInstance().getConfig();

    if (params.count("port")) cfg.connection.port = params.at("port");
    if (params.count("baudrate")) cfg.connection.baudrate = std::stoi(params.at("baudrate"));
    if (params.count("databits")) cfg.connection.databits = std::stoi(params.at("databits"));
    if (params.count("parity")) cfg.connection.parity = params.at("parity");
    if (params.count("stopbits")) cfg.connection.stopbits = std::stoi(params.at("stopbits"));
    if (params.count("flowcontrol")) cfg.connection.flowcontrol = params.at("flowcontrol");
    if (params.count("format")) cfg.display.format = params.at("format");
    if (params.count("encoding")) cfg.display.encoding = params.at("encoding");
    if (params.count("timestamp")) cfg.display.timestamp = params.at("timestamp") == "true";
    if (params.count("autoscroll")) cfg.display.autoscroll = params.at("autoscroll") == "true";
    if (params.count("lineEnding")) cfg.sending.lineEnding = params.at("lineEnding");
    if (params.count("hexMode")) cfg.sending.hexMode = params.at("hexMode") == "true";
    if (params.count("repeatEnabled")) cfg.sending.repeatEnabled = params.at("repeatEnabled") == "true";
    if (params.count("repeatInterval")) cfg.sending.repeatInterval = std::stoi(params.at("repeatInterval"));
    if (params.count("theme")) cfg.ui.theme = params.at("theme");
    if (params.count("language")) cfg.ui.language = params.at("language");

    ConfigManager::getInstance().save();
    return "{\"success\":true}";
}

std::string WebServer::apiGetStatus() {
    bool connected = serialPort && serialPort->isOpen();
    std::stringstream ss;
    ss << "{"
       << "\"connected\":" << (connected ? "true" : "false")
       << "}";
    return ss.str();
}

std::string WebServer::serveIndex() {
    std::ifstream file("./webui/index.html", std::ios::binary);
    if (file.is_open()) {
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    return R"(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SerialMonitor</title>
    <style>
        body { font-family: Arial, sans-serif; background: #1a1d29; color: #e8eaed; margin: 0; padding: 20px; text-align: center; }
        h1 { color: #4a9eff; }
        .container { max-width: 600px; margin: 50px auto; padding: 30px; background: #242836; border-radius: 12px; }
        .info { margin: 20px 0; padding: 15px; background: #2d3348; border-radius: 8px; }
        a { color: #4a9eff; }
    </style>
</head>
<body>
    <div class="container">
        <h1>SerialMonitor</h1>
        <div class="info">
            <p><strong>WebUI:</strong> <a href="http://localhost:8808">http://localhost:8808</a></p>
            <p><strong>Status:</strong> Running</p>
        </div>
        <p>Please visit the address above to use the serial monitor</p>
    </div>
</body>
</html>)";
}

std::string WebServer::serveStatic(const std::string& path) {
    std::string filePath = "./webui" + path;

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return "";
    }

    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string content(size, '\0');
    file.read(content.data(), size);

    return content;
}

std::string WebServer::urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%' && i + 2 < str.size()) {
            std::string hex = str.substr(i + 1, 2);
            char c = static_cast<char>(std::stoi(hex, nullptr, 16));
            result += c;
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

}
