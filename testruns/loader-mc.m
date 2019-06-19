pkg load control miscellaneous

function [dat] = load_data(fname)
	data = dlmread(fname, ";");

	time = ((data(:,1) - data(1, 1)) * 1e-6);
	dat.out_pitch = data(:, 2) / 10000;
	dat.out_yaw = data(:, 3) / 10000;
	dat.ia_pitch = data(:, 4) / 10000;
	dat.ib_pitch = data(:, 5) / 10000;
	dat.ia_yaw = data(:, 6) / 10000;
	dat.ib_yaw = data(:,7) / 10000;
	dat.user_pitch = data(:,8) / 10000;
	dat.user_yaw = data(:,9) / 10000;
	dat.pitch = data(:,10) / 10000;
	dat.yaw = data(:,11) / 10000;

    t = dat.t = interp1(time, time, linspace(time(1), time(end), (time(end) - time(1)) * 1000));
		dat.out_pitch = interp1(time, dat.out_pitch, t);
    dat.out_yaw = interp1(time, dat.out_yaw, t);
    dat.ia_pitch = interp1(time, dat.ia_pitch, t);
    dat.ib_pitch = interp1(time, dat.ib_pitch, t);
    dat.ia_yaw = interp1(time, dat.ia_yaw, t);
    dat.ib_yaw = interp1(time, dat.ib_yaw, t);
    dat.pitch = interp1(time, dat.pitch, t);
    dat.yaw = interp1(time, dat.yaw, t);

	dt = 0.001;

	[b, a] = butter(4, 5 * 2 * pi, 's');
	[b, a] = tfdata(c2d(tf(b, a), dt));
	%dat.pitch = filtfilt(b{1}, a{1}, dat.pitch);

	[b, a] = butter(4, 10 * 2 * pi, 's');
	[b, a] = tfdata(c2d(tf(b, a), dt));
  dat.pitch_vel = sgolayfilt(gradient(dat.pitch) ./ gradient(t), 3, 41);
	%dat.pitch_vel = filtfilt(b{1}, a{1}, gradient(dat.pitch) / dt);
  dat.yaw_vel = sgolayfilt(gradient(dat.yaw) ./ gradient(t), 3, 101);

end

