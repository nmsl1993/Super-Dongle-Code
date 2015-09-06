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
        self.data = ''
        self.addr = ''
        self.packet_counter = 0;
        self.runme = 1
    def run(self):
        while(self.runme):
            try:
                self.data, self.addr = self.sock.recvfrom(UDP_PAYLOAD_SIZE);
                self.packet_counter += 1
                data = ''
                addr = ''
                data = udpthread.data
                addr = udpthread.addr
                decode_string = str(CHANNEL_DEPTH*3) + 'H' + str(UDP_PAYLOAD_SIZE - CHANNEL_DEPTH*2*3) + 'x'
                nd = numpy.asarray(struct.unpack(decode_string,data)) #300 16bit unsigneds, followed by 50 junk bits


                transducer0 = nd[0::3]
                transducer1 = nd[1::3]
                transducer2 = nd[2::3]
                cumulative.append(nd)
            finally:
                time.sleep(.00001)
    def kill(self):
        self.runme = 0
udpthread = UDPThread()
udpthread.start()
#fig.legend(loc='center right')

#line1,line2,line3 = ax.plot(x, y, 'r-',label='ADC1',x,y,'b-',label='ADC2',x,y,'g-',label='ADC3') # Returns a tuple of line objects, thus the comma
start_time = time.time()
try:
    print "looping..."
    while(1):
        time.sleep(1)

except KeyboardInterrupt:
    print('pklr got ' + str(udpthread.packet_counter) + ' packets')
    print "kill self"
    udpthread.kill()
    udpthread.join()
    print('dumping...')

    pickle.dump(cumulative,open('dump.pkl','wb'))
