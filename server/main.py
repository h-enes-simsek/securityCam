from mjpeg.client import MJPEGClient    # will collect mjpeg frames to its buffer
from mjpeg.server import MJPEGResponse  # will use collected mjpeg frames
import requests                         # to make http requests
from flask import Flask, Response, render_template, jsonify
app = Flask(__name__, template_folder='templates')

# static ip of the cam
cam_mjpeg_url="http://<static_ip>/stream.mjpg"
cam_servo_url="http://<static_ip>/control_servo"

# Create a new client thread
client = MJPEGClient(cam_mjpeg_url)

# Allocate memory buffers for frames
# buffer size is important, as long as it is less than required, it will not work
bufs = client.request_buffers(300000, 50)
for b in bufs:
    client.enqueue_buffer(b)
    
# Start the client in a background thread
client.start()

def relay():
    while True:
        buf = client.dequeue_buffer()
        yield memoryview(buf.data)[:buf.used]
        client.enqueue_buffer(buf)

@app.route('/mjpeg')
def stream():
    return MJPEGResponse(relay())
    
@app.route("/turnleft", methods=['GET', 'POST'])
def turnLeft():
    res = requests.get(cam_servo_url + "?tr=-5")
    return Response(res.text, status=res.status_code)
    
@app.route("/turnright", methods=['GET', 'POST'])
def turnRigth():
    res = requests.get(cam_servo_url + "?tr=5")
    return Response(res.text, status=res.status_code)

@app.route("/turnup", methods=['GET', 'POST'])
def turnUp():
    res = requests.get(cam_servo_url + "?el=5")
    return Response(res.text, status=res.status_code)
    
@app.route("/turndown", methods=['GET', 'POST'])
def turnDown():
    res = requests.get(cam_servo_url + "?el=-5")
    return Response(res.text, status=res.status_code)

@app.route('/')
def index():
    return render_template('index.html', mjpeg_url="mjpeg")

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)