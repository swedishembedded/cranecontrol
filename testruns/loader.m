pkg load control miscellaneous

function [dat] = load_data(fname)
    data = dlmread(fname, ";");

    time = ((data(:,1) - data(1, 1)) * 1e-6);
    dat.vpitch = data(:, 2) / 10000;
    dat.vyaw = data(:, 3) / 10000;
    dat.ipitch = data(:, 4) / 10000;
    dat.iyaw = data(:, 5) / 10000;
    dat.joy_pitch = data(:, 6) / 10000;
    dat.joy_yaw = data(:,7) / 10000;
    dat.pitch = data(:,8) / 10000;
    dat.yaw = data(:,9) / 1000;

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

    dat.pitch_vel = sgolayfilt(gradient(dat.pitch) ./ gradient(t), 3, 41);
    dat.yaw_vel = sgolayfilt(gradient(dat.yaw) ./ gradient(t), 3, 101);
end

