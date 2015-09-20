clear;
load('~/Development/Super-Dongle-Code/dumps/ping_flat.mat')
load('~/Development/Super-Dongle-Code/sub_code/synthetic_data/400khz_165_multipath3.mat')

fs=400e3
time1 = (1:length(ping_A))./400e3;
time2 = (1:length(sample_0_0))./400e3;

figure()
plot(time1,ping_A,time1,ping_B,time1,ping_C)
title('Measured data from pool')
figure()
plot(time2,sample_0_0,time2,sample_0_1,time2,sample_1_0)
title('Synthesized data from pool')
