import socket, time, struct, binascii, mutex
import pickle
import threading


import numpy, scipy
CHANNEL_DEPTH = 128
UDP_PAYLOAD_SIZE = 818 #Derived from wireshark.
UDP_IP="" #This means all interfaces?
UDP_PORT=8899
cumulative =[]
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
        self.runme = 1
    def run(self):
        while(self.runme):
            self.lock.acquire()
            try:
                self.data, self.addr = self.sock.recvfrom(UDP_PAYLOAD_SIZE);
                self.packet_counter += 1
            finally:
                self.lock.release()
                time.sleep(.00001)
    def kill(self):
        self.runme = 0
udpthread = UDPThread()
udpthread.start()
x=numpy.arange(0,CHANNEL_DEPTH)
y=x
#fig.legend(loc='center right')

#line1,line2,line3 = ax.plot(x, y, 'r-',label='ADC1',x,y,'b-',label='ADC2',x,y,'g-',label='ADC3') # Returns a tuple of line objects, thus the comma
start_time = time.time()
try:
    while(1):
        print "drawloop"
        udpthread.lock.acquire()
        data = ''
        addr = ''
        try:
            data = udpthread.data
            addr = udpthread.addr
        finally:
            udpthread.lock.release()
        print 'Connected by:', addr, 'bufsize', len(data), 'Recieved', udpthread.packet_counter, ' packets. ', float(udpthread.packet_counter)/(time.time() - start_time), ' per second'
        #print binascii.hexlify(data)
        #print binascii.hexlify(data)[1200:]
        decode_string = str(CHANNEL_DEPTH*3) + 'H' + str(UDP_PAYLOAD_SIZE - CHANNEL_DEPTH*2*3) + 'x'
        #print decode_string
        nd = numpy.asarray(struct.unpack(decode_string,data)) #300 16bit unsigneds, followed by 50 junk bits
        print nd


        transducer0 = nd[0::3]
        transducer1 = nd[1::3]
        transducer2 = nd[2::3]
        cumulative.append(nd)
        t = numpy.arange(0,len(transducer1))
        print "ydata seet"
        time.sleep(.1)
except KeyboardInterrupt:
    print "kill self"
    udpthread.kill()
    udpthread.join()
    print('dumping...')
    pickle.dump(cumulative,open('dump.pkl','wb'))
