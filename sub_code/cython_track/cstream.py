#!/usr/bin/env python3
import numpy as np
import scipy.io
import pyximport
pyximport.install()
import ctrack_stream as stream
from matplotlib import mlab
import zmq
CHANNEL_DEPTH=128
UDP_PAYLOAD_SIZE=818
UDP_IP=""
UDP_PORT=8899
PING_FREQ = 37500

MODE='SIMULATED'
#MODE='UDP'
#MODE='ZMQ'
def zmq_run():
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.setsockopt_string(zmq.SUBSCRIBE, u"")
    socket.connect("tcp://192.168.0.93:34675")

    while True:
        nd = socket.recv_json()
        samples_0_1 = np.array(nd[0::3]).astype(np.double)
        samples_0_0 = np.array(nd[1::3]).astype(np.double)
        samples_1_0 = np.array(nd[2::3]).astype(np.double)
        processor.process(samples_0_0, samples_0_1, samples_1_0)

if __name__ == '__main__':
    processor = stream.Stream_Track(PING_FREQ)
    print("Using pinger with frequency %f" % PING_FREQ)
    if MODE == 'ZMQ':
        main()
    if MODE == 'SIMULATED':
        d = scipy.io.loadmat('../synthetic_data/400khz_165_multipath3.mat')
    #    print(d['sample_0_0'].size)
        for index in range(0,d['sample_0_0'].size,CHANNEL_DEPTH):
            samples_0_0 = d['sample_0_0'][:,index:index+CHANNEL_DEPTH].flatten().astype(np.double)
            samples_0_1 = d['sample_0_1'][:,index:index+CHANNEL_DEPTH].flatten().astype(np.double)
            samples_1_0 = d['sample_1_0'][:,index:index+CHANNEL_DEPTH].flatten().astype(np.double)
            processor.process(samples_0_0, samples_0_1, samples_1_0)
