#pragma once

#include <Arduino.h>

#if GSDEBUG == 1

#define GSD_INIT() Serial.begin(SERIAL_BAUD)
#define GSD_DEBUG(fmt, ...)            \
    Serial.printf(fmt, ##__VA_ARGS__); \
    Serial.write('\n');

#else

#define GSD_DEBUG
#define GSD_INIT()

#endif