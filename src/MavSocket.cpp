#include "MavSocket.h"

MavSocket::MavSocket(const char* ssid,
                     const char* defaultPassword,
                     const IPAddress& address,
                     const IPAddress& gateway,
                     const IPAddress& subnet,
                     uint16_t port,
                     bool wifiLongRange,
                     bool ssidHidden)
    : _ssid(ssid),
      _password(defaultPassword),
      _address(address),
      _gateway(gateway),
      _subnet(subnet),
      _port(port),
      _wifiLongRange(wifiLongRange),
      _ssidHidden(ssidHidden) {
    if (!_preferences.begin("gsd")) {
        Serial.print("failed to begin preferences");
        return;
    }

    String password = _preferences.getString("password");
    if (password.isEmpty()) {
        if (!_preferences.putString("password", defaultPassword))
            Serial.print("failed to write the password");
    } else {
        _password = password.c_str();
    }

    _preferences.end();
}

bool MavSocket::begin() {
    WiFi.mode(WIFI_AP);

    WiFi.onEvent([this](WiFiEvent_t event, auto info) {
        if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
            _remoteAddress = IPAddress();
            _remotePort = 0;
        }
    });

    if (!WiFi.softAPConfig(_address, _gateway, _subnet))
        return false;

    if (_wifiLongRange)
        esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_LR);

    const int channel = 0;
    const int maxConnections = 1;
    if (!WiFi.softAP(_ssid, _password.c_str(), channel, _ssidHidden, maxConnections))
        return false;

    if (!_udp.begin(_port))
        return false;

    return true;
}

void MavSocket::stop() {
    _udp.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);

    _remoteAddress = IPAddress();
    _remotePort = 0;
}

void MavSocket::write(const gsd::MavPacket& packet) {
    _udp.beginPacket(_remoteAddress, _remotePort);

    _udp.write(packet.data(), packet.size());

    if (!_udp.endPacket())
        Serial.print("failed to send the packet");
}

bool MavSocket::read(gsd::MavPacket& packet) {
    int packetSize = _udp.parsePacket();

    if (packetSize) {
        if (_remoteAddress[0] == 0) {
            _remoteAddress = _udp.remoteIP();
            _remotePort = _udp.remotePort();
        }

        if (_udp.remoteIP() == _remoteAddress) {
            int len = _udp.read(packet.data(), packet.capacity());
            packet.resize(static_cast<size_t>(len));
            return true;
        }
    }

    return false;
}

void MavSocket::changePassword(const char* oldPassword, const char* newPassword) {
    if (oldPassword != _password)
        return;

    _password = newPassword;

    if (!_preferences.begin("gsd"))
        Serial.print("failed to begin preferences");
    if (!_preferences.putString("password", _password.c_str()))
        Serial.print("failed to write the password");
    _preferences.end();

    stop();
    begin();
}

void MavSocket::setLowTxPower() {
    WiFi.setTxPower(WIFI_POWER_2dBm);
}

void MavSocket::setHighTxPower() {
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
}