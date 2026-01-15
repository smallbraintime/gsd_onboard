#pragma once

#include <WiFi.h>

#include "GsdCore/MavlinkGateway.h"

#include "Drive.h"
#include "GsdTicker.h"
#include "MavSocket.h"
#include "Pins.h"
#include "Security.h"
#include "Sensors.h"
#include "VideoStream.h"

class GsdSystem {
   public:
    struct Config {
        MavSocket::Config network;
        bool msgSigning = false;
    };

    GsdSystem(const GsdSystem&) = delete;
    GsdSystem& operator=(const GsdSystem&) = delete;

    explicit GsdSystem(const Config& config)
        : _config(config),
          _socket(config.network),
          _mavGateway({.msgSigning = config.msgSigning},
                      _socket,
                      _sensors,
                      _videoStream,
                      _drive,
                      _security) {}

    void begin() {
        _drive.begin(ESC_LEFT, ESC_RIGHT);
        _sensors.begin(GPS_RX, GPS_TX, BATTERY_IN);
        _videoStream.begin();
        _socket.begin();
    }
    void stop() { _socket.stop(); }
    void update() { _mavGateway.update(); }

   private:
    const Config _config;
    MavSocket _socket;
    Sensors _sensors;
    VideoStream _videoStream;
    Drive _drive;
    Security _security;
    gsd::MavlinkGateway<GsdTicker> _mavGateway;
};
