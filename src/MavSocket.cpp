#include "MavSocket.h"

MavSocket::MavSocket(const Config& config) : _config(config) {}

bool MavSocket::begin() {
    if (!_preferences.begin("gsd"))
        GSD_DEBUG("failed to begin preferences");

    String password = _preferences.getString("password");
    if (password.isEmpty()) {
        GSD_DEBUG("password not found");

        if (!_preferences.putString("password", _password.c_str()))
            GSD_DEBUG("failed to write the password");
    } else {
        _password = password.c_str();

        GSD_DEBUG("password found");
    }

    _preferences.end();

    WiFi.mode(WIFI_AP);

    WiFi.onEvent([this](WiFiEvent_t event, auto info) {
        if (event == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
            _remoteAddress = IPAddress();
            _remotePort = 0;
            _peerAlive = false;
            GSD_DEBUG("peer disconnected");
        }
    });

    if (!WiFi.softAPConfig(_config.address, _config.gateway, _config.subnet)) {
        GSD_DEBUG("failed to config ap");
        return false;
    }

    if (_config.wifiLongRange)
        esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_LR);

    const int channel = 0;
    const int maxConnections = 1;
    if (!WiFi.softAP(_config.ssid, _password.c_str(), channel, _config.ssidHidden,
                     maxConnections)) {
        GSD_DEBUG("failed to init ap");
        return false;
    }

    if (!_udp.begin(_config.port)) {
        GSD_DEBUG("failed to init udp");
        return false;
    }

    return true;
}

void MavSocket::stop() {
    _udp.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);

    _remoteAddress = IPAddress();
    _remotePort = 0;
    _peerAlive = false;
}

bool MavSocket::read(gsd::MavPacket& packet) {
    int packetSize = _udp.parsePacket();

    if (packetSize) {
        _ticker.detach();
        _ticker.once_ms(_config.connectionTimeoutMs, connectionTimeout, &_peerAlive);

        if (!peerAlive()) {
            _remoteAddress = _udp.remoteIP();
            _remotePort = _udp.remotePort();
            _peerAlive = true;
            GSD_DEBUG("peer connected: %s %u", _remoteAddress.toString().c_str(), _remotePort);
        }

        if (_udp.remoteIP() == _remoteAddress) {
            int len = _udp.read(packet.data(), packet.capacity());
            packet.resize(static_cast<size_t>(len));
            return true;
        }
    }

    return false;
}

void MavSocket::write(const gsd::MavPacket& packet, bool discreet) {
    if (!_peerAlive)
        return;

    _udp.beginPacket(_remoteAddress, _remotePort);

    _udp.write(packet.data(), packet.size());

    _udp.endPacket();
    // if (!_udp.endPacket())
    //     GSD_DEBUG("failed to send the packet");
}

bool MavSocket::peerAlive() {
    return _peerAlive;
}

void MavSocket::changePassword(const char* oldPassword, const char* newPassword) {
    if (oldPassword != _password) {
        GSD_DEBUG("invalid password");
        return;
    }

    size_t len = strlen(newPassword);
    if (len > _password.capacity() && len < 8) {
        GSD_DEBUG("invalid length of the password");
        return;
    }

    _password = newPassword;

    if (!_preferences.begin("gsd"))
        GSD_DEBUG("failed to begin preferences");
    if (!_preferences.putString("password", _password.c_str()))
        GSD_DEBUG("failed to write the password");
    _preferences.end();

    stop();
    begin();
}

void MavSocket::connectionTimeout(volatile bool* peerAlive) {
    *peerAlive = false;
}