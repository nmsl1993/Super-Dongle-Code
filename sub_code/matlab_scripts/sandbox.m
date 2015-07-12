clear;

M = csvread('../data/100Ksps/clean_new/clean_new.csv');
Mreshaped = reshape(M',[1 numel(M)]);
sample_rate = 100e3;
sample_depth = 128;
time = linspace(1,length(M)*sample_depth/sample_rate,length(M)*sample_depth);
t1_0 = Mreshaped(1:3:end);
t0_0 = Mreshaped(2:3:end);
t0_1 = Mreshaped(3:3:end);

time_max_comp = [];
time_psd = [];
for n=1:sample_depth:length(t0_0)-sample_depth

ft0_0_sub = fft(t0_0(n:n+sample_depth));
ft0_0_sub(1) = 0; %0 dc term again...
max_ind = find(ft0_0_sub == max(ft0_0_sub));
max_ind = max_ind(1); %dont get duplicates

psdx = abs(ft0_0_sub(max_ind)).^2/sum(abs(ft0_0_sub(1:sample_depth/2 + 1).^2));
time_max_comp = [time_max_comp max_ind];
time_psd = [time_psd psdx];
end
time_max_comp(1:5) = mean(time_max_comp); %idk why i need to do this...
disp('plotting')
[hAx,hLine1,hLine2] = plotyy(1:length(time_max_comp),time_max_comp,1:length(time_max_comp),time_psd);


title('PSD and FFT main component')
xlabel('FFT block')

ylabel(hAx(1),'Main Component') % left y-axis
ylabel(hAx(2),'PSD') % right y-axis
%plot(time,t0_0,'r.',time,t1_0,'g.',time,t0_1,'b.');
%legend('trans 0,0', 'trans 1,0', 'trans 0,1');

%spectrogram(t0_0,128);