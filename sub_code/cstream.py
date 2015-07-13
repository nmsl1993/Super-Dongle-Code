#!/usr/bin/env python3
import numpy as np
import pyximport
pyximport.install()
import ctrack_stream as stream
from matplotlib import mlab

import zmq

PING_FREQ = 37500

def main():
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.setsockopt_string(zmq.SUBSCRIBE, u"")
    socket.connect("tcp://192.168.0.93:34675")
    print("Using pinger with frequency %f" % PING_FREQ)

    while True:
        nd = socket.recv_json()
        samples_1_0 = nd[0::3]
        samples_0_0 = nd[1::3]
        samples_0_1 = nd[2::3]
        stream.process_streamed_data(samples_0_0, samples_0_1, samples_1_0)

if __name__ == '__main__':
    main()
