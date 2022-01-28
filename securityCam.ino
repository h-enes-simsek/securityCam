//#include <WiFi.h>
#include <WiFiClient.h>
// #include <WebServer.h>
//#include <ESPmDNS.h>
//#include <HTTPClient.h>

#include "ServoHandler.h"
#include "ServerHandler.h"
#include "Config.h"


// WebServer server(SERVER_PORT);

// servo class to control 2-axis servo
ServoHandler servoHandler;

ServerHandler serverHandler;

/*
void doPostRequest(){
  HTTPClient http;

  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  http.begin("http://192.168.1.2:5000/upload_file"); //HTTP

  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  //int httpCode = http.GET();
  int httpCode = http.POST("deneme123");

  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
      }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}


void handleRoot() {
  server.send(200, "text/plain", "hello from esp32!");

  //behave as a client
  doPostRequest();

  //make a rotate for servo (test)
  servoHandler.servoRotateTraverse(5); // 5 to clockwise
  servoHandler.servoRotateElevation(-5); // 5 to down
}


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
*/

void setup(void) {
  
  Serial.begin(BAUD_RATE);
  
  
  serverHandler.connectWifi();

  /*
  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  */
  // Serial.println("HTTP server started");

  serverHandler.createServer();
}

void loop(void) {
  //server.handleClient();
  serverHandler.keepServerAlive();
  delay(2);//allow the cpu to switch to other tasks
}
