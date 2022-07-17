#include "ServoHandler.h"
#include "ServerHandler.h"
#include "Config.h"

ServoHandler servoHandler; // servo class to control 2-axis servo
ServerHandler serverHandler; // wrapper class to create a server and client

void setup(void) {
  Serial.begin(BAUD_RATE);
  serverHandler.addServo(&servoHandler);
  serverHandler.connectWifi();
  serverHandler.createServer();
}

void loop(void) {
  serverHandler.keepServerAlive();
  delay(2); //allow the cpu to switch to other tasks
}
