#ifndef ServerHandler_H
#define ServerHandler_H

#include <WebServer.h> 
#include "Config.h"

class ServerHandler
{
	public:
	ServerHandler();
	void connectWifi(); // connect to the wifi AP
	void createServer(); // configure, create and start the server
	void keepServerAlive(); // this method must be called periodically to keep server alive
	
	private:
	
	// server response handlers
	void doPostRequest();
	void handleRoot();
	void handleNotFound();
	
	WebServer mServer;
};

#endif