import socketio
import time



sio = socketio.Client(logger=True, engineio_logger=True)

@sio.event
def connect():
    print('connection established')

@sio.event
def my_message(data):

    print('message received with ', data)

    sio.emit('my response', {'response': 'my response'})

@sio.event
def disconnect():
    print('disconnected from server')

@sio.on('frame')
def on_frame(image):
    print('I recieved a frame')
    #somehow do something with the image
    #display(image)



sio.connect('http://amcelroy.dyndns.org')

print('sending processed message')
sio.emit('message','processed')

sio.wait()

