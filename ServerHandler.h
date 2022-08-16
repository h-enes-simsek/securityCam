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
  
  httpd_handle_t server_httpd;

  static esp_err_t mjpegHandler(httpd_req_t *req);
  static esp_err_t controlServo(httpd_req_t *req);
  static ServoHandler *mServo; 
};

#endif
