clear;

M = csvread('../data/100Ksps/clean_new/clean_new.csv');
Mreshaped = reshape(M',[1 numel(M)]);
sample_rate = 100e3;
sample_depth = 128;
time = linspace(1,length(M)*sample_depth/sample_rate,length(M)*sample_depth);
t1_0 = Mreshaped(1:3:end);
t0_0 = Mreshaped(2:3:end);
t0_1 = Mreshaped(3:3:end);
disp('plotting')
plot(time,t0_0,'r.',time,t1_0,'g.',time,t0_1,'b.');
legend('trans 0,0', 'trans 1,0', 'trans 0,1');