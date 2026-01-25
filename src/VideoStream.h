#pragma once

#include <ESP32-RTSPServer.h>
#include <esp_camera.h>
#include <esp_random.h>
#include <etl/string.h>
#include <etl/to_string.h>
#include <cstdio>

#include <GsdCore/IVideoStream.h>

#include "Debug.h"

class VideoStream : public gsd::IVideoStream {
   public:
    VideoStream(const VideoStream&) = delete;
    VideoStream& operator=(const VideoStream&) = delete;

    VideoStream(const camera_config_t& config);

    void begin();
    void end();
    Url start() override;
    void stop() override;
    bool isOk() override;

   private:
    static void sendVideo(void* pvParameters);
    void createUrl();

    RTSPServer _rtsp;
    TaskHandle_t _taskHandle = NULL;
    camera_config_t _config;
    etl::string<32> _password;
    volatile bool _isOk = false;
    volatile bool _isStreaming = false;
    Url _url;
};
