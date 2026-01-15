#pragma once

#include <cstdint>

namespace gsd {
class ISecurity {
   public:
    using Key = etl::array<uint8_t, 32>;

    ~ISecurity() = default;
    virtual Key getKey() = 0;
    virtual uint64_t getTimestamp() = 0;
};
}  // namespace gsd
