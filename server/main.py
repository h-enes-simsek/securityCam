from mjpeg.client import MJPEGClient    # will collect mjpeg frames to its buffer
from mjpeg.server import MJPEGResponse  # will use collected mjpeg frames
import requests                         # to make http requests
from flask import Flask, Response, render_template, jsonify, stream_with_context, make_response
app = Flask(__name__, template_folder='templates')

# static ip of the cam
g_cam_mjpeg_url="http://<static_ip>/stream.mjpg"
g_cam_servo_url="http://<static_ip>/control_servo"

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
def stream():
    return MJPEGResponse(relay())
    
@app.route("/turnleft", methods=['GET', 'POST'])
def turnLeft():
    global g_cam_servo_url
    res = requests.get(g_cam_servo_url + "?tr=-5")
    return Response(res.text, status=res.status_code)
    
@app.route("/turnright", methods=['GET', 'POST'])
def turnRigth():
    global g_cam_servo_url
    res = requests.get(g_cam_servo_url + "?tr=5")
    return Response(res.text, status=res.status_code)

@app.route("/turnup", methods=['GET', 'POST'])
def turnUp():
    global g_cam_servo_url
    res = requests.get(g_cam_servo_url + "?el=5")
    return Response(res.text, status=res.status_code)
    
@app.route("/turndown", methods=['GET', 'POST'])
def turnDown():
    global g_cam_servo_url
    res = requests.get(g_cam_servo_url + "?el=-5")
    return Response(res.text, status=res.status_code)

@app.route('/')
def index():
    return render_template('index.html', mjpeg_url="mjpeg")
    
# register a function to run before each request. 
@app.before_request
def start_stream():
    global client, is_MJPEG_Client_Exist, number_of_active_request, g_cam_mjpeg_url
    number_of_active_request += 1 
    if not(is_MJPEG_Client_Exist):
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
    if(number_of_active_request == 0):
        # there is no active video consumer client, MJPEGClient might stop to prevent unncessary bandwidth consumption
        client.stop()
        is_MJPEG_Client_Exist = False
        print("MJPEGClient destroyed")

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
    
# multiple clients supported but not completely.
# fps output is inversely proportional to number of clients.
# setting gunicorn worker = 1 can restrict max connected client to prevent fps drop.