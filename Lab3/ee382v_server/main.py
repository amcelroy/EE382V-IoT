import time
from flask import Flask, jsonify
from flask import render_template
from picamera import PiCamera
import io
import threading
from flask_socketio import SocketIO, send, emit
import queue

stream = io.BytesIO();
app = Flask(__name__, static_url_path='/static')
socketio = SocketIO(app)
camera = PiCamera()

@socketio.on('label')
def label_handler(data):
    print('Label sent')
    socketio.emit('label', data)

@socketio.on('message')
def message_handler(message):
    print(message)
    if message == 'processed':
        socketio.emit('frame', q.get())
    elif message == 'serverGetFrame':
        socketio.emit('snapshot', q.get())

def camera_thread(q):
    camera.resolution = (512, 512)
    camera.start_preview()
    time.sleep(2)
    count = 0
    for foo in camera.capture_continuous(stream, 'jpeg', use_video_port=True, quality=30):
        num_bytes = stream.tell()
        stream.seek(0)
        q.put(stream.read(num_bytes))
        stream.seek(0)
        stream.truncate()

@app.route('/snapshot')
def snapshot():
    return q.get()
    #return jsonify({ 'jpeg' : q.get() })

@app.route('/')
def hello():
    return render_template("index.html")

if __name__ == '__main__':
    q = queue.Queue(maxsize=2)
    t1 = threading.Thread(target = camera_thread, args=(q,)).start()
    socketio.run(app, host='0.0.0.0', port=443, ssl_context=('cert.pem', 'key.pem'))
