pkg load control signal

source('loader-mc.m');

args = argv();

dat = load_data(args{1});
s = tf('s');

function [sys, kff] = idmodel(in, out)
	sys = moen4(iddata([out], [in], 0.001), 1);
	sys = tf(d2c(sys));
	kff = dcgain(sys);
end

yaw_in = dat.out_yaw';
yaw_out = dat.pitch_vel';
yaw_t = dat.t;
%idx = abs(yaw_in) > 0.1;
%yaw_t = dat.t(idx);
%yaw_out = yaw_out(idx);
%yaw_in = yaw_in(idx);

%[sys_pitch, pitch_kff] = idmodel(dat.pitch_duty', dat.pitch_vel')
%[sys_pitch_i, pitch_i_kff] = idmodel(dat.ipitch', dat.pitch_duty')
[sys_yaw, yaw_kff] = idmodel(yaw_in, yaw_out)

aggr = 0.5
process_gain = 1.16
time_constant = 0.2
dead_time = 0.05

[p, i, d] = pid_imc(aggr, process_gain, time_constant, dead_time)

G = (-1.16 / (1 + 0.2 * s));
%G = sys_yaw;
G = c2d(G, 0.001);
step(sys_yaw, G);
[Y, t] = lsim(G, yaw_in', yaw_t);
figure();
plot(t, Y, 'r', t, yaw_out, 'k');
title("Simulated system (red) vs actual yaw velocity (black)");
print -dpng "output/sysid_sim_yaw_vel_vs_real.png";

input("..");

Gc = feedback(c2d(pid(18, 0, 0.2, 1), 0.001) * G, -1);
%[p, z] = pzmap(d2c(Gc))
%step(Gc, 1)
[Y, t] = lsim(Gc, dat.out_pitch, dat.t);
%plot(t, Y, t, dat.yaw_vel);


