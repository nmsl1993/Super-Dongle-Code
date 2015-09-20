#!/usr/bin/env python3

#########################################
### CUAUV Hydrophones sample generator (modeled after code written by Patrick Dear
# @ author Patrick Dear (translated to Python3 from Matlab by Noah Levy)
#########################################


import numpy as np
import numpy.random
import scipy
import scipy.io
SAMPLE_FREQ = 400e3
ELEM_SPACING = 0.015 #element spacing in meters
SPEED_SOUND = 1497

PING_FREQ = 37500
PING_LENGTH = 0.0025 #In seconds (1ms)
INTER_PING_TIME = 1.5 #In seconds (1.5s)
PING_PERIOD = PING_LENGTH+INTER_PING_TIME

PING_AZIMUTH_ANGLE = np.radians(40) #NOTE: This is the azimuthal angle in spherical cooridinates (phi in physics)
PING_POLAR_ANGLE = np.radians(90) #This is the polar angle in spherical coords (theta in physics)

NOISE_POWER = 0.05

DATA_LEN = 5 #In seconds

lambda_sound = SPEED_SOUND/PING_FREQ #Phase velocity in H20 divided by ping frequency is wavelength (roughly 4 cm)
omega = 2*scipy.pi*PING_FREQ #Angular frequency

k_hat = np.asarray([np.sin(PING_POLAR_ANGLE)*np.cos(PING_AZIMUTH_ANGLE), np.sin(PING_POLAR_ANGLE)*np.sin(PING_AZIMUTH_ANGLE), np.cos(PING_POLAR_ANGLE)]) #Normalized wavevector, khat

ping_start_delay = np.random.random() #Delay start of first ping by 0-1 sec
ping_interval_uncertainty_time = .1 #Interval between pings can vary by as much as .1 second

delta_phase_x = k_hat[0]*ELEM_SPACING/SPEED_SOUND*omega
delta_phase_y = k_hat[1]*ELEM_SPACING/SPEED_SOUND*omega

phase_ref = 2*scipy.pi*np.random.random()

phase_1_0 = phase_ref + delta_phase_x;
phase_0_1 = phase_ref + delta_phase_y;


time = np.arange(0,DATA_LEN,1/SAMPLE_FREQ)
envelope = np.zeros((ping_start_delay*SAMPLE_FREQ,1))

while len(envelope) < len(time):

    on_period = np.ones((PING_LENGTH*SAMPLE_FREQ,1))
    off_period = np.zeros((SAMPLE_FREQ*(INTER_PING_TIME+ping_interval_uncertainty_time*(np.random.random() - .5)),1))
    #print(envelope.shape)
    #print(on_period.shape)
    #print(off_period.shape)
    envelope = np.concatenate((envelope,on_period,off_period),axis=0) #Generate an envelope a fixed size ON period, plus a variable sized inter ping time (because the pinger does not seem to have a totally deterministic duty cycle))
    print(envelope.shape)
envelope = envelope[0:len(time)].flatten()

source_0_0 = np.cos(omega*time - phase_ref)*envelope
source_0_0 += NOISE_POWER*(np.random.randn(len(time),1).flatten())
source_0_1 = np.cos(omega*time - phase_0_1)*envelope
source_0_1 += NOISE_POWER*(np.random.randn(len(time),1).flatten())
source_1_0 = np.cos(omega*time - phase_1_0)*envelope
source_1_0 += NOISE_POWER*(np.random.randn(len(time),1).flatten())


gain_0_0 = 250 + np.random.randint(-50,50)
gain_0_1 = 250 + np.random.randint(-50,50)
gain_1_0 = 250 + np.random.randint(-50,50)
sample_0_0 = 2048+source_0_0*gain_0_0
sample_0_1 = 2048+source_0_1*gain_0_1
sample_1_0 = 2048+source_1_0*gain_1_0

d = dict()
d['sample_0_0'] = sample_0_0
d['sample_0_1'] = sample_0_1
d['sample_1_0'] = sample_1_0

scipy.io.savemat('test',d,do_compression=True)
