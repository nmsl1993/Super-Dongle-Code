import socket, time, struct, binascii, mutex
import threading
import zmq

import numpy, scipy
CHANNEL_DEPTH = 128
UDP_PAYLOAD_SIZE = 818 #Derived from wireshark.
UDP_IP="" #This means all interfaces?
UDP_PORT=8899

#sock.setblocking(0)
class UDPThread(threading.Thread):
    def __init__(self):
        super(UDPThread,self).__init__()
        self.sock=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((UDP_IP,UDP_PORT))
        self.lock = threading.Lock()
        self.data = ''
        self.addr = ''
        self.packet_counter = 0;
    def run(self):
        while(1):
            self.lock.acquire()
            try:
                self.data, self.addr = self.sock.recvfrom(UDP_PAYLOAD_SIZE);
                self.packet_counter += 1
            finally:
                self.lock.release()
                time.sleep(.00001)
udpthread = UDPThread()
udpthread.start()

context = zmq.Context()
socket = context.socket(zmq.PUB)
socket.bind("tcp://*:34675")

start_time = time.time()
while(1):
    udpthread.lock.acquire()
    data = ''
    addr = ''
    try:
        data = udpthread.data
        addr = udpthread.addr
    finally:
        udpthread.lock.release()
    #print 'Connected by:', addr, 'bufsize', len(data), 'Recieved', udpthread.packet_counter, ' packets. ', float(udpthread.packet_counter)/(time.time() - start_time), ' per second'
    decode_string = str(CHANNEL_DEPTH*3) + 'H' + str(UDP_PAYLOAD_SIZE - CHANNEL_DEPTH*2*3) + 'x'
    nd = numpy.asarray(struct.unpack(decode_string,data)) #300 16bit unsigneds, followed by 50 junk bits

    socket.send_json(nd.tolist())
