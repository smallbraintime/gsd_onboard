#pragma once

#include <WiFi.h>

#include "GsdCore/MavlinkGateway.h"

#include "Drive.h"
#include "MavSocket.h"
#include "Notifier.h"
#include "SecurityKeyProvider.h"
#include "Sensors.h"
#include "VideoStream.h"

constexpr const char* SSID = "gsd";
constexpr const char* PASSWORD = "admin";

class GsdSystem {
   public:
    struct Config {
        IPAddress localAddress = (192, 168, 4, 1);
        IPAddress gateway = (192, 168, 4, 1);
        IPAddress subnet = (255, 255, 255, 0);
        uint16_t localPort = 14550;
        bool ssidHidden = false;
        bool wifiLongRangeMode = false;
        bool msgSigning = false;
    };

    GsdSystem(const GsdSystem&) = delete;
    GsdSystem& operator=(const GsdSystem&) = delete;

    explicit GsdSystem(const Config& config)
        : _config(config),
          _socket(SSID,
                  PASSWORD,
                  config.localAddress,
                  config.gateway,
                  config.subnet,
                  config.localPort,
                  config.wifiLongRangeMode,
                  config.ssidHidden),
          _mavGateway({.msgSigning = config.msgSigning},
                      _socket,
                      _sensors,
                      _videoStream,
                      _drive,
                      _keyProvider) {}

    void begin() { _socket.begin(); }
    void stop() { _socket.stop(); }
    void update() { _mavGateway.update(); }

   private:
    const Config _config;
    MavSocket _socket;
    Sensors _sensors;
    VideoStream _videoStream;
    Drive _drive;
    SecurityKeyProvider _keyProvider;
    gsd::MavlinkGateway<Notifier> _mavGateway;
};
