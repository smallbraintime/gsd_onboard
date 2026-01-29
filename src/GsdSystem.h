#pragma once

#include <WiFi.h>
#include <esp_camera.h>

#include <GsdCore/MavlinkGateway.h>

#include "Drive.h"
#include "GsdTicker.h"
#include "MavSocket.h"
#include "Security.h"
#include "Sensors.h"
#include "VideoStream.h"

class GsdSystem {
   public:
    struct Config {
        MavSocket::Config network;
        struct {
            int32_t batteryMaxMv = 7400;
            int32_t batteryMinMv = 6400;
            float voltageDivider = 3.0f;

            uint8_t batteryRxPin = 14;
            uint8_t gpsRxPin = 0;

            uint8_t escLeftPin = 21;
            uint8_t escRightPin = 47;

            camera_config_t camera{
                .pin_pwdn = -1,
                .pin_reset = -1,
                .pin_xclk = 15,
                .pin_sccb_sda = 4,
                .pin_sccb_scl = 5,
                .pin_d7 = 16,
                .pin_d6 = 17,
                .pin_d5 = 18,
                .pin_d4 = 12,
                .pin_d3 = 10,
                .pin_d2 = 8,
                .pin_d1 = 9,
                .pin_d0 = 11,
                .pin_vsync = 6,
                .pin_href = 7,
                .pin_pclk = 13,
                .xclk_freq_hz = 24000000,
                .ledc_timer = LEDC_TIMER_2,
                .ledc_channel = LEDC_CHANNEL_2,
                .pixel_format = PIXFORMAT_JPEG,
                .frame_size = FRAMESIZE_QVGA,
                .jpeg_quality = 10,
                .fb_count = 2,
                .fb_location = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM,
                .grab_mode = CAMERA_GRAB_LATEST,
            };
        } hardware;
        bool msgSigning = false;
    };

    GsdSystem(const GsdSystem&) = delete;
    GsdSystem& operator=(const GsdSystem&) = delete;

    explicit GsdSystem(const Config& config);
    ~GsdSystem();

    void begin();
    void stop();
    void update();

   private:
    static void mavGatewayUpdate(void* pvParameters);

    const Config _config;
    TaskHandle_t _taskHandle = NULL;
    MavSocket _socket;
    Drive _drive;
    Sensors _sensors;
    Security _security;
    VideoStream* _videoStream;
    gsd::MavlinkGateway<GsdTicker>* _mavGateway;
};
