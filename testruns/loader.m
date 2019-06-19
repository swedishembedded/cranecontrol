pkg load control miscellaneous

function [dat] = load_data(fname)
	data = dlmread(fname, ";");

	time = ((data(:,1) - data(1, 1)) * 1e-6);
	i = 2;
	dat.pitch_duty = data(:, i++) / 10000;
	dat.yaw_duty = data(:, i++) / 10000;
	dat.ipitch = data(:, i++) / 1000;
	dat.iyaw = data(:, i++) / 1000;
	dat.joy_pitch = data(:, i++) / 10000;
	dat.joy_yaw = data(:,i++) / 10000;
	dat.pitch = data(:,i++) / 10000;
	dat.yaw = data(:,i++) / 10000;
	dat.pitch_ctrl = data(:,i++) / 10000;
	dat.yaw_ctrl = data(:,i++) / 10000;
	dat.pitch_sp = data(:,i++) / 10000;
	dat.yaw_sp = data(:,i++) / 10000;
	dat.vmot = data(:,i++) / 1000;

	dyaw = [0; diff(dat.yaw)];
	dat.yaw = cumsum(atan2(sin(dyaw), cos(dyaw)));
	dyaw = [0; diff(dat.yaw_sp)];
	dat.yaw_sp = cumsum(atan2(sin(dyaw), cos(dyaw)));

    t = dat.t = interp1(time, time, linspace(time(1), time(end), (time(end) - time(1)) * 1000));
    dat.ipitch = interp1(time, dat.ipitch, t);
    dat.iyaw = interp1(time, dat.iyaw, t);
    dat.joy_pitch = interp1(time, dat.joy_pitch, t);
    dat.joy_yaw = interp1(time, dat.joy_yaw, t);
    dat.pitch = interp1(time, dat.pitch, t);
    dat.yaw = interp1(time, dat.yaw, t);
    dat.pitch_ctrl = interp1(time, dat.pitch_ctrl, t);
    dat.yaw_ctrl = interp1(time, dat.yaw_ctrl, t);
    dat.pitch_sp = interp1(time, dat.pitch_sp, t);
    dat.yaw_sp = interp1(time, dat.yaw_sp, t);
    dat.vmot = interp1(time, dat.vmot, t);
    dat.pitch_duty = interp1(time, dat.pitch_duty, t);
    dat.yaw_duty = interp1(time, dat.yaw_duty, t);

	dt = 0.001;

	[b, a] = butter(4, 5 * 2 * pi, 's');
	[b, a] = tfdata(c2d(tf(b, a), dt));
	dat.pitch = filtfilt(b{1}, a{1}, dat.pitch);

	[b, a] = butter(4, 10 * 2 * pi, 's');
	[b, a] = tfdata(c2d(tf(b, a), dt));
    %dat.pitch_vel = sgolayfilt(gradient(dat.pitch) ./ gradient(t), 3, 41);
	dat.pitch_vel = filtfilt(b{1}, a{1}, gradient(dat.pitch) / dt);
    dat.yaw_vel = sgolayfilt(gradient(dat.yaw) ./ gradient(t), 3, 101);

	[b, a] = butter(4, 5 * 2 * pi, 's');
	[b, a] = tfdata(c2d(tf(b, a), dt));
    %dat.pitch_acc = sgolayfilt(gradient(dat.pitch_vel) ./ gradient(t), 3, 101);
	dat.pitch_acc = filtfilt(b{1}, a{1}, gradient(dat.pitch_vel) / dt);
end

