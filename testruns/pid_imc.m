function [p, i, d] = pid_imc(aggr, process_gain, time_constant, dead_time)
	tc = max((0.1 * aggr) * time_constant, (0.8 * aggr) * dead_time);
	kc = (1.0/process_gain)*((time_constant + 0.5 * dead_time)/(tc + 0.5 * dead_time));
	ti = time_constant + 0.5 * dead_time;
	td = (time_constant * dead_time)/(2.0 * time_constant + dead_time);

	p = kc;
	i = kc / ti;
	d = kc * td;
end
