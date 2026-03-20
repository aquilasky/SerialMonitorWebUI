#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <atomic>
#include <mutex>
#include <boost/asio.hpp>

namespace SerialMonitor {

enum class Parity {
    None,
    Odd,
    Even,
    Mark,
    Space
};

enum class StopBits {
    One,
    OnePointFive,
    Two
};

enum class FlowControl {
    None,
    Hardware,
    Software
};

struct SerialConfig {
    std::string port;
    int baudrate = 115200;
    int dataBits = 8;
    Parity parity = Parity::None;
    StopBits stopBits = StopBits::One;
    FlowControl flowControl = FlowControl::None;
};

struct PortInfo {
    std::string name;
    std::string description;
    std::string hardwareId;
};

class SerialPort : public std::enable_shared_from_this<SerialPort> {
public:
    using DataReceivedCallback = std::function<void(const uint8_t* data, size_t length)>;

    SerialPort();
    ~SerialPort();

    bool open(const SerialConfig& config);
    void close();
    bool isOpen() const;

    bool write(const uint8_t* data, size_t length);
    bool write(const std::string& data);

    void setDataReceivedCallback(DataReceivedCallback callback);

    static std::vector<PortInfo> listPorts();

private:
    void startRead();
    void doRead();

    boost::asio::io_context ioContext;
    std::unique_ptr<boost::asio::serial_port> port;
    std::atomic<bool> isOpen_;
    std::atomic<bool> closing_;

    std::vector<uint8_t> readBuffer;
    std::mutex readMutex;

    DataReceivedCallback dataCallback;

    static constexpr size_t READ_BUFFER_SIZE = 4096;
};

}
