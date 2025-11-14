/*
 * ESP32-CAM - Capture and Send Images to Flask Server
 * This code captures images and sends them to the Flask server for dust detection
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";     // Replace with your WiFi SSID
const char* password = "YOUR_WIFI_PASSWORD";   // Replace with your WiFi password

// Flask server URL
const char* serverUrl = "http://YOUR_PC_IP:8000/upload-image";  // Replace with your PC's IP

// Capture interval (in milliseconds)
const unsigned long captureInterval = 60000;  // Capture every 60 seconds (1 minute)
unsigned long lastCaptureTime = 0;

// Camera pins for AI-Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// LED Pin (usually GPIO 4 on ESP32-CAM)
#define LED_PIN 4

// Function declarations
bool initCamera();
void sendImageToServer(camera_fb_t* fb);

void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  Serial.println("ESP32-CAM Starting...");

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize camera
  if (initCamera()) {
    Serial.println("Camera initialized successfully!");
  } else {
    Serial.println("Camera initialization failed!");
    ESP.restart();
  }

  delay(2000);
}

void loop() {
  // Check if it's time to capture and send image
  if (millis() - lastCaptureTime >= captureInterval) {
    lastCaptureTime = millis();
    
    Serial.println("Capturing image...");
    
    // Turn on LED for capture
    digitalWrite(LED_PIN, HIGH);
    
    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    
    if (!fb) {
      Serial.println("Camera capture failed!");
      digitalWrite(LED_PIN, LOW);
      return;
    }
    
    Serial.printf("Image captured: %d bytes\n", fb->len);
    
    // Turn off LED
    digitalWrite(LED_PIN, LOW);
    
    // Send image to server
    if (WiFi.status() == WL_CONNECTED) {
      sendImageToServer(fb);
    } else {
      Serial.println("WiFi not connected!");
      WiFi.reconnect();
    }
    
    // Return frame buffer
    esp_camera_fb_return(fb);
  }
  
  delay(1000);  // Small delay to prevent watchdog issues
}

bool initCamera() {
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

  // Image quality settings
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
    config.jpeg_quality = 10;  // 0-63, lower means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;  // 800x600
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;
  }

  // Sensor settings for better image quality
  sensor_t* s = esp_camera_sensor_get();
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect)
  s->set_whitebal(s, 1);       // 0 = disable, 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable, 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4
  s->set_exposure_ctrl(s, 1);  // 0 = disable, 1 = enable
  s->set_aec2(s, 0);           // 0 = disable, 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable, 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable, 1 = enable
  s->set_wpc(s, 1);            // 0 = disable, 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable, 1 = enable
  s->set_lenc(s, 1);           // 0 = disable, 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable, 1 = enable
  s->set_vflip(s, 0);          // 0 = disable, 1 = enable
  s->set_dcw(s, 1);            // 0 = disable, 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable, 1 = enable

  return true;
}

void sendImageToServer(camera_fb_t* fb) {
  HTTPClient http;
  
  Serial.println("Sending image to server...");
  
  http.begin(serverUrl);
  http.addHeader("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW");
  
  // Create multipart form data
  String head = "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n";
  head += "Content-Disposition: form-data; name=\"file\"; filename=\"image.jpg\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";
  
  String tail = "\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";
  
  uint32_t totalLen = head.length() + fb->len + tail.length();
  
  // Allocate buffer for complete request
  uint8_t* fullBuf = (uint8_t*)malloc(totalLen);
  if (!fullBuf) {
    Serial.println("Failed to allocate memory!");
    http.end();
    return;
  }
  
  // Copy data to buffer
  memcpy(fullBuf, head.c_str(), head.length());
  memcpy(fullBuf + head.length(), fb->buf, fb->len);
  memcpy(fullBuf + head.length() + fb->len, tail.c_str(), tail.length());
  
  // Send POST request
  int httpResponseCode = http.POST(fullBuf, totalLen);
  
  // Free buffer
  free(fullBuf);
  
  if (httpResponseCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    String response = http.getString();
    Serial.println("Server response:");
    Serial.println(response);
  } else {
    Serial.printf("Error sending POST: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  
  http.end(); 
}