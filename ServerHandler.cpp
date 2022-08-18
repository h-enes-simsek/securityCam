#include "ServerHandler.h"
#include "Arduino.h" 		// for delay()
#include "esp_camera.h"
#include "WiFi.h"

ServoHandler* ServerHandler::mServo = nullptr;

ServerHandler::ServerHandler()
{
  config = HTTPD_DEFAULT_CONFIG();
  config.server_port = SERVER_PORT;  
}

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
}

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

esp_err_t ServerHandler::mjpegHandler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t * _jpg_buf;
    char * part_buf[64];
    static int64_t last_frame = 0;
    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    while(true){
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
            break;
        }
        if(fb->format != PIXFORMAT_JPEG){
            bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            if(!jpeg_converted){
                ESP_LOGE(TAG, "JPEG compression failed");
                esp_camera_fb_return(fb);
                res = ESP_FAIL;
            }
        } else {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }

        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);

            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(fb->format != PIXFORMAT_JPEG){
            free(_jpg_buf);
        }
        esp_camera_fb_return(fb);
        if(res != ESP_OK){
            break;
        }
        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        ESP_LOGI(TAG, "MJPG: %uKB %ums (%.1ffps)",
            (uint32_t)(_jpg_buf_len/1024),
            (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);
    }

    last_frame = 0;
    return res;
}

esp_err_t ServerHandler::controlServo(httpd_req_t *req){
	
	bool isDataValid = true;
	
	if(mServo != nullptr)
	{
		// get query data
   
    size_t buf_len;
    char buff[50]; // to store query string, example query: tr=360&el=180 (13 char)
    char keyTr[5] = {0,}; // query string key 'tr'
    char keyEl[5] = {0,}; // query string key 'el'

    // query string lenght
    buf_len = httpd_req_get_url_query_len(req) + 1; // +1 for null terminator

    if(buf_len > 1)
    {
      if (httpd_req_get_url_query_str(req, buff, buf_len) == ESP_OK) 
      {
        if (httpd_query_key_value(buff, "tr", keyTr, sizeof(keyTr)) == ESP_OK &&
            httpd_query_key_value(buff, "el", keyEl, sizeof(keyEl)) == ESP_OK) 
        {
          // ok
        }
        else
        {
          SERIAL_PRINTLN("Query parameters are invalid.");
          httpd_resp_send_500(req);
        }
      }
      else
      {
        SERIAL_PRINTLN("Obtained query string could not be fetched.");
        httpd_resp_send_500(req);
      }
    }
    else
    {
      SERIAL_PRINTLN("Obtained query string size is zero.");
      httpd_resp_send_500(req);
    }

    String keyTrStr(keyTr);
    String keyElStr(keyEl);

    int tr = keyTrStr.toInt(); // toInt return 0 if fails
    int el = keyElStr.toInt(); // toInt return 0 if fails

    mServo->servoRotateTraverse(tr); 
    mServo->servoRotateElevation(el);
		
		String message = "incoming query str: " + String(buff) + "\ntr: " + keyTrStr + " el: " + keyElStr;
		SERIAL_PRINTLN(message);
		
    httpd_resp_send(req, "Servo control message received and valid.", HTTPD_RESP_USE_STRLEN);
	}
	else
	{
		SERIAL_PRINTLN("ServerHandler does not have a servo.");
    httpd_resp_set_status(req, "HTTP/1.1 Service Unavailable");
    httpd_resp_send(req, "ServerHandler does not have servo.", HTTPD_RESP_USE_STRLEN);  // service unavailable
    return ESP_FAIL;
	}

  return ESP_OK;
}

void ServerHandler::createServer()
{
  httpd_handle_t server_httpd = nullptr;
  
  httpd_uri_t servo_uri = {
      .uri       = "/control_servo",
      .method    = HTTP_GET,
      .handler   = ServerHandler::controlServo,
      .user_ctx  = NULL
  };

  httpd_uri_t stream_uri = {
      .uri       = "/mjpeg",
      .method    = HTTP_GET,
      .handler   = ServerHandler::mjpegHandler,
      .user_ctx  = NULL
  };

  Serial.printf("Starting web server on port: '%d'\n", config.server_port);
  if (httpd_start(&server_httpd, &config) == ESP_OK) 
  {
      httpd_register_uri_handler(server_httpd, &servo_uri);
      httpd_register_uri_handler(server_httpd, &stream_uri);
  }
}
