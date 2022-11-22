from mjpeg.client import MJPEGClient
from mjpeg.server import MJPEGResponse
from flask import Flask, Response
app = Flask(__name__)

# static ip of the cam
url="http://123.194.136.153/stream.mjpg"

# Create a new client thread
client = MJPEGClient(url)

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

@app.route('/')
def stream():
    return MJPEGResponse(relay())

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)