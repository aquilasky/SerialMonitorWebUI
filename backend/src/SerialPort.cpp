#include "SerialPort.h"
#include <iostream>
#include <algorithm>
#include <cstring>

namespace SerialMonitor {

SerialPort::SerialPort()
    : isOpen_(false)
    , closing_(false) {
    readBuffer.resize(READ_BUFFER_SIZE);
}

SerialPort::~SerialPort() {
    close();
}

bool SerialPort::open(const SerialConfig& config) {
    if (isOpen_) {
        close();
    }

    try {
        port = std::make_unique<boost::asio::serial_port>(ioContext);

        port->open(config.port);

        boost::asio::serial_port_base::baud_rate baudRate(config.baudrate);
        port->set_option(baudRate);

        boost::asio::serial_port_base::character_size charSize(static_cast<unsigned int>(config.dataBits));
        port->set_option(charSize);

        if (config.parity == Parity::Odd) {
            boost::asio::serial_port_base::parity parity(boost::asio::serial_port_base::parity::odd);
            port->set_option(parity);
        } else if (config.parity == Parity::Even) {
            boost::asio::serial_port_base::parity parity(boost::asio::serial_port_base::parity::even);
            port->set_option(parity);
        }

        if (config.stopBits == StopBits::Two) {
            boost::asio::serial_port_base::stop_bits stopBits(boost::asio::serial_port_base::stop_bits::two);
            port->set_option(stopBits);
        } else if (config.stopBits == StopBits::OnePointFive) {
            boost::asio::serial_port_base::stop_bits stopBits(boost::asio::serial_port_base::stop_bits::onepointfive);
            port->set_option(stopBits);
        }

        if (config.flowControl == FlowControl::Hardware) {
            boost::asio::serial_port_base::flow_control flow(boost::asio::serial_port_base::flow_control::hardware);
            port->set_option(flow);
        } else if (config.flowControl == FlowControl::Software) {
            boost::asio::serial_port_base::flow_control flow(boost::asio::serial_port_base::flow_control::software);
            port->set_option(flow);
        }

        isOpen_ = true;
        closing_ = false;

        startRead();

        std::thread([this]() {
            ioContext.run();
        }).detach();

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to open serial port: " << e.what() << std::endl;
        port.reset();
        isOpen_ = false;
        return false;
    }
}

void SerialPort::close() {
    if (!isOpen_) return;

    closing_ = true;

    try {
        if (port && port->is_open()) {
            port->cancel();
            port->close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error closing port: " << e.what() << std::endl;
    }

    port.reset();
    isOpen_ = false;
}

bool SerialPort::isOpen() const {
    return isOpen_ && port && port->is_open();
}

bool SerialPort::write(const uint8_t* data, size_t length) {
    if (!isOpen()) return false;

    try {
        boost::asio::write(*port, boost::asio::buffer(data, length));
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Write error: " << e.what() << std::endl;
        return false;
    }
}

bool SerialPort::write(const std::string& data) {
    return write(reinterpret_cast<const uint8_t*>(data.data()), data.size());
}

void SerialPort::setDataReceivedCallback(DataReceivedCallback callback) {
    dataCallback = std::move(callback);
}

void SerialPort::startRead() {
    if (!isOpen() || closing_) return;
    doRead();
}

void SerialPort::doRead() {
    if (!isOpen() || closing_) return;

    auto self = shared_from_this();
    port->async_read_some(
        boost::asio::buffer(readBuffer.data(), READ_BUFFER_SIZE),
        [this, self](const boost::system::error_code& ec, size_t bytesTransferred) {
            if (ec) {
                if (!closing_) {
                    std::cerr << "Read error: " << ec.message() << std::endl;
                }
                return;
            }

            if (dataCallback && bytesTransferred > 0) {
                std::lock_guard<std::mutex> lock(readMutex);
                dataCallback(readBuffer.data(), bytesTransferred);
            }

            if (!closing_) {
                doRead();
            }
        }
    );
}

std::vector<PortInfo> SerialPort::listPorts() {
    std::vector<PortInfo> ports;

#ifdef _WIN32
    for (int i = 1; i <= 256; ++i) {
        std::string portName = "COM" + std::to_string(i);

        HANDLE hPort = CreateFileA(
            portName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
        );

        if (hPort != INVALID_HANDLE_VALUE) {
            PortInfo info;
            info.name = portName;
            info.description = "Serial Port";

            CloseHandle(hPort);
            ports.push_back(info);
        }
    }
#endif

    return ports;
}

}
