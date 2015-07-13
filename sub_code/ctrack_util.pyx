import math
import numpy as np
import scipy.stats
cimport libc.math as cmath
cimport numpy as np
import matplotlib.pyplot as plt

DTYPE = np.double
ctypedef np.double_t DTYPE_t

cdef class NCO:
    cdef double _angle
    cdef double _step
    cdef double _cosTerm
    cdef double _sinTerm
    def __init__(self, freq):
        self._angle = 0.0
        SAMPLE_RATE = 256000.0
        samples_per_cycle = SAMPLE_RATE/freq
        self._step = 2.0*math.pi/samples_per_cycle
        self._update()

    cdef void _update(self):
        self._cosTerm = cmath.cos(self._angle)
        self._sinTerm = cmath.sin(self._angle)

    cpdef step(self):
        self._angle += self._step
        self._update()

    cdef double cosTerm(self):
        return self._cosTerm

    cdef double sinTerm(self):
        return self._sinTerm

cdef class RBuf:
    cdef double _sinSum, _cosSum
    cdef int _index, _size
    cdef np.ndarray _buf
    def __init__(self, bufsize):
        self._buf = np.zeros([bufsize, 2])
        self._sinSum = 0.0
        self._cosSum = 0.0
        self._index = 0
        self._size = bufsize

    cdef void put(self, double sinVal, double cosVal):
        cdef np.ndarray[DTYPE_t, ndim=2] buf = self._buf
        self._sinSum -= buf[self._index, 0]
        self._cosSum -= buf[self._index, 1]
        self._sinSum += sinVal
        self._cosSum += cosVal
        buf[self._index, 0] = sinVal
        buf[self._index, 1] = cosVal
        self._index = (self._index + 1) % self._size

    cdef double cosSum(self):
        return self._cosSum

    cdef double sinSum(self):
        return self._sinSum

    cdef double angle(self):
        return cmath.atan2(self._sinSum, self._cosSum)

cdef class DTFT:
    cdef RBuf _rbuf
    def __init__(self, bufsize):
        self._rbuf = RBuf(bufsize)

    cpdef step(self, double sample, NCO nco):
        cdef double cosProduct = sample * nco.cosTerm()
        cdef double sinProduct = sample * nco.sinTerm()
        self._rbuf.put(sinProduct, cosProduct)

    cpdef double phase(self):
        return self._rbuf.angle()

    cpdef double mag_sq(self):
        return self._rbuf.cosSum()**2 + self._rbuf.sinSum()**2

cdef class Phase_Var:
    cdef RBuf _avgbuf
    cdef np.ndarray _valbuf
    cdef double _avg
    cdef int _index, _bufsize
    def __init__(self, bufsize):
        self._avgbuf = RBuf(bufsize)
        self._valbuf = np.zeros(bufsize)
        self._avg = 0.0
        self._index = 0
        self._bufsize = bufsize

    cpdef put(self, double phase):
        cdef np.ndarray[DTYPE_t, ndim=1] valbuf = self._valbuf
        valbuf[self._index] = phase
        self._avgbuf.put(cmath.sin(phase)/self._bufsize,
                cmath.cos(phase)/self._bufsize)
        self._avg = self._avgbuf.angle()
        self._index = (self._index + 1) % self._bufsize

    cpdef double average(self):
        return self._avg

    cpdef double variance(self):
        cdef double cos_sum = 0.0
        cdef double sin_sum = 0.0
        cdef np.ndarray[DTYPE_t, ndim=1] valbuf = self._valbuf
        cdef int i
        for i in range(self._bufsize):
            cos_sum += cos(valbuf[i])
            sin_sum += sin(valbuf[i])
        cdef double Rsquared = cos_sum**2 + sin_sum**2
        cdef float var = 1-sqrt(Rsquared)/self._bufsize
        return float(total / self._bufsize)
    #return scipy.stats.variation(self._valbuf)

cdef double PI = math.pi

