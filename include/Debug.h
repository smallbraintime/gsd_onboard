#pragma once

#include <Arduino.h>

namespace gsd {
class Debug {
   public:
    static void init() {
#ifdef DEBUG
        Serial.begin(SERIAL_BAUD);
#endif
    }

    static void log(const char* msg) {
#ifdef DEBUG
        Serial.write(msg);
#endif
    }
};
}  // namespace gsd