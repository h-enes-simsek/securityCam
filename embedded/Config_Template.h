#ifndef Config_H
#define Config_H

// ESP model config (used by CameraPins.h)
#define CAMERA_MODEL_AI_THINKER

// Wifi config
#define WIFI_SSID       "ssid"
#define WIFI_PASS       "pass"
#define USE_STATIC_IP   true                            // true : static_ip, false : dhcp                                                       
                                                        // use following preprocessors when USE_STATIC_IP : true
#define LOCAL_IP        192, 168, 1, 100                // (optional) request local ip in LAN 
#define GATEWAY         192, 168, 1, 1                  // (optional) 
#define SUBNET          255, 255, 255, 0                // (optional)
#define DNS_PRIMARY     8, 8, 8, 8                      // (very optional) set a custom DNS
#define DNS_SECONDARY   8, 8, 4, 4                      // (very optional) set a custom DNS

// Server config
#define SERVER_STREAM_PORT     80
#define SERVER_SERVO_PORT      81

// serial connection config (for debug)
#define BAUD_RATE       115200

// serial channel printers
#define SERIAL_PRINT(s)     Serial.print(s)
#define SERIAL_PRINTLN(s)   Serial.println(s)

#endif
