#include "ConfigManager.h"
#include <fstream>
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <shlobj.h>
#include <windows.h>
#endif

namespace SerialMonitor {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager() {
    std::string configDir;
    std::string configFile;

#ifdef _WIN32
    char appData[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, appData))) {
        configDir = std::string(appData) + "\\SerialMonitor";
    } else {
        configDir = ".\\config";
    }
#else
    configDir = std::string(getenv("HOME")) + "/.config/SerialMonitor";
#endif

    std::filesystem::create_directories(configDir);
    configPath = configDir + "\\config.json";
}

AppConfig& ConfigManager::getConfig() {
    return config;
}

const AppConfig& ConfigManager::getConfig() const {
    return config;
}

void ConfigManager::setConfig(const AppConfig& cfg) {
    config = cfg;
}

std::string ConfigManager::getConfigPath() const {
    return configPath;
}

bool ConfigManager::load() {
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            std::cout << "No config file found, using defaults: " << configPath << std::endl;
            return false;
        }

        json j;
        file >> j;

        if (j.contains("connection")) {
            const auto& conn = j["connection"];
            config.connection.port = conn.value("port", "COM3");
            config.connection.baudrate = conn.value("baudrate", 115200);
            config.connection.databits = conn.value("databits", 8);
            config.connection.parity = conn.value("parity", "none");
            config.connection.stopbits = conn.value("stopbits", 1);
            config.connection.flowcontrol = conn.value("flowcontrol", "none");
        }

        if (j.contains("display")) {
            const auto& disp = j["display"];
            config.display.format = disp.value("format", "hex");
            config.display.encoding = disp.value("encoding", "utf-8");
            config.display.timestamp = disp.value("timestamp", true);
            config.display.autoscroll = disp.value("autoscroll", true);
        }

        if (j.contains("sending")) {
            const auto& send = j["sending"];
            config.sending.lineEnding = send.value("lineEnding", "crlf");
            config.sending.hexMode = send.value("hexMode", false);
            config.sending.repeatEnabled = send.value("repeatEnabled", false);
            config.sending.repeatInterval = send.value("repeatInterval", 1000);
        }

        if (j.contains("ui")) {
            const auto& ui = j["ui"];
            config.ui.theme = ui.value("theme", "dark-blue");
            config.ui.language = ui.value("language", "zh-CN");
        }

        std::cout << "Config loaded from: " << configPath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load config: " << e.what() << std::endl;
        return false;
    }
}

bool ConfigManager::save() {
    try {
        json j;

        j["connection"] = {
            {"port", config.connection.port},
            {"baudrate", config.connection.baudrate},
            {"databits", config.connection.databits},
            {"parity", config.connection.parity},
            {"stopbits", config.connection.stopbits},
            {"flowcontrol", config.connection.flowcontrol}
        };

        j["display"] = {
            {"format", config.display.format},
            {"encoding", config.display.encoding},
            {"timestamp", config.display.timestamp},
            {"autoscroll", config.display.autoscroll}
        };

        j["sending"] = {
            {"lineEnding", config.sending.lineEnding},
            {"hexMode", config.sending.hexMode},
            {"repeatEnabled", config.sending.repeatEnabled},
            {"repeatInterval", config.sending.repeatInterval}
        };

        j["ui"] = {
            {"theme", config.ui.theme},
            {"language", config.ui.language}
        };

        std::ofstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file for writing: " << configPath << std::endl;
            return false;
        }

        file << j.dump(4);
        std::cout << "Config saved to: " << configPath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to save config: " << e.what() << std::endl;
        return false;
    }
}

}
