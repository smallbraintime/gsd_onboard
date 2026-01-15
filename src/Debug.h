#pragma once

#include <Arduino.h>

#ifdef GSDEBUG

#define GSD_INIT() Serial.begin(SERIAL_BAUD)
#define GSD_DEBUG(...)          \
    Serial.printf(__VA_ARGS__); \
    Serial.println()

#else

#define GSD_DEBUG
#define GSD_INIT()

#endif