#ifndef ServerHandler_H
#define ServerHandler_H

#include <WebServer.h> 
#include "Config.h"
#include "ServoHandler.h"

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
	
	ServoHandler *mServo; 
	WebServer mServer;
	
};

#endif