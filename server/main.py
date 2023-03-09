from mjpeg.client import MJPEGClient    # will collect mjpeg frames to its buffer
from mjpeg.server import MJPEGResponse  # will use collected mjpeg frames
import requests                         # to make http requests
from flask import Flask, Response, render_template, jsonify, stream_with_context, make_response

from flask_httpauth import HTTPBasicAuth                                    # for basic auth
from werkzeug.security import generate_password_hash, check_password_hash   # for basic auth

import environ                          # to make settings configurable

# read .env file
env = environ.Env()
environ.Env.read_env()

app = Flask(__name__, template_folder='templates', static_url_path="", static_folder="templates/static")
auth = HTTPBasicAuth()

# created accounts
users = {}
users[env("HTTP_AUTH_USERNAME")] = generate_password_hash(env("HTTP_AUTH_PASSWORD"))
print(users)
print(type(env("HTTP_AUTH_PASSWORD")))

@auth.verify_password
def verify_password(username, password):
    if username in users and check_password_hash(users.get(username), password):
        return username

# static ip of the cam
g_cam_mjpeg_url = env("CAM_URL")
g_cam_servo_url = env("SERVO_URL") 

# only one MJPEGClient should exist
is_MJPEG_Client_Exist = False     

# as long as active request is greater than 1 at any time (long running streaming(multipart/x-mixed-replace) causes this)
# MJPEGClient should be exist
number_of_active_request = 0      

# global MJPEG client
client = None

# TODO: performance of stream_with_context should be investigated
# thanks to stream_with_context decorator, teardown_request is not called before http request has ended for streaming(multipart/x-mixed-replace) 
@stream_with_context
def relay():
    while True:
        buf = client.dequeue_buffer()
        yield memoryview(buf.data)[:buf.used]
        client.enqueue_buffer(buf)

# number of active video watcher
# if this counter counts wrong, it might indicate deployment problem.   
@app.route('/counter')
def summary():
    global number_of_active_request
    return make_response(jsonify(number_of_active_request-1), 200) # -1 because of this request

@app.route('/mjpeg')
@auth.login_required
def stream():
    return MJPEGResponse(relay())
    
@app.route("/turnleft", methods=['GET', 'POST'])
@auth.login_required
def turnLeft():
    global g_cam_servo_url
    try:
        res = requests.get(g_cam_servo_url + "?tr=5&el=0")
        return Response(res.text, status=res.status_code)
    except Exception as e:
        print(e)
        return Response(status=500)
    
    
@app.route("/turnright", methods=['GET', 'POST'])
@auth.login_required
def turnRigth():
    global g_cam_servo_url
    try:
        res = requests.get(g_cam_servo_url + "?tr=-5&el=0")
        return Response(res.text, status=res.status_code)
    except Exception as e:
        print(e)
        return Response(status=500)

@app.route("/turnup", methods=['GET', 'POST'])
@auth.login_required
def turnUp():
    global g_cam_servo_url
    try:
        res = requests.get(g_cam_servo_url + "?tr=0&el=-5")
        return Response(res.text, status=res.status_code)
    except Exception as e:
        print(e)
        return Response(status=500)
    
@app.route("/turndown", methods=['GET', 'POST'])
@auth.login_required
def turnDown():
    global g_cam_servo_url
    try:
        res = requests.get(g_cam_servo_url + "?tr=0&el=5")
        return Response(res.text, status=res.status_code)
    except Exception as e:
        print(e)
        return Response(status=500)

@app.route('/')
@auth.login_required
def index():
    return render_template('index.html', mjpeg_url="mjpeg")

# register a function to run before each request. 
@app.before_request
def start_stream():
    global client, is_MJPEG_Client_Exist, number_of_active_request, g_cam_mjpeg_url
    number_of_active_request += 1 
    if (is_MJPEG_Client_Exist == False):
        is_MJPEG_Client_Exist = True
        print("new MJPEGClient thread")
        
        client = MJPEGClient(g_cam_mjpeg_url) # Create a new client thread

        # Allocate memory buffers for frames
        # buffer size is important, as long as it is less than required, it will not work
        bufs = client.request_buffers(300000, 50)
        for b in bufs:
            client.enqueue_buffer(b)
        
        client.start()
    
# register a function to run after each request.
@app.teardown_request
def end_stream(exc):
    global client, is_MJPEG_Client_Exist, number_of_active_request
    #print("Teardown {0!r}".format(exc))
    number_of_active_request -= 1 
    if(number_of_active_request == 0 and is_MJPEG_Client_Exist == True):
        # there is no active video consumer client, MJPEGClient might stop to prevent unncessary bandwidth consumption
        client.stop()
        is_MJPEG_Client_Exist = False
        print("MJPEGClient destroyed")

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
    
# multiple clients supported but not completely.
# fps output is inversely proportional to number of clients.
# setting gunicorn worker = 1 can restrict max connected client to prevent fps drop.