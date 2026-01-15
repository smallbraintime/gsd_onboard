#pragma once

#include <cstdint>

#include <GsdCore/ISecurity.h>

class Security : public gsd::ISecurity {
   public:
    Key getKey() override { return {}; }
    uint64_t getTimestamp() override { return 0; }
};
