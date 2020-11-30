import socketio
import time
import pickle
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
from imagecodecs import jpeg_decode

sio = socketio.Client(logger=False, engineio_logger=False, ssl_verify = False)

start_time = time.time_ns() 
output_filename = 'output0.jpg'

def save_myimage(my_im):
    plt.imshow(my_im)
    plt.axis('off')
    
    print(output_filename)
    plt.savefig(output_filename, bbox_inches=0)
    print("done")


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


#@sio.event
#def snapshot(image):
#    print('Received snapshot at time %f s'% ((time.time_ns() - start_time)/1000000000))

#    my_im = jpeg_decode(image)
#    save_myimage(my_im)
#    print("done")

@sio.event
def snapshot(image):
    print('Received snapshot at time %f s'% ((time.time_ns() - start_time)/1000000000))
    print('Received an image of '+str(type(image)))
    sio.emit('labels',data=image)

    #sio.emit('labels',im_jpeg)
    print("done")


#@sio.on('frame')
#def on_frame(image):
    #print('I recieved a frame')
    #print('Received frame at time %f ms'% ((time.time_ns() - start_time)/1000000))
    #print(image)
    #plt.figure()
    
    #with open('image_data.pkl', 'wb') as output:
    #    pickle.dump(image, output, pickle.HIGHEST_PROTOCOL)
    #    exit()

    #my_im = jpeg_decode(image)
    #print("done")

    #plt.imshow(my_im)
    #plt.savefig('output.jpeg')
    #print(image.dtype())
    #somehow do something with the image
    #display(image)


sio.connect('https://amcelroy.dyndns.org')

print('sending processed message at time %f s' % ((time.time_ns() - start_time)/1000000000))

sio.emit('message',data="serverGetFrame")

sio.sleep(5)
output_filename = 'output1.jpg'
print('sending processed message at time %f s' % ((time.time_ns() - start_time)/1000000000))

sio.emit('message',data="serverGetFrame")

sio.sleep(5)

while(1):
    sio.emit('message', data="serverGetFrame")
    sio.sleep(5)

sio.disconnect()
exit()
