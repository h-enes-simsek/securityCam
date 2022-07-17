#ifndef ServerHandler_H
#define ServerHandler_H

#include <WebServer.h>      // server
#include "Config.h"
#include "ServoHandler.h"
//#include <WiFi.h>			// client and to connect wifi, galiba gerekli değil

class ServerHandler
{
	public:
	ServerHandler();
	void addServo(ServoHandler *servo); 
	void connectWifi(); // connect to the wifi AP
	void createServer(); // configure, create and start the server
	void keepServerAlive(); // this method must be called periodically to keep server alive
	
	private:
	
	// server response handlers
	void http404();
	void controlServo();
	
	// client
	void doPostRequest();
	
	ServoHandler *mServo; 
	WebServer mServer;
	WiFiClient mClient;
	
};

#endif
