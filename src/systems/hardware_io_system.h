#pragma once

#include "base_system.h"
#include "ofMain.h"
#include "../components/hardware_components.h"

namespace ecs {

// ============================================================================
// Hardware IO System
// ============================================================================
// Handles communication with external hardware (serial, OSC, etc).
// Components are pure data - this system contains the IO logic.

class HardwareIOSystem : public ISystem {
public:
    const char* getName() const override { return "HardwareIOSystem"; }
    
    void update(entt::registry& registry, float deltaTime) override;
    
    // Serial functions
    static void updateSerial(serial_component& comp, float dt);
    static bool connectSerial(serial_component& comp, const std::string& device = "", int baud = 9600);
    static void disconnectSerial(serial_component& comp);
    static void sendSerial(serial_component& comp, const std::vector<unsigned char>& data);
    static void sendSerial(serial_component& comp, const std::string& message);
    static void sendSerialByte(serial_component& comp, unsigned char byte);
    static std::vector<unsigned char> receiveSerial(serial_component& comp);
    
    // OSC functions
    static void updateOSC(osc_component& comp);
    static void setupOSCSender(osc_component& comp, const std::string& host, int port);
    static void setupOSCReceiver(osc_component& comp, int port);
    static void sendOSCFloat(osc_component& comp, const std::string& address, float value);
    static void sendOSCInt(osc_component& comp, const std::string& address, int value);
    static void sendOSCString(osc_component& comp, const std::string& address, const std::string& value);
};

} // namespace ecs
