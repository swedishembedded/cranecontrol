pkg load control signal

source('loader.m');

dat = load_data('2019-04-18/2019-04-18-012716.csv');

figure(1, 'position',[0,0,1600,860]);

subplot(3, 2, 1);
plot(
	dat.t, dat.joy_pitch, 'r',
	dat.t, dat.pitch_vel, 'g',
	dat.t, dat.ipitch, 'b');
subplot(3, 2, 2);
plot(
	dat.t, dat.joy_yaw, 'r',
	dat.t, dat.yaw, 'g',
	dat.t, dat.iyaw, 'b');

input("..");
