# SerialMonitor

A powerful WebUI-based serial port debug assistant built with C++ backend and Vue 3 frontend.

![SerialMonitor](https://img.shields.io/badge/Version-1.0.0-blue)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-yellow)
![License](https://img.shields.io/badge/License-MIT-green)

## Features

### Core Functionality
- **Serial Port Communication**: Full-featured serial port support with configurable parameters
- **Port Selection**: Auto-scan and manual selection of available COM ports
- **Baud Rate Selection**: Support for standard rates (300 to 921600) plus custom rates
- **Data Format Display**: Multiple formats - ASCII, Hex, Decimal, Binary, Mixed
- **Real-time Data Monitoring**: Live data streaming via WebSocket
- **Configuration Persistence**: Settings saved automatically to local JSON file

### Connection Options
- Data Bits: 5, 6, 7, 8
- Parity: None, Odd, Even, Mark, Space
- Stop Bits: 1, 1.5, 2
- Flow Control: None, Hardware (RTS/CTS), Software (XON/XOFF)
- Line Endings: CR, LF, CR+LF, None

### UI/UX
- **6 Color Themes**: Dark Blue, Carbon Black, Matrix Green, Daylight White, Violet, Sunset Orange
- **Smooth Animations**: Elegant transitions (150-300ms)
- **Responsive Layout**: Optimized for various screen sizes
- **Web-based Interface**: Access from any browser

### Advanced Features
- **Data History**: Quick command shortcuts
- **Auto Reconnection**: Automatic reconnection on connection loss
- **Timestamp Display**: Optional time stamping for received data
- **Auto Scroll**: Configurable automatic scrolling

## Quick Start

### Download Release

Download the latest release from [Releases](https://github.com/your-repo/SerialMonitor/releases):

```
SerialMonitor.exe
```

Double-click to run. The application will start with ASCII art splash and display:

```
WebUI: http://localhost:8808
Press Ctrl+C to stop the server
```

Open your browser and navigate to `http://localhost:8080` to use the application.

## Build from Source

### Prerequisites

#### Windows
- **Visual Studio 2019+** or **MinGW-w64**
- **CMake 3.16+**
- **Node.js 18+** (for frontend development)

#### Dependencies
- Boost.Asio (for serial port communication)
- CivetWeb (embedded HTTP server)
- nlohmann/json (JSON parsing)

### Build Steps

#### 1. Clone the Repository

```powershell
git clone https://github.com/your-repo/SerialMonitor.git
cd SerialMonitor
```

#### 2. Download Third-Party Libraries

Download and extract the following to `thirdparty/` directory:

- **CivetWeb**: https://github.com/civetweb/civetweb/releases (v1.16)
- **Boost**: https://boost.org (1.70+) or use vcpkg

#### 3. Build Backend (CMake)

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

Or with MinGW:

```powershell
cmake .. -G "MinGW Makefiles"
mingw32-make
```

#### 4. Build Frontend (Optional)

If you want to develop the frontend:

```powershell
cd frontend
npm install
npm run build
```

The built files will be in `frontend/dist/`.

#### 5. Copy Frontend to Build Output

```powershell
mkdir build/bin/webui
copy frontend\dist\* build\bin\webui\
```

### Directory Structure

```
SerialMonitor/
├── CMakeLists.txt
├── SPEC.md
├── README.md
├── backend/
│   ├── include/
│   │   ├── SerialPort.h
│   │   ├── ConfigManager.h
│   │   └── WebServer.h
│   └── src/
│       ├── main.cpp
│       ├── SerialPort.cpp
│       ├── ConfigManager.cpp
│       └── WebServer.cpp
├── frontend/
│   ├── src/
│   │   ├── main.ts
│   │   ├── App.vue
│   │   ├── stores/
│   │   └── components/
│   └── dist/
├── thirdparty/
│   ├── civetweb/
│   └── boost/
└── docs/
    └── DEVELOPMENT_TUTORIAL.md
```

## Usage

### Starting the Application

Double-click `SerialMonitor.exe` or run from command line:

```powershell
.\SerialMonitor.exe
```

### Startup Screen

The application displays an ASCII art splash screen:

```
╔═══════════════════════════════════════════════════════════════╗
║   █████╗  ██████╗ ██████╗███████╗███████╗███████╗             ║
║  ██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝██╔════╝             ║
║  ███████║██║     ██║     █████╗  ███████╗███████╗             ║
║  ██╔══██║██║     ██║     ██╔══╝  ╚════██║╚════██║             ║
║  ██║  ██║╚██████╗╚██████╗███████╗███████║███████║             ║
║  ╚═╝  ╚═╝ ╚═════╝ ╚═════╝╚══════╝╚══════╝╚══════╝             ║
║                                                               ║
║            WebUI Serial Port Debug Assistant                 ║
║                      Version 1.0.0                            ║
║                                                               ║
║  ┌─────────────────────────────────────────────────────────┐  ║
║  │  Status: RUNNING                                        │  ║
║  │  WebUI: http://localhost:8080                          │  ║
║  │  Press Ctrl+C to stop server                           │  ║
║  └─────────────────────────────────────────────────────────┘  ║
╚═══════════════════════════════════════════════════════════════╝
```

### Accessing WebUI

1. Open any modern web browser
2. Navigate to `http://localhost:8080`
3. The WebUI will load with the default theme

### Connecting to a Serial Port

1. Click **Refresh** to scan for available ports
2. Select your target port from the dropdown
3. Configure baud rate and other parameters
4. Click **Connect**
5. Status indicator will turn green when connected

### Sending Data

1. Enter text in the send input area
2. Select line ending format (CR+LF, CR, LF, None)
3. Optionally enable Hex mode
4. Click **Send** or press Enter

### Theme Switching

Click any theme color in the header to instantly switch themes. Themes include:
- Dark Blue (default)
- Carbon Black
- Matrix Green
- Daylight White
- Violet
- Sunset Orange

### Stopping the Application

Press `Ctrl+C` in the console window to gracefully stop the server.

## Configuration

Configuration is automatically saved to:
- **Windows**: `%LOCALAPPDATA%\SerialMonitor\config.json`
- **Linux**: `~/.config/SerialMonitor/config.json`

### Config File Structure

```json
{
  "connection": {
    "port": "COM3",
    "baudrate": 115200,
    "databits": 8,
    "parity": "none",
    "stopbits": 1,
    "flowcontrol": "none"
  },
  "display": {
    "format": "hex",
    "encoding": "utf-8",
    "timestamp": true,
    "autoscroll": true
  },
  "sending": {
    "lineEnding": "crlf",
    "hexMode": false
  },
  "ui": {
    "theme": "dark-blue",
    "language": "zh-CN"
  }
}
```

## API Reference

The backend provides REST API endpoints:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/ports` | GET | List available serial ports |
| `/api/connect` | GET | Connect to a port with query params |
| `/api/disconnect` | GET | Disconnect from current port |
| `/api/send` | GET | Send data to the port |
| `/api/config` | GET/POST | Get or save configuration |
| `/api/status` | GET | Get connection status |

### WebSocket

Connect to `/ws` for real-time data streaming. Messages are JSON:

```json
{
  "type": "data",
  "timestamp": "14:32:05",
  "data": "Hello World"
}
```

## Troubleshooting

### Port Not Found

1. Ensure the device is connected and drivers are installed
2. Click **Refresh** to rescan ports
3. Check if another application is using the port

### Connection Failed

1. Verify the port is not in use by another application
2. Check hardware connections
3. Try a different baud rate

### WebUI Not Loading

1. Ensure the application is running (console window should be visible)
2. Check the port 8080 is not blocked by firewall
3. Try `http://127.0.0.1:8080` instead

## Development

### Frontend Development Setup

See [DEVELOPMENT_TUTORIAL.md](docs/DEVELOPMENT_TUTORIAL.md) for detailed instructions.

### Project Architecture

```
┌─────────────────────────────────────────────────┐
│                 C++ Backend                     │
│  ┌──────────┐  ┌──────────┐  ┌──────────────┐  │
│  │ Serial   │  │  Config  │  │ Web Server    │  │
│  │ Port     │  │ Manager  │  │ (CivetWeb)    │  │
│  └──────────┘  └──────────┘  └──────────────┘  │
└─────────────────────┬───────────────────────────┘
                      │ HTTP/WebSocket
┌─────────────────────┴───────────────────────────┐
│                 Vue 3 Frontend                   │
│  ┌──────────┐  ┌──────────┐  ┌──────────────┐  │
│  │ Pinia    │  │ Element  │  │ SCSS Themes   │  │
│  │ Store    │  │ Plus     │  │               │  │
│  └──────────┘  └──────────┘  └──────────────┘  │
└─────────────────────────────────────────────────┘
```

## License

MIT License

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.
