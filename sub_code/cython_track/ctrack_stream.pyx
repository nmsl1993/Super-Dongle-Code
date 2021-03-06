#!python
#cython: language_level=3, cdivision=True, boundscheck=False
import math
import numpy as np
import scipy.stats
import scipy.io
cimport libc.math as cmath
cimport numpy as np
import matplotlib.pyplot as plt
cimport cython

DTYPE = np.double
ctypedef np.double_t DTYPE_t

cdef double PI = math.pi

cdef class NCO:
    cdef double _angle
    cdef double _step
    cdef double _cosTerm
    cdef double _sinTerm
    cdef double _nco_update_rate 
    def __init__(self, nco_update_rate, freq):
        self._angle = 0.0
        self._nco_update_rate = nco_update_rate 
        samples_per_cycle = self._nco_update_rate/freq
        self._step = 2.0*math.pi/samples_per_cycle
        self._update()

    cdef void _update(self):
        self._cosTerm = cmath.cos(self._angle)
        self._sinTerm = cmath.sin(self._angle)

    cdef void step(self):
        self._angle -= self._step
        if self._angle < 2*PI:
            self._angle += 2*PI
        self._update()

    cdef double cosTerm(self):
        return self._cosTerm

    cdef double sinTerm(self):
        return self._sinTerm

cdef class RBuf:
    cdef double _sinSum, _cosSum
    cdef unsigned int _index, _size
    cdef np.ndarray _buf
    def __init__(self, unsigned int bufsize):
        self._buf = np.zeros([bufsize, 2])
        self._sinSum = 0.0
        self._cosSum = 0.0
        self._index = 0
        self._size = bufsize

    cdef void put(self, double sinVal, double cosVal):
        cdef np.ndarray[DTYPE_t, ndim=2] buf = self._buf
        self._sinSum -= buf[self._index, <unsigned int> 0]
        self._cosSum -= buf[self._index, <unsigned int> 1]
        self._sinSum += sinVal
        self._cosSum += cosVal
        buf[self._index, <unsigned int> 0] = sinVal
        buf[self._index, <unsigned int> 1] = cosVal
        self._index = (self._index + 1) % self._size

    cdef double cosSum(self):
        return self._cosSum

    cdef double sinSum(self):
        return self._sinSum

    cdef double angle(self):
        return cmath.atan2(self._sinSum, self._cosSum)

cdef class DTFT:
    cdef RBuf _rbuf
    cdef NCO _nco
    def __init__(self, bufsize, nco):
        self._rbuf = RBuf(bufsize)
        self._nco = nco

    cdef void step(self, double sample):
        cdef double cosProduct = sample * self._nco.cosTerm()
        cdef double sinProduct = sample * self._nco.sinTerm()
        self._rbuf.put(sinProduct, cosProduct)

    cdef double phase(self):
        return self._rbuf.angle()

    cdef double mag_sq(self):
        return self._rbuf.cosSum()**2 + self._rbuf.sinSum()**2

cdef class Phase_Var:
    cdef RBuf _avgbuf
    cdef np.ndarray _valbuf
    cdef double _avg
    cdef unsigned int _index, _bufsize
    def __init__(self, unsigned int bufsize):
        self._avgbuf = RBuf(bufsize)
        self._valbuf = np.zeros(bufsize)
        self._avg = 0.0
        self._index = 0
        self._bufsize = bufsize

    cdef void put(self, double phase):
        cdef np.ndarray[DTYPE_t, ndim=1] valbuf = self._valbuf
        valbuf[self._index] = phase
        self._avgbuf.put(cmath.sin(phase)/self._bufsize,
                cmath.cos(phase)/self._bufsize)
        self._avg = self._avgbuf.angle()
        self._index = (self._index + 1)%self._bufsize

    cdef double average(self):
        return self._avg

    cdef double variance(self):
        cdef double cos_sum = 0.0
        cdef double sin_sum = 0.0
        cdef np.ndarray[DTYPE_t, ndim=1] valbuf = self._valbuf
        cdef int i
        for i in range(self._bufsize):
            cos_sum += cmath.cos(valbuf[i])
            sin_sum += cmath.sin(valbuf[i])
        cdef double Rsquared = cos_sum**2 + sin_sum**2
        cdef double var = 1-cmath.sqrt(Rsquared)/self._bufsize
        return var
    #return scipy.stats.variation(self._valbuf)

cdef double phase_difference(double a, double b):
    cdef double diff = a - b
    while diff > math.pi:
        diff -= 2*math.pi
    while diff < -math.pi:
        diff += 2*math.pi
    return diff

cdef int DTFT_TARGET_LENGTH = 512
cdef int PHASE_VAR_LENGTH = 128
cdef int SAMPLE_RATE = 400000
cdef double PHASE_THRESH = .00005
cdef double MAG_THRESH = 5e7
cdef int PING_COOLDOWN = 200000
cdef int PING_COOLUP = 2000

