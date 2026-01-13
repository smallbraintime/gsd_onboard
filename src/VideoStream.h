#pragma once

#include <GsdCore/IVideoStream.h>

class VideoStream : public gsd::IVideoStream {
   public:
    VideoStream(const VideoStream&) = delete;
    VideoStream& operator=(const VideoStream&) = delete;

    VideoStream() = default;

    Url start() { return Url(); };
    void stop() {};
    bool isStreaming() { return true; };
};
