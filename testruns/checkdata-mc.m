pkg load control signal

function [angle] = normalize_angle(omega)
	angle = atan2(sin(omega), cos(omega));
end

source('loader-mc.m');

args = argv();

dat = load_data(args{1});

figure(1, 'position',[0,0,1600,860]);

subplot(3, 2, 1);
plot(
	dat.t, dat.ib_pitch, 'k',
	dat.t, dat.out_pitch * 20, 'r');
title('PITCH command vs current');
legend('Current', 'Command');

subplot(3, 2, 2);
plot(
	dat.t, dat.ib_yaw, 'k',
	dat.t, dat.out_yaw, 'r');
title('YAW command vs current');
legend('Current', 'Command');

subplot(3, 2, 3);
plot(
	dat.t, dat.out_yaw, 'k',
	dat.t, dat.pitch_vel * 50, 'r');
title('Output YAW vs velocity');
legend('Output', 'Velocity');

input("..");
