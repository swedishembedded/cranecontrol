pkg load control signal

source('loader.m');

args = argv();

dat = load_data(args{1});
s = tf('s');

function [sys, kff] = idmodel(in, out)
	sys = moen4(iddata([out], [in], 0.001), 1);
	sys = tf(d2c(sys));
	kff = dcgain(sys);
end

yaw_in = dat.yaw_duty';
yaw_out = -dat.yaw_vel';
idx = abs(yaw_in) > 0.1;
yaw_t = dat.t(idx);
yaw_out = -yaw_out(idx);
yaw_in = yaw_in(idx);

%[sys_pitch, pitch_kff] = idmodel(dat.pitch_duty', dat.pitch_vel')
%[sys_pitch_i, pitch_i_kff] = idmodel(dat.ipitch', dat.pitch_duty')
[sys_yaw, yaw_kff] = idmodel(yaw_in, yaw_out)

pitch_in = dat.pitch_duty';
pitch_out = -dat.pitch_vel';
idx = abs(pitch_in) > 0.1;
pitch_t = dat.t(idx);
pitch_out = -pitch_out(idx);
pitch_in = pitch_in(idx);
[sys_pitch, pitch_kff] = idmodel(pitch_in, pitch_out)

%step(sys_yaw);

G = (-0.39 / (1 + 0.01 * s));
G = c2d(G, 0.001);
[Y, t] = lsim(G, dat.yaw_duty, dat.t);
figure();
plot(t, Y, 'r', t, dat.yaw_vel, 'k');
title("Simulated system (red) vs actual yaw velocity (black)");
print -dpng "output/sysid_sim_yaw_vel_vs_real.png";


Gc = feedback(c2d(pid(18, 0, 0.2, 1), 0.001) * G, -1);
%[p, z] = pzmap(d2c(Gc))
%step(Gc, 1)
[Y, t] = lsim(Gc, dat.pitch_duty, dat.t);
%plot(t, Y, t, dat.yaw_vel);


input("..");
