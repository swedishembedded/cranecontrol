#pragma once

#define vt_goto_col(con, col) console_printf(con, "\033[%dG", col)
#define vt_field_i32(con, val) console_printf(con, "\033[44;37m%10d\033[0m", val)
#define vt_field_fi32(con, base, val)                                                              \
	console_printf(con, "\033[44;37m%5d.%-3d\033[0m", val / base, val % base)
#define vt_field_f32(con, val) tui_field_fi32(con, 1000, val * 1000)
#define STR(str) console_printf(con, "\033[44;37m%10s\033[0m", str)
#define BOOL(val, on, off) console_printf(con, " \033[44;37m%9s\033[0m ", (val) ? on : off)
//#define FLOAT(num) console_printf(con, FIELD, (int)((num * 1000)))
#define NAME(name) console_printf(con, "%s:" TAB, name)
#define NL() console_printf(con, "\n")
#define vt_cursor_goto(con, x, y) console_printf(con, "\033[%d;%dH", y, x)
#define vt_cursor_home(con) console_printf(con, "\033[H")
#define vt_clear_screen(con) console_printf(con, "\033[2J")
#define BLUE_BG() console_printf(con, "\033[44;37m")
#define DEFAULT_BG() console_printf(con, "\033[0m")
#define CURSOR_OFF() console_printf(con, "\033[?25l")
#define CURSOR_ON() console_printf(con, "\033[?25h")

#define CHOICE(val, choice)                                                                        \
	do {                                                                                       \
		if ((unsigned)(val) < (sizeof(choice) / sizeof(choice[0])))                        \
			STR(choice[(unsigned)(val)]);                                              \
	} while (0)
