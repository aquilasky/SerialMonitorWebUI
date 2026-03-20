#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <atomic>

#include "SerialPort.h"
#include "ConfigManager.h"
#include "WebServer.h"

#ifdef _WIN32
#include <windows.h>
#endif

std::atomic<bool> g_running(true);

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        g_running = false;
    }
}

void printAsciiArt() {
    std::cout << R"(
╔═══════════════════════════════════════════════════════════════════════════════════════════╗
║                                                                                           ║
║   █████╗  ██████╗ ██████╗███████╗███████╗███████╗     ██████╗ ██████╗ ███╗   ███╗██████╗ ║
║  ██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝██╔════╝    ██╔════╝██╔═══██╗████╗ ████║██╔══██╗
║  ███████║██║     ██║     █████╗  ███████╗███████╗    ██║     ██║   ██║██╔████╔██║██║  ██║
║  ██╔══██║██║     ██║     ██╔══╝  ╚════██║╚════██║    ██║     ██║   ██║██║╚██╔╝██║██║  ██║
║  ██║  ██║╚██████╗╚██████╗███████╗███████║███████║    ╚██████╗╚██████╔╝██║ ╚═╝ ██║██████╔╝
║  ╚═╝  ╚═╝ ╚═════╝ ╚═════╝╚══════╝╚══════╝╚══════╝     ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═════╝  ║
║                                                                                           ║
╠═══════════════════════════════════════════════════════════════════════════════════════════╣
║                                                                                           ║
║                    WebUI Serial Port Debug Assistant  v1.0.0                              ║
║                                                                                           ║
║  ┌─────────────────────────────────────────────────────────────────────────────────────┐  ║
║  │                                                                                     │  ║
║  │   ██╗  ██╗  RUNNING                                                                 │  ║
║  │   ██║ ██╔╝                                                                          │  ║
║  │   █████╔╝   WebUI:  http://localhost:8808                                           │  ║
║  │   ██╔═██╗                                                                            │  ║
║  │   ██║  ██╗                                                                           │  ║
║  │   ╚═╝  ╚═╝                                                                           │  ║
║  │                                                                                     │  ║
║  └─────────────────────────────────────────────────────────────────────────────────────┘  ║
║                                                                                           ║
╠═══════════════════════════════════════════════════════════════════════════════════════════╣
║                                                                                           ║
║   Press  Ctrl+C  to stop the server                                                      ║
║   Press  Ctrl+Z  and then Enter to force quit                                            ║
║                                                                                           ║
║   Tips: Open your browser and navigate to http://localhost:8808 to access the WebUI     ║
║                                                                                           ║
╚═══════════════════════════════════════════════════════════════════════════════════════════╝
)" << std::endl;
}

void printShutdownMessage() {
    std::cout << R"(
╔═══════════════════════════════════════════════════════════════════════════════════════════╗
║                                                                                           ║
║   ███████╗███████╗ ██████╗ █████╗ ██████╗ ███████╗██████╗ ██╗     ██╗ ██████╗███████╗    ║
║   ██╔════╝██╔════╝██╔════╝██╔══██╗██╔══██╗██╔════╝██╔══██╗██║     ██║██╔════╝██╔════╝    ║
║   █████╗  ███████╗██║     ███████║██████╔╝█████╗  ██████╔╝██║     ██║██║     █████╗      ║
║   ██╔══╝  ╚════██║██║     ██╔══██║██╔═══╝ ██╔══╝  ██╔═══╝ ██║     ██║██║     ██╔══╝      ║
║   ███████╗███████║╚██████╗██║  ██║██║     ███████╗██║     ███████╗██║╚██████╗███████╗    ║
║   ╚══════╝╚══════╝ ╚═════╝╚═╝  ╚═╝╚═╝     ╚══════╝╚═╝     ╚══════╝╚═╝ ╚═════╝╚══════╝    ║
║                                                                                           ║
║                           Thank you for using SerialMonitor!                              ║
║                                                                                           ║
╚═══════════════════════════════════════════════════════════════════════════════════════════╝
)" << std::endl;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    printAsciiArt();

    std::cout << "[INFO] Loading configuration..." << std::endl;
    SerialMonitor::ConfigManager::getInstance().load();

    auto serialPort = std::make_shared<SerialMonitor::SerialPort>();
    SerialMonitor::WebServer server(8808);
    server.setSerialPort(serialPort);

    std::cout << "[INFO] Starting web server..." << std::endl;
    if (!server.start()) {
        std::cerr << "[ERROR] Failed to start web server!" << std::endl;
        return 1;
    }

    std::cout << "[INFO] SerialMonitor is running!" << std::endl;

    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\n[INFO] Shutting down..." << std::endl;
    SerialMonitor::ConfigManager::getInstance().save();
    server.stop();

    if (serialPort->isOpen()) {
        serialPort->close();
    }

    printShutdownMessage();

    return 0;
}
