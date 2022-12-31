#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#define CAMERA_MODEL_AI_THINKER
#include "UdpClient.hpp"
#include "ai_thinker_esp32_cam_meta.h"

const char* ssid = "helloword";
const char* passwd = "zxcvbnm8";
const char* host = "150.158.27.240";
const uint16_t serverUdpPort = 8004;
const uint16_t localUdpPort = 2333;

LightUDP streamSender;

void connectWifi(const char* ssid, const char* passphrase) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passphrase);

    Serial.println("connecting to router... ");
    //等待wifi连接成功
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.print("\nWiFi connected, local IP address:");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    while (!Serial) {
        /* code */
    }
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
    //                      for larger pre-allocated frame buffer.
    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
    Serial.println("get sensor ");
    sensor_t* s = esp_camera_sensor_get();
    // drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_SVGA);

    connectWifi(ssid, passwd);
    streamSender.begin(WiFi.localIP(), localUdpPort);
    streamSender.setServer(host, serverUdpPort);
}

void loop() {
    camera_fb_t* fb = NULL;
    size_t len = 0;
    Serial.println("do loop");

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            return;
        }
        len = fb->len;
        streamSender.send(fb->buf, len);
        esp_camera_fb_return(fb);
    }
}