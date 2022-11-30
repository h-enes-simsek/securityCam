# DIY Security Camera Server

Python Flask web server to broadcast mjpeg stream and to control servo motors to rotate the ESP32-CAM.

### Screenshots

<div float="left">
<img src="ss/web.png" width="700" height="330">
</div>
<sup><sub>Original video frame was replaced.</sub></sup>

## Installation

### Virtual Environment for Python
```
cd securityCam/server
python -m venv VENV_CAM
// activate venv (os dependent)
Win: VENV_CAM\Scripts\activate.bat
Linux: source VENV_CAM/bin/activate
pip install -r requirement.txt
```

### Test with Flask's Server
```
python main.py
```
It should be accessible from internet. 

Notes:
- Don't forget to add a rule to allow the port specified in main.py for firewall. (if necessary)
- Don't forget to allocate enough buffer size for each jpeg frame in `client.request_buffers()`. Otherwise, it silently won't work. (py-mjpeg's bug or feature :) )

## Deployment

The flask app can be deployed with a web server and a wsgi as recommended.
Let's use nginx and gunicorn. (only works on linux)

### Nginx configuration

/etc/nginx/sites-available/example.conf
```
server {
    listen 80;
    server_name your-domain.com;

    location / {
        include proxy_params;
        proxy_pass http://unix:/run/cam.gunicorn.sock;
        proxy_buffering off;
        proxy_request_buffering off;
        proxy_socket_keepalive on;
        proxy_redirect off;
        add_header Last-Modified $date_gmt;
        add_header Cache-Control 'no-store, no-cache';
        if_modified_since off;
        expires off;
        etag off;
    }
}
```

Notes:
- Cached images in browsers are a problem for streaming. This nginx configuration tries to solve this issue by saying to browsers please don't cache images that i'll send.
- Don't forget add symlink to other nginx folder
`ln -s /etc/nginx/sites-available/example.conf /etc/nginx/sites-enabled/` 

### Gunicorn configuration

My flask server has multiple client support but it causes fps drop. Later, maybe i'll use/write another mjpeg stream lib instead of py-mjpeg.

Gunicorn duplicates flask instances for each worker. And my code has a global jpg producer (mjpeg-client). Becase this global object is duplicated by wsgi, mjpeg-client will be duplicated as well.
This means for each video stream, a socket will be opened to the ESP32-CAM that cannot handle. Wsgi should be configured not to duplicate flask instances.
`--workers 1` will create only one instance. `--threads int` will create threads that shares the global object.

Thread number will limit the maximum allowed user at a time, because every streaming(x-mixed-replace makes busy) blocks a thread.

Additionally, instead of starting gunicorn manually we can create a service that will be triggered automatically when request has come.

/etc/systemd/system/cam.gunicorn.service
```
[Unit]
Description=gunicorn daemon for cam
Requires=cam.gunicorn.socket
After=network.target

[Service]
User=root
Group=www-data
WorkingDirectory=path-to-project-folder/securityCam/server
ExecStart=path-to-project-folder/securityCam/server/VENV_CAM/bin/gunicorn \
          --access-logfile - \
          --workers 1 \
          --threads 5 \
          --worker-class=gthread \
          --bind unix:/run/cam.gunicorn.sock \
          --timeout 600 \
          wsgi:app

[Install]
WantedBy=multi-user.target
```

/etc/systemd/system/cam.gunicorn.socket
```
[Unit]
Description=gunicorn socket for cam

[Socket]
ListenStream=/run/cam.gunicorn.sock

[Install]
WantedBy=sockets.target
```

After creating these files start services.
```
sudo systemctl start cam.gunicorn.socket
sudo systemctl status cam.gunicorn.socket // should be active

file /run/cam.gunicorn.sock // output: /run/cam.gunicorn.sock: socket
sudo journalctl -u cam.gunicorn.socket // if fails, how to see logs
sudo systemctl stop cam.gunicorn.socket // to stop gunicorn trigger (because it is automatically trigger gunicorn)
sudo systemctl stop cam.gunicorn.service // to stop gunicorn 

// if /etc/systemd/system/cam.gunicorn.service changes, run these to restart
sudo systemctl daemon-reload
sudo systemctl restart gunicorn
```




