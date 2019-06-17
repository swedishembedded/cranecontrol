pkg load control signal

function [angle] = normalize_angle(omega)
	angle = atan2(sin(omega), cos(omega));
end

source('loader.m');

args = argv();

dat = load_data(args{1});

figure(1);
plot(
	dat.t, dat.pitch_vel, 'r',
	dat.t, dat.pitch_duty, 'b');
title('Pitch velocity vs duty cycle');
%legend('Pitch velocity', 'Duty cycle');
print -dpng "output/pitch_vel_vs_duty.png";

figure(1);
plot(
	dat.t, dat.yaw_vel, 'r',
	dat.t, dat.yaw_duty, 'b');
title('Yaw velocity vs duty cycle');
%legend('Yaw velocity', 'Duty cycle');
print -dpng "output/yaw_vel_vs_duty.png";

% PITCH manual & current
figure(1);
plot(
	dat.t, dat.joy_pitch, 'r',
	dat.t, dat.ipitch, 'b');
title('Pitch motor');
legend('Joystick', 'Pitch motor current (A)');
print -dpng "output/joystick_pitch_vs_current.png";

% YAW manual & current
figure(1);
plot(
	dat.t, dat.joy_yaw, 'r',
	dat.t, dat.iyaw, 'b');
title('Yaw motor');
legend('Joystick', 'Yaw motor current (A)');
print -dpng "output/joystick_yaw_vs_current.png";

figure(1);
plot(
	dat.t, dat.pitch, 'r',
	dat.t, dat.pitch_sp, 'g'
);
title('Target vs actual pitch');
legend('Pitch', 'Pitch target');
print -dpng "output/auto_pitch_vs_pitch_sp.png";

figure(1);
plot(
	dat.t, dat.yaw, 'r',
	dat.t, dat.yaw_sp, 'g'
);
title('Target vs actual yaw');
legend('Yaw', 'Yaw target');
print -dpng "output/auto_yaw_vs_yaw_sp.png";

figure(1);
plot(
	dat.t, dat.vmot - 24, 'b');
title('VMOT');
legend('VMOT - 24V (V)');
print -dpng "output/vmot_variation.png";

figure(1);
plot(
	dat.t, dat.pitch_duty, 'r',
	dat.t, dat.yaw_duty, 'g'
);
title('Duty cycle');
legend('Pitch duty', 'Yaw duty');
print -dpng "output/pitch_yaw_duty_output.png";

