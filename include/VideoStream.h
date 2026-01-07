#pragma once

#include <string>

namespace gsd {
class VideoStream {
   public:
    VideoStream(const VideoStream&) = delete;
    VideoStream& operator=(const VideoStream&) = delete;

    VideoStream(const char* password) {};

    std::string init(const char* url);
    void start();
    void stop();
};
}  // namespace gsd