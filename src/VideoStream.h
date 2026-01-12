#pragma once

#include <GsdCore/IVideoStream.h>

class VideoStream : public gsd::IVideoStream {
   public:
    Url start() { return Url(); };
    void stop() {};
    bool isStreaming() { return true; };
};
