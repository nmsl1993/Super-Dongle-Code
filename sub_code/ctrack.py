#!/usr/bin/env python3
import numpy as np
import pyximport
pyximport.install()
import ctrack_util as util
from matplotlib import mlab

SAMPLE_RATE = 256000
DEFAULT_PING_FREQ = 37500
MIN_BIN = 14
MAX_BIN = 46
PEAK_WIDTH = 4
SEARCH_THRESH = 10000

def load_samples(path):
    return np.loadtxt(open(path, 'rb'), delimiter=',')

def main():
    print("Initializing...")
    samples_0_0 = load_samples('data/256Ksps/clean/samples_0_0.csv')
    samples_0_1 = load_samples('data/256Ksps/clean/samples_0_1.csv')
    samples_1_0 = load_samples('data/256Ksps/clean/samples_1_0.csv')
    print("Finished loading samples")

    # Search for pingers in up to first 1.2s of samples
    num_samples = int(1.2*SAMPLE_RATE)
    if len(samples_0_0) <= num_samples:
        search_range = samples_0_0
    else:
        search_range = samples_0_0[:num_samples]

    # Compute rfft magnitude in 256 sample steps, no overlap. Remove DC
    # component from signal (introduced due to 2.5V DC bias for ADCs)
    spectrum, freqs, times = mlab.specgram(search_range, 256, SAMPLE_RATE,
                                           noverlap=0, mode='magnitude', detrend='mean')

    bins = set()
    # For each FFT chunk, identify peaks over a threshold
    for i in range(len(times)):
        result = spectrum[:, i]
        for bin_id in range(MIN_BIN, MAX_BIN+1):
            if result[bin_id] > SEARCH_THRESH:
                detect = True
                for k in range(int(-PEAK_WIDTH/2), int(PEAK_WIDTH/2 + 1)):
                    detect = detect and ((k == 0) or result[bin_id] >
                                            result[bin_id+k])
                if detect:
                    bins.add(bin_id)

    for bin_id in bins:
        print("Pinger detected with frequency %f Hz" % freqs[bin_id])

    # We could use some heuristic to select a pinger, but instead
    # we'll just let python's set choose one at random
    if len(bins) > 0:
        ping_freq = freqs[bins.pop()]
        print("Chose pinger at %f Hz" % ping_freq)
    else:
        ping_freq = DEFAULT_PING_FREQ
        print("No pinger detected. Defaulting to %f Hz" % ping_freq)

    util.process_and_show(samples_0_0, samples_0_1, samples_1_0, ping_freq)

if __name__ == '__main__':
    main()
