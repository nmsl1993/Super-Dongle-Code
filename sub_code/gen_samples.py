#!/usr/bin/env python3

#########################################
### CUAUV Hydrophones sample generator (modeled after code written by Patrick Dear
# @ author Patrick Dear (translated to Python3 from Matlab by Noah Levy)
#########################################

#Multipathing channel model taken from here: Underwater Acoustic Communications: Design Considerations on the Phyisical Layer
import numpy as np
import numpy.random
import scipy
import scipy.io
SAMPLE_FREQ = 400e3
ELEM_SPACING = 0.015 #element spacing in meters
SPEED_SOUND = 1497

PING_FREQ = 37500
PING_LENGTH = 0.0095 #In seconds (9ms)
PINGER_RING_UP_TAU = 0.0010 #In seconds (1ms tau)
INTER_PING_TIME = 1.5 #In seconds (1.5s)
PING_PERIOD = PING_LENGTH+INTER_PING_TIME

PING_AZIMUTH_ANGLE = np.radians(165) #NOTE: This is the azimuthal angle in spherical cooridinates (phi in physics)
PING_POLAR_ANGLE = np.radians(110) #This is the polar angle in spherical coords (theta in physics)

NOISE_POWER = 0.05
NUMBER_OF_MULTIPATHS = 10
MAXIMUM_MULTIPATH_DISTANCE = 12 #In meters
MAX_MULTIPATH_GAIN = .3
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
envelope = np.zeros(ping_start_delay*SAMPLE_FREQ)

while len(envelope) < len(time):

    on_period = (1-np.exp(-1*np.linspace(0,PING_LENGTH,PING_LENGTH*SAMPLE_FREQ)/PINGER_RING_UP_TAU)) #*np.ones(PING_LENGTH*SAMPLE_FREQ)
    off_period = np.zeros(SAMPLE_FREQ*(INTER_PING_TIME+ping_interval_uncertainty_time*(np.random.random() - .5)))
    #print(envelope.shape)
    #print(on_period.shape)
    #print(off_period.shape)
    envelope = np.concatenate((envelope,on_period,off_period),axis=0) #Generate an envelope a fixed size ON period, plus a variable sized inter ping time (because the pinger does not seem to have a totally deterministic duty cycle))
    print(envelope.shape)
envelope = envelope[0:len(time)].flatten()

source_0_0 = np.cos(omega*time - phase_ref)*envelope
source_0_0 += NOISE_POWER*(np.random.randn(len(time)))
source_0_1 = np.cos(omega*time - phase_0_1)*envelope
source_0_1 += NOISE_POWER*(np.random.randn(len(time)))
source_1_0 = np.cos(omega*time - phase_1_0)*envelope
source_1_0 += NOISE_POWER*(np.random.randn(len(time)))

###########################################
##LAME ATTEMPT AT MODELING MULTIPATHING
#######################################
multipath_distances = MAXIMUM_MULTIPATH_DISTANCE*np.random.random(NUMBER_OF_MULTIPATHS) #random
multipath_delays_in_seconds = multipath_distances/SPEED_SOUND
multipath_delays_in_ticks = multipath_delays_in_seconds*SAMPLE_FREQ
multipath_gains = np.random.random(multipath_delays_in_ticks.shape)*MAX_MULTIPATH_GAIN

channel_vector = np.zeros(np.max(multipath_delays_in_ticks) + 1)
channel_vector[0] = 1 #Line of sight path
for idx,x in enumerate(multipath_delays_in_ticks):
    channel_vector[x] = multipath_gains[idx]
print(channel_vector)

source_0_0 = np.convolve(source_0_0,channel_vector)
source_0_1 = np.convolve(source_0_1,channel_vector)
source_1_0 = np.convolve(source_1_0,channel_vector)


gain_0_0 = 250 + np.random.randint(-50,50)
gain_0_1 = 250 + np.random.randint(-50,50)
gain_1_0 = 250 + np.random.randint(-50,50)
sample_0_0 = 2048+np.floor(source_0_0*gain_0_0)
sample_0_1 = 2048+np.floor(source_0_1*gain_0_1)
sample_1_0 = 2048+np.floor(source_1_0*gain_1_0)

d = dict()
d['sample_0_0'] = sample_0_0.flatten()
d['sample_0_1'] = sample_0_1.flatten()
d['sample_1_0'] = sample_1_0.flatten()

scipy.io.savemat('test',d,do_compression=True)
