#include "SecurityKeyProvider.h"

SecurityKeyProvider::Key SecurityKeyProvider::getKey() {
    Key key;

    Preferences preferences;

    if (!preferences.begin("gsd")) {
        Serial.print("failed to begin the preferences");
        return key;
    }

    if (!preferences.getBytes("key", key.data(), key.size())) {
        for (int i = 0; i < 32; i++) {
            key[i] = (uint8_t)esp_random();
        }

        if (!preferences.putBytes("key", key.data(), key.size()))
            Serial.print("failed to write the key");
    }

    preferences.end();

    return key;
}