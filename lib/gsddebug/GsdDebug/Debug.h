#pragma once

#ifdef DEBUG

#include <Arduino.h>
#include <cstdlib>
#define GSD_INIT() Serial.begin(SERIAL_BAUD)
#define GSD_LOG(msg)                  \
    do {                              \
        Serial.println("[LOG] " msg); \
    } while (0)
#define GSD_ASSERT(cond)                                                                     \
    do {                                                                                     \
        if (!(cond)) {                                                                       \
            Serial.printf("\n[ASSERT] %s\nFile: %s\nLine: %d\n", #cond, __FILE__, __LINE__); \
            Serial.flush();                                                                  \
            std::abort();                                                                    \
        }                                                                                    \
    } while (0)

#else

#define GSD_INIT() ((void)0)
#define GSD_LOG(msg) ((void)0)
#define GSD_ASSERT(cond) ((void)0)

#endif