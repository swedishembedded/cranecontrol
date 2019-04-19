pkg load signal control

function [G] = build_filter(name, freq, dt)
	[b, a] = butter(2, freq, 's');
	G = c2d(tf(b, a), dt);
	[n, d] = tfdata(G);
	printf('%s filter: \n', name);
	printf('float a0 = %d;\n', d{1}(2));
	printf('float a1 = %d;\n', d{1}(3));
	printf('float b0 = %d;\n', n{1}(1));
	printf('float b1 = %d;\n', n{1}(2));
end

build_filter('Pitch', 400, 0.001)
build_filter('Pitch velocity', 40, 0.001)
