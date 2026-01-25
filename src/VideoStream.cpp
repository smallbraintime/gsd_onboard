#include "VideoStream.h"

VideoStream::VideoStream(const camera_config_t& config) : _config(config) {}

void VideoStream::begin() {
    if (esp_camera_init(&_config) != ESP_OK)
        GSD_DEBUG("camera init failed");
}

void end() {
    esp_camera_deinit();
}

gsd::IVideoStream::Url VideoStream::start() {
    if (_taskHandle != NULL || _isStreaming || _isOk)
        return _url;

    _rtsp.maxRTSPClients = 2;
    _rtsp.transport = RTSPServer::VIDEO_ONLY;

    if (!_rtsp.init()) {
        GSD_DEBUG("failed to init rtsp server");
        return {};
    }

    _isStreaming = true;

    xTaskCreatePinnedToCore(sendVideo, "Video", 8192, this, 0, &_taskHandle, 0);

    createUrl();
    GSD_DEBUG(_url.c_str());
    return _url;
};

void VideoStream::stop() {
    if (_isStreaming)
        return;
    _isStreaming = false;
}

bool VideoStream::isOk() {
    return _isOk;
};

void VideoStream::sendVideo(void* pvParameters) {
    VideoStream* videoStream = static_cast<VideoStream*>(pvParameters);
    sensor_t* s = esp_camera_sensor_get();

    while (videoStream->_isStreaming) {
        if (videoStream->_rtsp.readyToSendFrame()) {
            camera_fb_t* fb = esp_camera_fb_get();

            if (fb) {
                videoStream->_rtsp.sendRTSPFrame(fb->buf, fb->len, s->status.quality, fb->width,
                                                 fb->height);
                esp_camera_fb_return(fb);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    videoStream->_rtsp.deinit();
    videoStream->_isOk = false;
    videoStream->_taskHandle = NULL;
    vTaskDelete(NULL);
}

void VideoStream::createUrl() {
    _url.clear();
    _url.append("rtsp://");
    _url.append(WiFi.softAPIP().toString().c_str());
    _url.append(":");
    etl::to_string(_rtsp.rtspPort, _url, true);
    _url.append("/");
}