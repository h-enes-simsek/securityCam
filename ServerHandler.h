#ifndef ServerHandler_H
#define ServerHandler_H

#include "esp_http_server.h"
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
	
	ServoHandler *mServo; 
  httpd_handle_t server_httpd;
};

#endif
