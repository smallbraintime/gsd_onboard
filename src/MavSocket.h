#pragma once

#include <Preferences.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>
#include <etl/string.h>

#include <GsdCore/IMavSocket.h>

class MavSocket : public gsd::IMavSocket {
   public:
    MavSocket(const MavSocket&) = delete;
    MavSocket& operator=(const MavSocket&) = delete;

    explicit MavSocket(const char* ssid,
                       const char* defaultPassword,
                       const IPAddress& address,
                       const IPAddress& gateway,
                       const IPAddress& subnet,
                       uint16_t port,
                       bool wifiLongRange,
                       bool ssidHidden);

    bool begin();
    void stop();
    void write(const gsd::MavPacket& packet) override;
    bool read(gsd::MavPacket& packet) override;
    void changePassword(const char* oldPassword, const char* newPassword) override;
    void setLowTxPower() override;
    void setHighTxPower() override;

   private:
    WiFiUDP _udp;
    const char* _ssid;
    etl::string<64> _password;
    const IPAddress _address;
    const IPAddress _gateway;
    const IPAddress _subnet;
    const uint16_t _port;
    const bool _wifiLongRange;
    const bool _ssidHidden;
    IPAddress _remoteAddress;
    uint16_t _remotePort;
    Preferences _preferences;
};
