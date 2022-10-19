#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libfirmware/console.h>

#include "py/runtime.h"
#include "py/gc.h"
#include "py/repl.h"
#include "py/pyexec.h"

static console_device_t _python_con = NULL;
static char python_heap[2048 * 4];

// Receive single character
int mp_hal_stdin_rx_chr(void)
{
	char ch = 0;
	if (_python_con) {
		while (console_read(_python_con, &ch, 1, 100) != 1) {
		}
	}
	return (int)ch;
}

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len)
{
	while (len--)
		console_printf(_python_con, "%c", *str++);
}
void mp_hal_stdout_tx_strn_cooked(const char *str, mp_uint_t len)
{
	while (len--)
		console_printf(_python_con, "%c", *str++);
}
void mp_hal_stdout_tx_str(const char *str)
{
	console_printf(_python_con, "%s", str);
}
int mp_import_stat(const char *file)
{
	return 0;
}

static char *stack_top = NULL;
void gc_collect()
{
	// WARNING: This gc_collect implementation doesn't try to get root
	// pointers from CPU registers, and thus may function incorrectly.
	void *dummy;
	gc_collect_start();
	gc_collect_root(&dummy, ((mp_uint_t)stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t));
	gc_collect_end();
	gc_dump_info();
}

mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs)
{
	return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

void nlr_jump_fail(void *val)
{
	printk("jump fail!\n");
	while (1) {
	}
}

static int _py_probe(void *fdt, int fdt_node)
{
	char stack_dummy = 0;
	_python_con = console_find_by_ref(fdt, fdt_node, "console");

	if (!_python_con) {
		printk("python: missing console\n");
		return 0;
	}

	stack_top = &stack_dummy;

	gc_init(python_heap, python_heap + sizeof(python_heap));

	mp_init();
	pyexec_friendly_repl();
	mp_deinit();

	return 0;
}

static int _py_remove(void *fdt, int fdt_node)
{
	// TODO
	return -1;
}

DEVICE_DRIVER(python, "app,python", _py_probe, _py_remove)
