#pragma once

#include <string>
#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <atomic>
#include <vector>
#include <civetweb.h>

namespace SerialMonitor {

class SerialPort;

class WebServer : public std::enable_shared_from_this<WebServer> {
public:
    using RequestHandler = std::function<std::string(const std::map<std::string, std::string>& params)>;

    WebServer(int port = 8808);
    ~WebServer();

    bool start();
    void stop();

    void setSerialPort(std::shared_ptr<SerialPort> port);

    void broadcastData(const std::string& data);

private:
    struct mg_context* ctx;
    struct mg_callbacks callbacks;
    int serverPort;
    std::atomic<bool> running;
    std::shared_ptr<SerialPort> serialPort;

    std::mutex clientMutex;
    std::vector<struct mg_connection*> clients;

    static WebServer* instance;

    static int handleRequest(struct mg_connection* conn);

    std::string handleApi(const std::string& path, const std::map<std::string, std::string>& params);

    std::string apiListPorts();
    std::string apiConnect(const std::map<std::string, std::string>& params);
    std::string apiDisconnect();
    std::string apiSend(const std::map<std::string, std::string>& params);
    std::string apiGetConfig();
    std::string apiSaveConfig(const std::map<std::string, std::string>& params);
    std::string apiGetStatus();

    std::string serveIndex();
    std::string serveStatic(const std::string& path);

    bool isApiRequest(const std::string& uri, std::string& path, std::map<std::string, std::string>& params);

    static std::string urlDecode(const std::string& str);
    static bool endsWith(const std::string& str, const std::string& suffix);

    friend int webSocketConnectHandler(const struct mg_connection* conn, void* userData);
    friend void webSocketReadyHandler(struct mg_connection* conn, void* userData);
    friend int webSocketDataHandler(struct mg_connection* conn, int bits, char* data, size_t size, void* userData);
    friend void webSocketCloseHandler(const struct mg_connection* conn, void* userData);
};

int webSocketConnectHandler(const struct mg_connection* conn, void* userData);
void webSocketReadyHandler(struct mg_connection* conn, void* userData);
int webSocketDataHandler(struct mg_connection* conn, int bits, char* data, size_t size, void* userData);
void webSocketCloseHandler(const struct mg_connection* conn, void* userData);

}
