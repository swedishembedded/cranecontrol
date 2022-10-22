/**
 * SPDX-License-Identifier: GPLv2
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2019-2022, Martin K. Schröder, All Rights Reserved
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

// Include required definitions first.
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "fb.h"

struct fb *_python_fb_ptr;

// This is the function which will be called from Python as example.add_ints(a, b).
STATIC mp_obj_t mp_fb_readswitch(mp_obj_t o_idx)
{
	int idx = mp_obj_get_int(o_idx);
	if (_python_fb_ptr && idx >= 0 && idx < 8) {
		int val = (int)gpio_read(_python_fb_ptr->inputs.sw_gpio, (uint32_t)(8 + idx));
		return mp_obj_new_int(val);
	}
	return mp_obj_new_int(0);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_fb_readswitch_obj, mp_fb_readswitch);

STATIC const mp_rom_map_elem_t fb_module_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_fb) },
	{ MP_ROM_QSTR(MP_QSTR_readswitch), MP_ROM_PTR(&mp_fb_readswitch_obj) },
};
STATIC MP_DEFINE_CONST_DICT(fb_module_globals, fb_module_globals_table);

const mp_obj_module_t fb_cmodule = {
	.base = { &mp_type_module },
	.globals = (mp_obj_dict_t *)&fb_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_fb, fb_cmodule, 1);
