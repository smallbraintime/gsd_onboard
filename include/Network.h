#pragma once

#include <cstddef>
#include <cstdint>

namespace gsd {
class Network {
   public:
    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;

    Network(const char* name,
            const char* password,
            const char* localAddress,
            uint16_t localPort,
            bool longRangeMode = false) {}

    bool start();
    void stop();
    bool changePassword();
    void write(const uint8_t* buffer, size_t size);
    int read(const uint8_t* buffer, size_t size);
};
}  // namespace gsd