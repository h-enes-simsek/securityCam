#include <ESPmDNS.h>		// Multicast DNS
//#include <HTTPClient.h>
#include "ServerHandler.h"
#include "Arduino.h" 		// for delay()
///#include <WiFiClient.h>

ServerHandler::ServerHandler() : mServer(SERVER_PORT)
{}

void ServerHandler::addServo(ServoHandler *servo)
{
	mServo = servo;
}

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
		SERIAL_PRINTLN("\nMDNS responder started.");
	}
}

void ServerHandler::createServer()
{
	// configure urls and responses
	mServer.on("/control_servo", HTTP_POST, std::bind(&ServerHandler::controlServo, this)); 
	mServer.onNotFound(std::bind(&ServerHandler::http404, this));

	mServer.begin(); // start server
	
	//  lambda function example if needed
	//  mServer.on("/example_uri", [this]() {
  //  	mServer.send(200, "text/plain", "this works as well");
	//  });
}

void ServerHandler::keepServerAlive()
{
	mServer.handleClient();
}

/*
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
*/

void ServerHandler::doPostRequest()
{
	IPAddress server(192,168,1,3);
	String boundary = "abcd123xyz987";
	String boundaryWithPrefix = "\r\n--" + boundary;
	if (mClient.connect(server, 5000)) 
	{
		SERIAL_PRINT("connected");
		//mClient.println("POST /upload_file?q=arduino HTTP/1.1");
		mClient.println("POST /camera HTTP/1.1");
		mClient.println("Host: 192.168.1.3:5000");
		mClient.println("Content-Type: multipart/x-mixed-replace;boundary=" + boundary);
		mClient.println(boundaryWithPrefix);


    mClient.println("Content-Type: image/jpeg\r\nContent-Length: 4\r\n");
    mClient.println("gghh");
    mClient.println(boundaryWithPrefix);

    mClient.println("Content-Type: image/jpeg\r\nContent-Length: 4\r\n");
    mClient.println("ttyyu");
    mClient.println(boundaryWithPrefix);

    /*
		int i = 0;
		String imgSize = "5";
		char *img = "abcde";
		while(i < 3 && mClient.connected())
		{
			mClient.println("Content-Type: image/jpeg\r\nContent-Length: " + imgSize + "\r\n");
			mClient.println(boundaryWithPrefix);
			i = i + 1;
		}
   */
	}
}


void ServerHandler::controlServo() {
	bool isDataValid = true;
	
	if(mServo != nullptr)
	{
		// get data obtained 
		
		String message = "\n";
		for (uint8_t i = 0; i < mServer.args(); i++) {
			String key = mServer.argName(i);
			String val = mServer.arg(i);
			message += mServer.argName(i) + ": " + mServer.arg(i) + "\n";
			
			if(key == "tr")
				mServo->servoRotateTraverse( val.toInt() ); // toInt return 0 if fails
			else if(key == "el")
				mServo->servoRotateElevation( val.toInt() ); // toInt return 0 if fails
			else 
				isDataValid = false;
		}
		
		SERIAL_PRINTLN(message);
		
		if(isDataValid)
			mServer.send(200, "text/plain", "Servo control message received and valid.");
		else
			mServer.send(400, "text/plain", "Servo control message received but NOT valid."); // bad request
	}
	else
	{
		SERIAL_PRINTLN("ServerHandler does not have servo.");
		mServer.send(503, "text/plain", "ServerHandler does not have servo."); // service unavaible
	}
  
}

void ServerHandler::http404() {
doPostRequest();
  String message = "Page Not Found\n\n";
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