cpdef double phase_difference(double a, double b):
    cdef double diff = a - b
    #while diff > math.pi:
    #    diff -= 2*math.pi
    #while diff < -math.pi:
    #    diff += 2*math.pi
    diff = diff % (2*PI)
    if diff > PI:
        diff = diff - 2*PI
    return diff

cdef int DTFT_TARGET_LENGTH = 128
cdef int PHASE_VAR_LENGTH = 32
cdef int SAMPLE_RATE = 256000
cdef double PHASE_THRESH = .001
cdef double MAG_THRESH = 1.5e7
cdef double PING_COOLDOWN = 10000

highlights = []
cdef add_highlight_region(int start, int end, color='red'):
    highlights.append((start, end, color))

def render_plots(samples, phase, magnitude, variance):
    def do_plot(num, vals, title, line=None, blocking=False):
        plt.figure(num)
        plt.plot(np.arange(0, len(vals)), vals)
        plt.title(title)
        for highlight in highlights:
            plt.axvspan(highlight[0], highlight[1], color=highlight[2],
                    alpha=0.5)
        if line:
            plt.axhline(line, color='green')
        plt.show(block=blocking)
    do_plot(1, samples, "Raw Samples")
    do_plot(2, phase, "Angle")
    do_plot(3, magnitude, "Signal Magnitude", MAG_THRESH)
    do_plot(4, variance, "Phase Variance", PHASE_THRESH, True)

def process_and_show(np.ndarray[DTYPE_t] samples_0_0, np.ndarray[DTYPE_t]
        samples_0_1, np.ndarray[DTYPE_t] samples_1_0, double freq):
    cdef int length = len(samples_0_0)
    length = 256000
    cdef NCO nco = NCO(freq)
    samples_per_period = SAMPLE_RATE / freq
    dtft_len = round(samples_per_period*round(DTFT_TARGET_LENGTH/samples_per_period))
    print("Selected DTFT length of %d samples" % dtft_len)
    cdef DTFT dtft_0_0 = DTFT(dtft_len)
    cdef DTFT dtft_0_1 = DTFT(dtft_len)
    cdef DTFT dtft_1_0 = DTFT(dtft_len)
    cdef Phase_Var phase0 = Phase_Var(PHASE_VAR_LENGTH)
    cdef Phase_Var phase1 = Phase_Var(PHASE_VAR_LENGTH)
    cdef Phase_Var angle_buf = Phase_Var(PHASE_VAR_LENGTH)

    cdef double diff0, diff1

    phases = []
    variances = []
    magnitudes = []

    cdef int hlstart = -1
    cdef int lastping = 0

    cdef int i
    for i in range(length):
        nco.step()
        dtft_0_0.step(samples_0_0[i], nco)
        dtft_0_1.step(samples_0_1[i], nco)
        dtft_1_0.step(samples_1_0[i], nco)

        diff0 = phase_difference(dtft_0_1.phase(), dtft_0_0.phase())
        diff1 = phase_difference(dtft_1_0.phase(), dtft_0_0.phase())

        phase0.put(diff0)
        phase1.put(diff1)
        angle_buf.put(cmath.atan2(phase0.average(), phase1.average()))
        if angle_buf.variance() < PHASE_THRESH and dtft_0_0.mag_sq() > MAG_THRESH:
            if hlstart == -1:
                hlstart = i

            if i > lastping + PING_COOLDOWN:
                lastping = i
                heading = angle_buf.average()*180/PI
                print("PING! Heading = %f degrees, sample = %d" % (heading, i))
                add_highlight_region(i-PHASE_VAR_LENGTH, i, "green")
        elif hlstart != -1:
            add_highlight_region(hlstart, i)
            hlstart = -1
        phases.append(angle_buf.average()*180/PI)
        variances.append(angle_buf.variance())
        magnitudes.append(dtft_0_0.mag_sq())
    if hlstart != -1:
        add_highlight_region(hlstart, length)

    render_plots(samples_0_0[:200000], phases, magnitudes, variances)
