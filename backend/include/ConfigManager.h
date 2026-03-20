#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace SerialMonitor {

struct AppConfig {
    struct ConnectionConfig {
        std::string port = "COM3";
        int baudrate = 115200;
        int databits = 8;
        std::string parity = "none";
        int stopbits = 1;
        std::string flowcontrol = "none";
    } connection;

    struct DisplayConfig {
        std::string format = "hex";
        std::string encoding = "utf-8";
        bool timestamp = true;
        bool autoscroll = true;
    } display;

    struct SendingConfig {
        std::string lineEnding = "crlf";
        bool hexMode = false;
        bool repeatEnabled = false;
        int repeatInterval = 1000;
    } sending;

    struct UIConfig {
        std::string theme = "dark-blue";
        std::string language = "zh-CN";
    } ui;
};

class ConfigManager {
public:
    static ConfigManager& getInstance();

    bool load();
    bool save();

    AppConfig& getConfig();
    const AppConfig& getConfig() const;

    void setConfig(const AppConfig& config);

    std::string getConfigPath() const;

private:
    ConfigManager();
    ~ConfigManager() = default;

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    AppConfig config;
    std::string configPath;

    void ensureConfigDirectory();
};

}
