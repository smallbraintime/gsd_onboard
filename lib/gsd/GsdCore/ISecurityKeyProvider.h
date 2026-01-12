#pragma once

#include <etl/array.h>
#include <cstdint>

namespace gsd {
class ISecurityKeyProvider {
   public:
    using Key = etl::array<uint8_t, 32>;

    virtual ~ISecurityKeyProvider() = default;
    Key getKey();
};
}  // namespace gsd