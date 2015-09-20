import socket, time, struct, binascii, mutex
import threading
import scipy.io


import numpy, scipy
CHANNEL_DEPTH = 128
UDP_PAYLOAD_SIZE = 818 #Derived from wireshark.
UDP_IP="" #This means all interfaces?
UDP_PORT=8899
cumulative = numpy.empty((0,1))
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

                old_data = self.data
                self.data, self.addr = self.sock.recvfrom(UDP_PAYLOAD_SIZE);
                if(old_data == self.data):
                    print("WARNING: DATA DUPLICATION DETECTED")
                    self.runme = 0
                    break
                self.packet_counter += 1
                decode_string = str(CHANNEL_DEPTH*3) + 'H' + str(UDP_PAYLOAD_SIZE - CHANNEL_DEPTH*2*3) + 'x'

                nd = numpy.asarray(struct.unpack(decode_string,self.data)) #300 16bit unsigneds, followed by 50 junk bits


                cumulative = np.append(cumulative, nd)
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
    print('matr got ' + str(udpthread.packet_counter) + ' packets')
    print("kill self")
    udpthread.kill()
    udpthread.join()
    print('slicing...')
    d = dict()
    d['ping_A'] = cumulative[0::3]
    d['ping_B'] = cumulative[1::3]
    d['ping_C'] = cumulative[2::3]

    print('dumping...')
    scipy.io.savemat('dump.mat',d,do_compression=True)
