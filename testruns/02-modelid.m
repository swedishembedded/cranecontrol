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

[sys_pitch, pitch_kff] = idmodel(dat.pitch_duty', dat.pitch_vel')
[sys_yaw, yaw_kff] = idmodel(dat.yaw_duty', dat.yaw_vel')

G = (-1.35 / (1 + 0.02 * s));
G = c2d(G, 0.001);
[Y, t] = lsim(G, dat.yaw_duty, dat.t);
plot(t, Y, 'r', t, dat.yaw_vel, 'k');

Gc = feedback(c2d(pid(2, 0, 0.0, 1), 0.001) * G, -1);
step(Gc, 1)
[Y, t] = lsim(Gc, dat.pitch_duty, dat.t);
%plot(t, Y, t, dat.pitch_vel);

input("..");
