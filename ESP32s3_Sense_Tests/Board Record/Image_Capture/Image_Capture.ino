#include "esp_camera.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// 📡 WiFi Credentials
const char* ssid = "Mediacom_Wifi";
const char* password = "Mediacom123!";

// 📌 Set a Static IP Address
IPAddress local_IP(192, 168, 0, 34);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

// Camera Pin Config (for XIAO ESP32S3 Sense)
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    10
#define SIOD_GPIO_NUM    40
#define SIOC_GPIO_NUM    39
#define Y9_GPIO_NUM      48
#define Y8_GPIO_NUM      11
#define Y7_GPIO_NUM      12
#define Y6_GPIO_NUM      14
#define Y5_GPIO_NUM      16
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM      17
#define Y2_GPIO_NUM      15
#define VSYNC_GPIO_NUM   38
#define HREF_GPIO_NUM    47
#define PCLK_GPIO_NUM    13

AsyncWebServer server(80);

// 🚀 Initialize Camera
void startCamera() {
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
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("❌ Camera init failed! Error code: 0x%x\n", err);
    } else {
        Serial.println("✅ Camera initialized successfully!");
    }
}

void setup() {
    Serial.begin(115200);

    // 🔹 Set Static IP
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        Serial.println("❌ Failed to configure Static IP");
    } else {
        Serial.println("✅ Static IP configured!");
    }

    WiFi.begin(ssid, password);

    Serial.print("📡 Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\n✅ WiFi connected!");
    Serial.print("🌐 Access the stream at: http://");
    Serial.println(WiFi.localIP());

    startCamera();

    // 📸 Capture a Single Image (Access: http://192.168.0.34/capture)
    server.on("/capture", HTTP_GET, [](AsyncWebServerRequest *request){
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            request->send(500, "text/plain", "❌ Camera capture failed!");
            return;
        }
        AsyncWebServerResponse *response = request->beginResponse_P(200, "image/jpeg", fb->buf, fb->len);
        response->addHeader("Content-Disposition", "inline; filename=capture.jpg");
        request->send(response);
        esp_camera_fb_return(fb);
    });

    // 🎥 Live Stream (Access: http://192.168.0.34/stream)
    server.on("/stream", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncResponseStream *response = request->beginResponseStream("multipart/x-mixed-replace; boundary=frame");

        while (true) {
            camera_fb_t *fb = esp_camera_fb_get();
            if (!fb) {
                Serial.println("❌ Camera capture failed!");
                break;
            }

            response->print("--frame\r\n");
            response->print("Content-Type: image/jpeg\r\n");
            response->print("Content-Length: " + String(fb->len) + "\r\n\r\n");
            response->write(fb->buf, fb->len);
            response->print("\r\n");

            esp_camera_fb_return(fb);
            delay(50);  // Adjust frame rate
        }

        request->send(response);
    });

    server.begin();
}

void loop() {
    // Nothing needed here, server handles everything
}
