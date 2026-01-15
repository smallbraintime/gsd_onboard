#pragma once

#include <etl/string.h>

namespace gsd {
class IVideoStream {
   public:
    using Url = etl::string<160>;

    ~IVideoStream() = default;
    virtual Url start() = 0;
    virtual void stop() = 0;
    virtual bool isOk() = 0;
};
}  // namespace gsd
