#pragma once

#include <GsdCore/IVideoStream.h>

class VideoStream : public gsd::IVideoStream {
   public:
    VideoStream(const VideoStream&) = delete;
    VideoStream& operator=(const VideoStream&) = delete;

    VideoStream() = default;

    void begin() {}

    Url start() override { return Url(); };
    void stop() override {};
    bool isOk() override { return true; };
};
