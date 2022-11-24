# Arduino Project for DIY Security Camera with ESP32

# Installation

Target MCU is ESP32-CAM from Ai-Thinker.

The project includes the library [ESP32Servo](https://github.com/jkb-git/ESP32Servo) that uses Arduino's Servo lib.

Create Config.h file with desired configurations by looking Config_Template.h file.
You need to set a static local ip if you want to access the esp32 from internet, not only from your local network.

# Features

Following HTTP Requests (GET) are defined:

- http://&#60;LOCAL_IP&#62;:&#60;SERVER_PORT&#62;/mjpeg
- http://&#60;LOCAL_IP&#62;:&#60;SERVER_PORT&#62;/control_servo?tr=integer,el=integer

Example HTTP Requests:

- http://192.168.1.100:8000/mjpeg
Start mjpeg stream with mime type = multipart/x-mixed-replace
- http://192.168.1.100:8000/control_servo?tr=5&el=-10
Control servo motors with query parameters 'tr' and 'el' (traverse and elevation in degree)

