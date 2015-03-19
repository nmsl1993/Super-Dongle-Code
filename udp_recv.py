import socket, time, struct, binascii, mutex
import numpy, scipy
import threading
import matplotlib.pyplot as plt
UDP_PAYLOAD_SIZE=650
UDP_IP="" #This means all interfaces?
UDP_PORT=8889

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
plt.ion()
x=numpy.arange(0,100)
y=x
fig = plt.figure()
#fig.legend(loc='center right')
plt.title("ADC Data from STM32F4 Sampling @ 200Khz")
plt.xlabel("Sample (n)")
plt.ylabel("Amplitude")


ax = fig.add_subplot(111)
ax.set_xlim((0,100))
ax.set_ylim((0,4100))
#line1,line2,line3 = ax.plot(x, y, 'r-',label='ADC1',x,y,'b-',label='ADC2',x,y,'g-',label='ADC3') # Returns a tuple of line objects, thus the comma
line1,line2,line3 = ax.plot(x, y, 'r-',x,y,'b-',x,y,'g-') # Returns a tuple of line objects, thus the comma
start_time = time.time()
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
    nd = numpy.asarray(struct.unpack("300H50x",data)) #300 16bit unsigneds, followed by 50 junk bits
    print nd


    transducer0 = nd[0::3]
    transducer1 = nd[1::3]
    transducer2 = nd[2::3]

    t = numpy.arange(0,len(transducer1))
    #plt.plot(t,transducer0,'r.-', label="ADC1")
    #plt.plot(t,transducer1,'g.-', label="ADC2")
    #plt.plot(t,transducer2,'b.-', label="ADC3")
    line1.set_ydata(transducer0)
    line2.set_ydata(transducer1)
    line3.set_ydata(transducer2)
    fig.canvas.draw()
    time.sleep(.1)
