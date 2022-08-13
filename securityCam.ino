#include "ServoHandler.h"
#include "ServerHandler.h"
#include "Config.h"
#include "esp_camera.h" // cam conf
#include "CameraPins.h" // cam conf

ServoHandler servoHandler; // servo class to control 2-axis servo
ServerHandler serverHandler; // wrapper class to create a server and client

void setup(void) {
  Serial.begin(BAUD_RATE);
  serverHandler.addServo(&servoHandler);
  serverHandler.connectWifi();
  serverHandler.createServer();

  // config cam
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_1; 
  config.ledc_timer = LEDC_TIMER_1; 
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

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 15;
  config.fb_count = 2;
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void loop(void) {
  serverHandler.keepServerAlive();
  delay(2000); //allow the cpu to switch to other tasks
}
