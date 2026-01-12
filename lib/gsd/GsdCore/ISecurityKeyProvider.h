#pragma once

#include <etl/string.h>

namespace gsd {
class ISecurityKeyProvider {
   public:
    using Key = etl::string<32>;

    virtual ~ISecurityKeyProvider() = default;
    const Key& getKey();
};
}  // namespace gsd