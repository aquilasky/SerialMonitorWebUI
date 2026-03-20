# SerialMonitor - CMake Build Configuration
#
# Prerequisites:
# 1. Boost 1.86 installed at C:\local\boost_1_86_0
# 2. CivetWeb source at thirdparty/civetweb
#
# Build Steps:
#   mkdir build
#   cd build
#   cmake .. -G "Visual Studio 17 2022" -A x64
#   cmake --build . --config Release

cmake_minimum_required(VERSION 3.16)
project(SerialMonitor VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_FRONTEND "Build frontend assets" OFF)

if(MSVC)
    add_compile_options(/W4 /utf-8)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DNO_SSL)
    add_definitions(-DNO_CACHING)
    add_definitions(-DNO_CGI)
else()
    add_compile_options(-Wall -Wextra)
    add_definitions(-DNO_SSL)
    add_definitions(-DNO_CACHING)
    add_definitions(-DNO_CGI)
endif()

# Find Boost from installed location
set(BOOST_ROOT "C:/local/boost_1_86_0")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
set(BOOST_ROOT "${BOOST_ROOT}" CACHE PATH "Boost installation root")
mark_as_advanced(BOOST_ROOT)

find_package(Boost 1.70 REQUIRED COMPONENTS system thread)
if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
endif()

find_package(Threads REQUIRED)

# CivetWeb configuration
set(CIVETWEB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/civetweb")
set(CIVETWEB_INCLUDE_DIR "${CIVETWEB_ROOT}/include")
set(CIVETWEB_SOURCE "${CIVETWEB_ROOT}/src/civetweb.c")

if(NOT EXISTS "${CIVETWEB_INCLUDE_DIR}/civetweb.h")
    message(FATAL_ERROR "CivetWeb header not found at ${CIVETWEB_INCLUDE_DIR}/civetweb.h")
endif()

if(NOT EXISTS "${CIVETWEB_SOURCE}")
    message(FATAL_ERROR "CivetWeb source not found at ${CIVETWEB_SOURCE}")
endif()

message(STATUS "CivetWeb found at ${CIVETWEB_ROOT}")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/include
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nlohmann/include
    ${Boost_INCLUDE_DIRS}
    ${CIVETWEB_INCLUDE_DIR}
)

set(BACKEND_SOURCES
    backend/src/main.cpp
    backend/src/SerialPort.cpp
    backend/src/ConfigManager.cpp
    backend/src/WebServer.cpp
)

add_executable(SerialMonitor ${BACKEND_SOURCES} ${CIVETWEB_SOURCE})

target_link_libraries(SerialMonitor
    ${Boost_LIBRARIES}
    Threads::Threads
    ws2_32
    winmm
)

if(BUILD_FRONTEND)
    add_custom_command(TARGET SerialMonitor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/frontend/dist/"
        "$<TARGET_FILE_DIR:SerialMonitor>/webui/"
    )
endif()

set_target_properties(SerialMonitor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS SerialMonitor DESTINATION bin)
