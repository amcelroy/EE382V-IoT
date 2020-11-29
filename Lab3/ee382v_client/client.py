import socketio
import time
import pickle
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
from imagecodecs import jpeg_decode

sio = socketio.Client(logger=False, engineio_logger=False, ssl_verify = False)

start_time = time.time_ns() 

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
    #print('I recieved a frame')
    print('Received frame at time %f ms'% ((time.time_ns() - start_time)/1000000))
    #print(image)
    #plt.figure()
    
    #with open('image_data.pkl', 'wb') as output:
    #    pickle.dump(image, output, pickle.HIGHEST_PROTOCOL)
    #    exit()

    my_im = jpeg_decode(image)
    print("done")

    #plt.imshow(my_im)
    #plt.savefig('output.jpeg')
    #print(image.dtype())
    #somehow do something with the image
    #display(image)

sio.connect('https://amcelroy.dyndns.org')

print('sending processed message at time %d ms' % ((time.time_ns() - start_time)/1000000))

sio.emit('message','processed')

sio.sleep(10)

print('sending processed message at time %d ms' % ((time.time_ns() - start_time)/1000000))

sio.emit('message','processed')

sio.disconnect()