cdef class Stream_Track:
    cdef NCO nco
    cdef DTFT dtft_0_0
    cdef DTFT dtft_0_1
    cdef DTFT dtft_1_0
    cdef Phase_Var phase0
    cdef Phase_Var phase1
    cdef Phase_Var angle_buf
    cdef int lastping
    cdef unsigned int idx
    cdef int ping_start
    cdef double mtm

    def __init__(self, double freq):
        self.nco = NCO(SAMPLE_RATE,freq)
        samples_per_period = SAMPLE_RATE / freq
        dtft_len = round(samples_per_period*round(DTFT_TARGET_LENGTH/samples_per_period))
        print("Selected DTFT length of %d samples" % dtft_len)
        self.dtft_0_0 = DTFT(dtft_len, self.nco)
        self.dtft_0_1 = DTFT(dtft_len, self.nco)
        self.dtft_1_0 = DTFT(dtft_len, self.nco)
        self.phase0 = Phase_Var(PHASE_VAR_LENGTH)
        self.phase1 = Phase_Var(PHASE_VAR_LENGTH)
        self.angle_buf = Phase_Var(PHASE_VAR_LENGTH)
        self.idx = 0
        self.lastping = 0
        self.ping_start = -1
        self.mtm = 0;

    
    def process(self, np.ndarray[DTYPE_t, ndim=1] samples_0_0, np.ndarray[DTYPE_t, ndim=1]
            samples_0_1, np.ndarray[DTYPE_t, ndim=1] samples_1_0):
        cdef unsigned int length = len(samples_0_0)
        cdef double diff0, diff1, angle

        cdef unsigned int i

        # loggers for matlab analysis
        cdef np.ndarray[DTYPE_t, ndim=1] nco_sin_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] nco_cos_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] dtft_0_0_mag_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] dtft_0_1_mag_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] dtft_1_0_mag_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] dtft_0_0_phase_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] dtft_0_1_phase_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] dtft_1_0_phase_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] phase0_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] phase0_avg_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] phase0_var_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] phase1_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] phase1_avg_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] phase1_var_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] angle_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] angle_avg_log = np.empty_like(samples_0_0, dtype=DTYPE)
        cdef np.ndarray[DTYPE_t, ndim=1] angle_var_log = np.empty_like(samples_0_0, dtype=DTYPE)
        
        for i in range(length):
            self.nco.step()
            nco_sin_log[i] = self.nco.sinTerm()
            nco_cos_log[i] = self.nco.cosTerm()
            self.dtft_0_0.step(samples_0_0[i])
            self.dtft_0_1.step(samples_0_1[i])
            self.dtft_1_0.step(samples_1_0[i])
            dtft_0_0_mag_log[i] = self.dtft_0_0.mag_sq()
            dtft_0_0_phase_log[i] = self.dtft_0_0.phase()
            dtft_0_1_mag_log[i] = self.dtft_0_1.mag_sq()
            dtft_0_1_phase_log[i] = self.dtft_0_1.phase()
            dtft_1_0_mag_log[i] = self.dtft_1_0.mag_sq()
            dtft_1_0_phase_log[i] = self.dtft_1_0.phase()
    
            diff0 = phase_difference(self.dtft_0_1.phase(), self.dtft_0_0.phase())
            diff1 = phase_difference(self.dtft_1_0.phase(), self.dtft_0_0.phase())
            phase0_log[i] = diff0
            phase1_log[i] = diff1
    
            self.phase0.put(diff0)
            self.phase1.put(diff1)
            phase0_avg_log[i] = self.phase0.average()
            phase0_var_log[i] = self.phase0.variance()
            phase1_avg_log[i] = self.phase1.average()
            phase1_var_log[i] = self.phase1.variance()

            angle = cmath.atan2(self.phase0.average(), self.phase1.average())
            angle_log[i] = angle
            self.angle_buf.put(angle)
            angle_avg_log[i] = self.angle_buf.average()
            angle_var_log[i] = self.angle_buf.variance()
            if self.angle_buf.variance() < PHASE_THRESH and self.dtft_0_0.mag_sq() > MAG_THRESH:
                if self.idx > self.lastping + PING_COOLDOWN or self.idx < self.lastping:
                    if self.ping_start == -1:
                        self.ping_start = self.idx
                    elif self.ping_start + PING_COOLUP > self.idx:
                        pass
                    else:
                        self.lastping = self.idx
                        self.ping_start = -1
                        heading = self.angle_buf.average()*180/PI
                        diff0 = diff0*180/PI
                        diff1 = diff1*180/PI
                        phase0a = self.phase0.average()*180/PI
                        phase1a = self.phase1.average()*180/PI
                        print("PING! Heading = %f degrees, sample = %d" % (heading, self.idx))
                        print("mag %f, pvar %f" % (self.dtft_0_0.mag_sq(), self.angle_buf.variance()))
                        print("phase0 %f (%f), phase1 %f (%f)" % (phase0a, diff0, phase1a, diff1))

            self.idx = self.idx + 1
            '''
            if self.dtft_0_0.mag_sq() > self.mtm:
                self.mtm = self.dtft_0_0.mag_sq()
                print("UPDATE THRESHMAX", self.mtm)
                heading = self.angle_buf.average()*180/PI
                print("PING! Heading = %f degrees, sample = %d" % (heading, self.idx))
            '''
        l = dict()
        l['nco_sin'] = nco_sin_log
        l['nco_cos'] = nco_cos_log
        l['dtft_0_0_mag'] = dtft_0_0_mag_log
        l['dtft_0_1_mag'] = dtft_0_1_mag_log
        l['dtft_1_0_mag'] = dtft_1_0_mag_log
        l['dtft_0_0_phase'] = dtft_0_0_phase_log
        l['dtft_0_1_phase'] = dtft_0_1_phase_log
        l['dtft_1_0_phase'] = dtft_1_0_phase_log
        l['phase0'] = phase0_log
        l['phase0_avg'] = phase0_avg_log
        l['phase0_var'] = phase0_var_log
        l['phase1'] = phase1_log
        l['phase1_avg'] = phase1_avg_log
        l['phase1_var'] = phase1_var_log
        l['angle'] = angle_log
        l['angle_avg'] = angle_avg_log
        l['angle_var'] = angle_var_log
        print('dumping')
        scipy.io.savemat('log.mat',l,do_compression=True)
