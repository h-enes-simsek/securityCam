#include <WiFi.h>			// to connect wifi
#include <ESPmDNS.h>		// Multicast DNS
#include <HTTPClient.h>
#include "ServerHandler.h"
#include "Arduino.h" 		// for delay()

ServerHandler::ServerHandler() : mServer(SERVER_PORT)
{}

void ServerHandler::connectWifi()
{
	const char* ssid = WIFI_SSID;
	const char* password = WIFI_PASS;
	
	WiFi.mode(WIFI_STA); // connect Access Point(AP)
	
	#if USE_STATIC_IP
		IPAddress local_IP(LOCAL_IP);
		IPAddress gateway(GATEWAY);
		IPAddress subnet(SUBNET);
		IPAddress primaryDNS(DNS_PRIMARY);  //optional
		IPAddress secondaryDNS(DNS_SECONDARY); //optional

		// Configures static IP address
		if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
		{
			SERIAL_PRINTLN("Static IP failed to configure. IP will be assigned by DHCP.");
		}
	#endif
	
	// connect wifi
	WiFi.begin(ssid, password);

	// Wait until connection
	while (WiFi.status() != WL_CONNECTED) 
	{
		delay(500);
		SERIAL_PRINT(".");
	}
	
	SERIAL_PRINTLN("");
	SERIAL_PRINT("Connected to ");
	SERIAL_PRINT(ssid);
	SERIAL_PRINT(", IP address: ");
	SERIAL_PRINT(WiFi.localIP());
	
	if (MDNS.begin("esp32")) 
	{
		SERIAL_PRINTLN("MDNS responder started.");
	}
}

void ServerHandler::createServer()
{
	// configure urls and responses
	mServer.on("/", std::bind(&ServerHandler::handleRoot, this)); 
	mServer.onNotFound(std::bind(&ServerHandler::handleNotFound, this));

	mServer.begin(); // start server
	
	//  lambda function example if needed
	//  mServer.on("/inline", [this]() {
    //  	mServer.send(200, "text/plain", "this works as well");
	//  });
}

void ServerHandler::keepServerAlive()
{
	mServer.handleClient();
}


void ServerHandler::doPostRequest(){
  HTTPClient http;

  SERIAL_PRINT("[HTTP] begin...\n");
  // configure traged server and url
  http.begin("http://192.168.1.2:5000/upload_file"); //HTTP

  SERIAL_PRINT("[HTTP] GET...\n");
  // start connection and send HTTP header
  //int httpCode = http.GET();
  int httpCode = http.POST("deneme123");

  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      SERIAL_PRINT("[HTTP] GET... code: ");
	  SERIAL_PRINT(httpCode);
	  SERIAL_PRINT("\n");

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          SERIAL_PRINTLN(payload);
      }
  } else {
      SERIAL_PRINT("[HTTP] GET... failed, error: ");
	  SERIAL_PRINT(http.errorToString(httpCode).c_str());
	  SERIAL_PRINT("\n");
  }

  http.end();
}


void ServerHandler::handleRoot() {
  mServer.send(200, "text/plain", "hello from esp32!");

  //behave as a client
  doPostRequest();

  //make a rotate for servo (test)
  //servoHandler.servoRotateTraverse(5); // 5 to clockwise
  //servoHandler.servoRotateElevation(-5); // 5 to down
}


void ServerHandler::handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += mServer.uri();
  message += "\nMethod: ";
  message += (mServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += mServer.args();
  message += "\n";
  for (uint8_t i = 0; i < mServer.args(); i++) {
    message += " " + mServer.argName(i) + ": " + mServer.arg(i) + "\n";
  }
  mServer.send(404, "text/plain", message);
}