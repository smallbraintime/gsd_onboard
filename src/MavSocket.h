#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <Ticker.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>
#include <etl/string.h>

#include <GsdCore/IMavSocket.h>

#include "Debug.h"

class MavSocket : public gsd::IMavSocket {
   public:
    struct Config {
        const char* ssid = "gsd";
        const char* defaultPassword = "gsdadmin123";
        IPAddress address{192, 168, 4, 1};
        IPAddress gateway{192, 168, 4, 1};
        IPAddress subnet{255, 255, 255, 0};
        uint16_t port = 14550;
        bool wifiLongRange = false;
        bool ssidHidden = false;
        uint32_t connectionTimeoutMs = 5000;
    };

    MavSocket(const MavSocket&) = delete;
    MavSocket& operator=(const MavSocket&) = delete;

    explicit MavSocket(const Config& config);

    bool begin();
    void stop();
    bool read(gsd::MavPacket& packet) override;
    void write(const gsd::MavPacket& packet, bool discreet) override;
    bool peerAlive() override;
    void changePassword(const char* oldPassword, const char* newPassword) override;

   private:
    void static connectionTimeout(volatile bool* peerAlive);

    WiFiUDP _udp;
    etl::string<64> _password;
    IPAddress _remoteAddress;
    uint16_t _remotePort;
    volatile bool _peerAlive = false;
    Ticker _ticker;
    Preferences _preferences;
    const Config _config;
};
