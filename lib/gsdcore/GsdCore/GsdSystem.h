#pragma once

#include <cstdint>

#include "IDrive.h"
#include "ISensors.h"
#include "IVideoStream.h"

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
        bool wifiRangeMode = false;
    };

    GsdSystem(const GsdSystem&) = delete;
    GsdSystem& operator=(const GsdSystem&) = delete;
    GsdSystem(GsdSystem&&) = delete;
    GsdSystem& operator=(GsdSystem&&) = delete;

    GsdSystem(const GsdConfig& config, IVideoStream& videoStream, ISensors& sensors, IDrive& drive)
        : _videoStream(videoStream), _sensors(sensors), _drive(drive) {}

    void init();
    void update();

   private:
    IVideoStream& _videoStream;
    ISensors& _sensors;
    IDrive& _drive;
};
}  // namespace gsd