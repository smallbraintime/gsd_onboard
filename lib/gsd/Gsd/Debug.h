#pragma once

#ifdef DEBUG

#ifdef ARDUINO
#include <Arduino.h>
#include <cstdlib>
#define GSD_DEBUG_INIT() Serial.begin(SERIAL_BAUD)
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
#include <cassert>
#include <iostream>
#define GSD_DEBUG_INIT() ((void)0)
#define GSD_LOG(msg) std::cout << "[LOG] " << msg << std::endl
#define GSD_ASSERT(cond) assert(cond)
#endif

#else

#define GSD_INIT() ((void)0)
#define GSD_LOG(msg) ((void)0)
#define GSD_ASSERT(cond) ((void)0)

#endif