pkg load control signal

source('loader.m');

args = argv();

dat = load_data(args{1});
s = tf('s');

function [sys, kff] = idmodel(in, out)
	sys = moen4(iddata([out], [in], 0.001), 2);
	sys = tf(d2c(sys));
	kff = dcgain(sys);
end

[sys_pitch, pitch_kff] = idmodel(dat.pitch_duty', dat.pitch_vel')
%[sys_pitch_i, pitch_i_kff] = idmodel(dat.ipitch', dat.pitch_duty')
[sys_pitch_i, pitch_i_kff] = idmodel(dat.ipitch', dat.pitch_duty')
[sys_yaw, yaw_kff] = idmodel(dat.yaw_duty', dat.yaw_vel')

G = (-0.5 / (1 + 0.01 * s));
G = c2d(G, 0.001);
%[Y, t] = lsim(sys_pitch, dat.pitch_duty, dat.t);
%plot(t, Y, 'r', t, dat.pitch_vel, 'k');
[Y, t] = lsim(G, dat.pitch_duty, dat.t);
plot(t, Y, 'r', t, dat.ipitch, 'k');

Gc = feedback(c2d(pid(18, 0, 0.2, 1), 0.001) * G, -1);
%[p, z] = pzmap(d2c(Gc))
%step(Gc, 1)
[Y, t] = lsim(Gc, dat.pitch_duty, dat.t);
%plot(t, Y, t, dat.pitch_vel);

input("..");
