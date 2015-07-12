#!/usr/bin/env python3
import track_util as util
import matplotlib.pyplot as plt
import numpy as np
import sys
import math

DTFT_BUFFER_TARGET = 128
PHASE_VAR_BUFFER_LENGTH = 32
SAMPLE_RATE = 100000
PING_FREQ = 37500
PHASE_THRESH = .0002
MAG_THRESH = 2.3e10
PING_COOLDOWN = 2000

def load_samples(path):
    return np.loadtxt(open(path, 'rb'), delimiter=',')

highlights = []
def add_highlight_region(start, end, color='red'):
    highlights.append((start, end, color))

def render_plots(samples, phase, magnitude, variance):
    def do_plot(num, vals, title, blocking=False):
        plt.figure(num)
        plt.plot(np.arange(0, len(vals)), vals)
        plt.title(title)
        for highlight in highlights:
            plt.axvspan(highlight[0], highlight[1], color=highlight[2], alpha=0.5)
        plt.show(block=blocking)
    do_plot(1, samples, "Raw Samples")
    do_plot(2, phase, "Phase Difference")
    do_plot(3, magnitude, "Signal Magnitude")
    do_plot(4, variance, "Phase Variance", True)

def main():
    print("Initializing...")
    #samples_0_0 = load_samples('csvs/samples_0_0.csv')
    #samples_0_1 = load_samples('csvs/samples_0_1.csv')
    #samples_1_0 = load_samples('csvs/samples_1_0.csv')
    samples_0_0 = load_samples(sys.argv[1])
    samples_0_1 = load_samples(sys.argv[2])
    samples_1_0 = load_samples(sys.argv[3])

    print("Finished loading samples")

    nco = util.NCO(PING_FREQ)
    samples_per_period = SAMPLE_RATE / PING_FREQ
    dtft_length = round(samples_per_period*round(DTFT_BUFFER_TARGET/samples_per_period))
    dtft_length = 133
    print("Selected DTFT length of %d samples" % dtft_length)
    dtft_0_0 = util.DTFT(dtft_length)
    dtft_0_1 = util.DTFT(dtft_length)
    dtft_1_0 = util.DTFT(dtft_length)
    phase0 = util.Phase_Var(PHASE_VAR_BUFFER_LENGTH)
    phase1 = util.Phase_Var(PHASE_VAR_BUFFER_LENGTH)

    phases = []
    variances = []
    magnitudes = []

    hlstart = -1

    lastping = 0

    for i in range(len(samples_0_0)):
        nco.step()
        dtft_0_0.step(samples_0_0[i], nco)
        dtft_0_1.step(samples_0_1[i], nco)
        dtft_1_0.step(samples_1_0[i], nco)

        diff0 = util.phase_difference(dtft_0_1.phase(), dtft_0_0.phase())
        diff1 = util.phase_difference(dtft_1_0.phase(), dtft_0_0.phase())

        phase0.put(diff0)
        phase1.put(diff1)
        if phase0.variance() < PHASE_THRESH and dtft_0_0.mag_sq() > MAG_THRESH:
            if hlstart == -1:
                hlstart = i

            if i > lastping + PING_COOLDOWN:
                lastping = i
                heading = math.atan2(phase0.average(),
                                     phase1.average())*180/math.pi
                print("PING! Heading = %f degrees, sample = %d" % (heading, i))
                add_highlight_region(i-PHASE_VAR_BUFFER_LENGTH, i, "green")
        elif hlstart != -1:
            add_highlight_region(hlstart, i)
            hlstart = -1
        phases.append(diff0)
        variances.append(phase0.variance())
        magnitudes.append(dtft_0_0.mag_sq())
    if hlstart != -1:
        add_highlight_region(hlstart, len(samples_0_0))

    render_plots(samples_0_0, phases, magnitudes, variances)

if __name__ == '__main__':
    main()
