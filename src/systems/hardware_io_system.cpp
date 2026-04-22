#include "hardware_io_system.h"

namespace ecs {
using namespace ecs;

void HardwareIOSystem::update(entt::registry& registry, float deltaTime) {
    // Update serial components
	auto serials = registry.view<serial_component>();
    for (auto entity : serials) {
		auto & serial = registry.get<serial_component>(entity);
        updateSerial(serial, deltaTime);
    }
    
    // Update OSC components
	auto oscs = registry.view<osc_component>();
    for (auto entity : oscs) {
		auto & osc = registry.get<osc_component>(entity);
        updateOSC(osc);
    }
}

// ============================================================================
// Serial Functions
// ============================================================================

void HardwareIOSystem::updateSerial(serial_component & comp, float dt) {
    if (!comp.connected && comp.autoReconnect) {
        comp.timeSinceLastAttempt += dt;
        if (comp.timeSinceLastAttempt >= comp.reconnectInterval) {
            ofLogNotice("HardwareIOSystem") << "Attempting to reconnect serial...";
            connectSerial(comp);
            comp.timeSinceLastAttempt = 0.0f;
        }
    }
    
    if (comp.connected) {
        auto data = receiveSerial(comp);
        if (!data.empty() && comp.onDataReceived) {
            comp.onDataReceived(data);
        }
    }
}

bool HardwareIOSystem::connectSerial(serial_component & comp, const std::string & device, int baud) {
    if (!device.empty()) comp.devicePath = device;
    if (baud > 0) comp.baudRate = baud;
    
    if (comp.devicePath.empty()) {
        ofLogError("HardwareIOSystem") << "No device path specified";
        return false;
    }
    
    comp.connected = comp.serial.setup(comp.devicePath, comp.baudRate);
    if (comp.connected) {
        ofLogNotice("HardwareIOSystem") << "Connected to " << comp.devicePath << " @ " << comp.baudRate;
    } else {
        ofLogError("HardwareIOSystem") << "Failed to connect to " << comp.devicePath;
    }
    
    return comp.connected;
}

void HardwareIOSystem::disconnectSerial(serial_component & comp) {
    if (comp.connected) {
        comp.serial.close();
        comp.connected = false;
        ofLogNotice("HardwareIOSystem") << "Disconnected from " << comp.devicePath;
    }
}

void HardwareIOSystem::sendSerial(serial_component & comp, const std::vector<unsigned char> & data) {
    if (!comp.connected) return;
    
    for (unsigned char byte : data) {
        comp.serial.writeByte(byte);
    }
}

void HardwareIOSystem::sendSerial(serial_component & comp, const std::string & message) {
    if (!comp.connected) return;
    
    std::vector<unsigned char> data(message.begin(), message.end());
    sendSerial(comp, data);
}

void HardwareIOSystem::sendSerialByte(serial_component & comp, unsigned char byte) {
    if (comp.connected) {
        comp.serial.writeByte(byte);
    }
}

std::vector<unsigned char> HardwareIOSystem::receiveSerial(serial_component & comp) {
    comp.readBuffer.clear();
    
    if (!comp.connected) return comp.readBuffer;
    
    while (comp.serial.available() > 0) {
        unsigned char byte = comp.serial.readByte();
        comp.readBuffer.push_back(byte);
    }
    
    return comp.readBuffer;
}

// ============================================================================
// OSC Functions
// ============================================================================

void HardwareIOSystem::updateOSC(osc_component & comp) {
    // Send queued messages
    if (comp.sendEnabled && !comp.messageQueue.empty()) {
        // TODO: Actually send via ofxOscSender when integrated
        comp.messageQueue.clear();
    }
    
    // Receive messages
    if (comp.receiveEnabled) {
        // TODO: Check ofxOscReceiver for messages when integrated
    }
}

void HardwareIOSystem::setupOSCSender(osc_component & comp, const std::string & host, int port) {
    comp.sendHost = host;
    comp.sendPort = port;
    comp.sendEnabled = true;
    ofLogNotice("HardwareIOSystem") << "OSC sender setup: " << host << ":" << port;
}

void HardwareIOSystem::setupOSCReceiver(osc_component & comp, int port) {
    comp.receivePort = port;
    comp.receiveEnabled = true;
    ofLogNotice("HardwareIOSystem") << "OSC receiver setup on port " << port;
}

void HardwareIOSystem::sendOSCFloat(osc_component & comp, const std::string & address, float value) {
    if (!comp.sendEnabled) return;
	osc_component::OSCMessage msg;
    msg.address = address;
	msg.type = osc_component::OSCMessage::FLOAT;
    msg.floatValue = value;
    comp.messageQueue.push_back(msg);
}

void HardwareIOSystem::sendOSCInt(osc_component & comp, const std::string & address, int value) {
    if (!comp.sendEnabled) return;
	osc_component::OSCMessage msg;
    msg.address = address;
	msg.type = osc_component::OSCMessage::INT;
    msg.intValue = value;
    comp.messageQueue.push_back(msg);
}

void HardwareIOSystem::sendOSCString(osc_component & comp, const std::string & address, const std::string & value) {
    if (!comp.sendEnabled) return;
	osc_component::OSCMessage msg;
    msg.address = address;
	msg.type = osc_component::OSCMessage::STRING;
    msg.stringValue = value;
    comp.messageQueue.push_back(msg);
}

} // namespace ecs
