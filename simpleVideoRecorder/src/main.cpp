#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include "esp_task_wdt.h"
#define CAMERA_MODEL_AI_THINKER
#include "UdpClient.hpp"
#include "ai_thinker_esp32_cam_meta.h"
#include "PubSubClient.h"
#include "blink.h"

const char* ssid = "CDS";
const char* passwd = "Chan888999";
const char* host = "150.158.27.240";
const uint16_t serverUdpPort = 8005;
const uint16_t localUdpPort = 2333;

const char* mqtt_server = "bemfa.com"; //默认，MQTT服务器
const int mqtt_server_port = 9501;      //默认，MQTT服务器
const char*  topic = "esp32cam002";

WiFiClient espClient;
PubSubClient client(espClient);
LightUDP streamSender;

String getChipID () {
    uint32_t chipId = 0;
    for(int i=0; i<17; i=i+8) {
	  chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
    return String(chipId);
}

void connectWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid, passwd);

    Serial.println("connecting to router... ");
    //等待wifi连接成功
    while (!WiFi.isConnected()) {
        Serial.print(".");
        delay(1000);
    }
    Serial.print("\nWiFi connected, local IP address:");
    Serial.println(WiFi.localIP());
}

uint captureInterval = 1000;
unsigned long lastSetCaptureIntervalTime = 0;
void setCaptureInterval (uint v) {
    captureInterval = v;
    lastSetCaptureIntervalTime = millis();
    Serial.print("\nset captureInterval: ");
    Serial.println(v);
}

void callback(char* topic, byte* payload, size_t length) {
	Serial.print("Topic:");
	Serial.println(topic);
	String msg = "";
	for (size_t i = 0; i < length; i++) {
		msg += (char)payload[i];
	}
	Serial.print("Msg:");
	Serial.println(msg);

    if (msg.startsWith("CaptureInterval:")) {
        String s = msg.substring(16);
        setCaptureInterval(atoi(s.c_str()));
    }
    if (msg.equals("restart")) {
        Serial.println("delay 3s restart.");
        delay(1000 * 3);
        ESP.restart();
    }
}

void reconnect() {
    if (!WiFi.isConnected()) {
        delay(1000);
    }
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.println("Attempting MQTT connection...");
		// Attempt to connect
        char* ID_MQTT = "d6d8fcd160ce48a3b38ff76e7e2df726"; // getChipID().c_str();
		if (client.connect(ID_MQTT)) {
			Serial.println("connected");
			Serial.print("subscribe:");
			Serial.println(topic);
			//订阅主题，如果需要订阅多个主题，可发送多条订阅指令client.subscribe(topic2);client.subscribe(topic3);
			client.subscribe(topic);
			blink_ok();
		} else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 3 seconds before retrying
			blink_err();
			delay(5000);
		}
	}
}

void setCamera () {
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
}

unsigned long lastCaptureTime = 0;
camera_fb_t* fb = NULL;
void doCapture () {
    unsigned long curTime = millis();
    if (curTime < lastCaptureTime || curTime > lastCaptureTime + captureInterval) {
        Serial.print("\ncaptureInterval ");
        Serial.println(curTime - lastCaptureTime);
        lastCaptureTime = curTime;
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            return;
        }
        streamSender.send(fb->buf, fb->len);
        esp_camera_fb_return(fb);

        if (curTime < lastSetCaptureIntervalTime || curTime > lastSetCaptureIntervalTime + 1000 * 60 * 1) {
            captureInterval =  1000 * 60 * 10;
            Serial.print("set big capture interval ");
            Serial.println(captureInterval);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    while (!Serial) {
        /* code */
    }
    Serial.println("setup");

    esp_task_wdt_init(60 * 3, true);
    esp_task_wdt_add(NULL);

    connectWifi();
    setCamera();

    streamSender.begin(WiFi.localIP(), localUdpPort);
    streamSender.setServer(host, serverUdpPort);

    client.setServer(mqtt_server, mqtt_server_port); // 设置mqtt服务器
  	client.setCallback(callback); // mqtt消息处理
}

void loop() {
    esp_task_wdt_reset();
    
    if (client.connected()) {
        doCapture();
		client.loop();
	} else {
		reconnect();
	}
}