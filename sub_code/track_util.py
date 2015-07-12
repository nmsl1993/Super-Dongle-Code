import math

class NCO():
    def __init__(self, freq):
        self._angle = 0.0
        SAMPLE_RATE = 100000.0
        samples_per_cycle = SAMPLE_RATE/freq
        self._step = 2.0*math.pi/samples_per_cycle
        self._update()

    def _update(self):
        self._cosTerm = math.cos(self._angle)
        self._sinTerm = math.sin(self._angle)

    def step(self):
        self._angle += self._step
        self._update()

    def cosTerm(self):
        return self._cosTerm

    def sinTerm(self):
        return self._sinTerm

class RBuf():
    def __init__(self, bufsize):
        self._sinBuffer = []
        self._cosBuffer = []
        for x in range(bufsize):
            self._sinBuffer.append(0.0)
            self._cosBuffer.append(0.0)
        self._sinSum = 0.0
        self._cosSum = 0.0
        self._index = 0

    def put(self, sinVal, cosVal):
        self._sinSum -= self._sinBuffer[self._index]
        self._cosSum -= self._cosBuffer[self._index]
        self._sinSum += sinVal
        self._cosSum += cosVal
        self._sinBuffer[self._index] = sinVal
        self._cosBuffer[self._index] = cosVal
        self._index += 1
        self._index = self._index % len(self._sinBuffer)

    def cosSum(self):
        return self._cosSum

    def sinSum(self):
        return self._sinSum

    def angle(self):
        return math.atan2(self._sinSum, self._cosSum)

class DTFT():
    def __init__(self, bufsize):
        self._rbuf = RBuf(bufsize)

    def step(self, sample, nco):
        cosProduct = sample * nco.cosTerm()
        sinProduct = sample * nco.sinTerm()
        self._rbuf.put(sinProduct, cosProduct)

    def phase(self):
        return self._rbuf.angle()

    def mag_sq(self):
        return self._rbuf.cosSum()**2 + self._rbuf.sinSum()**2

class Phase_Var():
    def __init__(self, bufsize):
        self._avgbuf = RBuf(bufsize)
        self._valbuf = []
        for x in range(bufsize):
            self._valbuf.append(0.0)
        self._avg = 0.0
        self._index = 0
        self._bufsize = bufsize

    def put(self, phase):
        self._valbuf[self._index] = phase
        self._avgbuf.put(math.sin(phase)/self._bufsize,
                         math.cos(phase)/self._bufsize)
        self._avg = self._avgbuf.angle()
        self._index += 1
        self._index = self._index % len(self._valbuf)

    def average(self):
        return self._avg

    def variance(self):
        diff = [phase_difference(x, self._avg)**2 for x in self._valbuf]
        return float(sum(diff)/len(diff))

def phase_difference(a, b):
    diff = a - b
    while diff > math.pi:
        diff -= 2*math.pi
    while diff < -math.pi:
        diff += 2*math.pi
    return diff
