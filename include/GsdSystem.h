#pragma once

#include <cstdint>

#include "Actuators.h"
#include "Network.h"
#include "Sensors.h"
#include "VideoStream.h"

namespace gsd {
class GsdSystem {
   public:
    struct GsdConfig {
        const char* ssid = "gsd";
        const char* password = "admin";
        const char* localAddress = "192.168.4.1";
        uint16_t localPort = 4210;
        uint32_t heartbeatTimeoutMs = 3000;
        uint32_t reconnectTimeoutMs = 150000;
    };

    GsdSystem(const GsdSystem&) = delete;
    GsdSystem& operator=(const GsdSystem&) = delete;
    GsdSystem(GsdSystem&&) = delete;
    GsdSystem& operator=(GsdSystem&&) = delete;

    GsdSystem(const GsdConfig& config);

    void update();

   private:
    Network _network;
    Sensors _sensors;
    VideoStream _video;
    Actuators _actuators;
};
}  // namespace gsd