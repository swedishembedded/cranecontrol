pkg load control signal

function [angle] = normalize_angle(omega)
	angle = atan2(sin(omega), cos(omega));
end

source('loader.m');

args = argv();

dat = load_data(args{1});

figure(1, 'position',[0,0,1600,860]);

% PITCH manual & current
subplot(3, 2, 1);
plot(
	dat.t, dat.joy_pitch, 'r',
	dat.t, dat.pitch_vel, 'g',
	dat.t, dat.ipitch, 'b');
title('Pitch motor');
legend('Joystick', 'Pitch velocity truth', 'Pitch motor current (A)');

% YAW manual & current
subplot(3, 2, 2);
plot(
	dat.t, dat.joy_yaw, 'r',
	dat.t, dat.yaw_vel, 'g',
	dat.t, dat.iyaw, 'b');
title('Yaw motor');
legend('Joystick', 'Yaw velocity truth', 'Yaw motor current (A)');

subplot(3, 2, 3);
plot(
	dat.t, dat.pitch_vel, 'r',
	dat.t, dat.yaw_vel, 'g',
	dat.t, dat.vmot - 24, 'b');
title('Pitch and yaw activity vs vmot');
legend('Pitch', 'Yaw', 'VMOT (V)');

subplot(3, 2, 4);
plot(
	dat.t, dat.pitch, 'r',
	dat.t, dat.pitch_sp, 'g',
	dat.t, dat.pitch_ctrl, 'b',
	dat.t, (dat.pitch_sp - dat.pitch) * 100, 'k'
);
title('Target vs actual pitch');
legend('Pitch', 'Pitch target', 'Pitch controller output', 'Pitch setpoint error');

subplot(3, 2, 5);
plot(
	dat.t, dat.yaw, 'r',
	dat.t, dat.yaw_sp, 'g',
	dat.t, dat.yaw_ctrl, 'b',
	dat.t, normalize_angle(dat.yaw_sp - dat.yaw), 'k'
);
title('Target vs actual yaw');
legend('Yaw', 'Yaw target', 'Yaw control signal', 'Yaw error');

subplot(3, 2, 6);
plot(
	dat.t, dat.pitch_duty, 'r',
	dat.t, dat.yaw_duty, 'g'
);
title('Duty cycle');
legend('Pitch duty', 'Yaw duty');

input("..");
