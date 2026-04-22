#pragma once
#include "ofMain.h"
#include <entt.hpp>
#include <functional>

// ============================================================================
// HARDWARE INTEGRATION COMPONENTS
// ============================================================================

namespace ecs {

// ============================================================================
// Serial Component (Arduino, sensors, etc.)
// ============================================================================

struct serial_component {
    ofSerial serial;
    
    std::string devicePath;
    int baudRate;
    
    std::vector<unsigned char> readBuffer;
    std::vector<unsigned char> writeBuffer;
    
    bool connected;
    bool autoReconnect;
    float reconnectInterval;
    float timeSinceLastAttempt;
    
    std::function<void(const std::vector<unsigned char>&)> onDataReceived;
    
    serial_component()
        : devicePath("")
        , baudRate(9600)
        , connected(false)
        , autoReconnect(false)
        , reconnectInterval(5.0f)
        , timeSinceLastAttempt(0.0f)
    {}
    
    // IO operations are handled by HardwareIOSystem
    // Use HardwareIOSystem::connectSerial, sendSerial, etc.
    
    // Helper to list available devices (doesn't require system)
    std::vector<std::string> listDevices() {
        std::vector<std::string> devices;
        auto deviceList = serial.getDeviceList();
        for (auto& device : deviceList) {
            devices.push_back(device.getDevicePath());
        }
        return devices;
    }
    
    bool isConnected() const { return connected; }
    int available() { return connected ? serial.available() : 0; }
};

// ============================================================================
// OSC Component (Network communication)
// ============================================================================

// Note: This requires ofxOsc addon
// If ofxOsc is not available, this component will be a stub

struct osc_component {
    std::string sendHost;
    int sendPort;
    int receivePort;
    
    bool sendEnabled;
    bool receiveEnabled;
    
    // Message queue for sending
    struct OSCMessage {
        std::string address;
        enum Type { FLOAT, INT, STRING } type;
        float floatValue;
        int intValue;
        std::string stringValue;
    };
    
    std::vector<OSCMessage> messageQueue;
    
    osc_component()
        : sendHost("localhost")
        , sendPort(8000)
        , receivePort(8001)
        , sendEnabled(false)
        , receiveEnabled(false)
    {}
    
    // IO operations are handled by HardwareIOSystem
    // Use HardwareIOSystem::setupOSCSender, sendOSCFloat, etc.
};


} // namespace ecs
