#pragma once

#include <string>

namespace gsd {
class IVideoStream {
   public:
    ~IVideoStream() = default;
    virtual std::string init() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void isOk() = 0;
};
}  // namespace gsd
