#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libfirmware/console.h>

extern void rust_function(console_device_t dev);

static int _rust_probe(void *fdt, int fdt_node) {
	console_device_t con = console_find_by_ref(fdt, fdt_node, "console");
	rust_function(con);
	return 0;
}

static int _rust_remove(void *fdt, int fdt_node) {
	// TODO
	return -1;
}


DEVICE_DRIVER(rust, "app,rust", _rust_probe, _rust_remove)
