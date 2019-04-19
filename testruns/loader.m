pkg load control miscellaneous

function [dat] = load_data(fname)
    data = dlmread(fname, ";");

    time = ((data(:,1) - data(1, 1)) * 1e-6);
    dat.vpitch = data(:, 2) / 10000;
    dat.vyaw = data(:, 3) / 10000;
    dat.ipitch = data(:, 4) / 1000;
    dat.iyaw = data(:, 5) / 1000;
    dat.joy_pitch = data(:, 6) / 10000;
    dat.joy_yaw = data(:,7) / 10000;
    dat.pitch = data(:,8) / 10000;
    dat.yaw = data(:,9) / 10000;
    dat.pitch_ctrl = data(:,10) / 10000;
    dat.yaw_ctrl = data(:,11) / 10000;
    dat.pitch_sp = data(:,12) / 10000;
    dat.yaw_sp = data(:,13) / 10000;
	dat.vmot = data(:,14) / 1000;
	dat.pitch_duty = data(:,15) / 1000;
	dat.yaw_duty = data(:,16) / 1000;

	dyaw = [0; diff(dat.yaw)];
	dat.yaw = cumsum(atan2(sin(dyaw), cos(dyaw)));

    t = dat.t = interp1(time, time, linspace(time(1), time(end), (time(end) - time(1)) * 1000));
    dat.vpitch = interp1(time, dat.vpitch, t);
    dat.vyaw = interp1(time, dat.vyaw, t);
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

	[b, a] = butter(4, 400, 's');
	[b, a] = tfdata(c2d(tf(b, a), 0.001));
	dat.pitch_truth = filter(b{1}, a{1}, dat.pitch);

	[b, a] = butter(4, 40, 's');
	[b, a] = tfdata(c2d(tf(b, a), 0.001));
    %dat.pitch_vel = sgolayfilt(gradient(dat.pitch) ./ gradient(t), 3, 41);
	dat.pitch_vel = filter(b{1}, a{1}, gradient(dat.pitch) / 0.001);
    dat.yaw_vel = sgolayfilt(gradient(dat.yaw) ./ gradient(t), 3, 101);
end

