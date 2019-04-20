pkg load signal control

function [G] = build_filter(name, freq, dt)
	[b, a] = butter(2, freq * 2 * pi, 's');
	G = c2d(tf(b, a), dt);
	[n, d] = tfdata(G);
	printf('%s filter: \n', name);
	printf('(struct fb_filter_config){.a0 = %d, ', d{1}(2));
	printf('.a1 = %d, ', d{1}(3));
	printf('.b0 = %d, ', n{1}(1));
	printf('.b1 = %d},', n{1}(2));
	printf('// cutoff %dHz\n', freq);
end

build_filter('Pitch', 50, 0.001)
G = build_filter('Pitch velocity', 10, 0.001)
bode(G);

input("..");
