/** :ms-top-comment
 *  _     ___ ____  _____ ___ ____  __  ____        ___    ____  _____
 * | |   |_ _| __ )|  ___|_ _|  _ \|  \/  \ \      / / \  |  _ \| ____|
 * | |    | ||  _ \| |_   | || |_) | |\/| |\ \ /\ / / _ \ | |_) |  _|
 * | |___ | || |_) |  _|  | ||  _ <| |  | | \ V  V / ___ \|  _ <| |___
 * |_____|___|____/|_|   |___|_| \_\_|  |_|  \_/\_/_/   \_\_| \_\_____|
 *
 * Copyright (c) 2020, Martin K. Schröder, All Rights Reserved
 *
 * This library is distributed under LGPLv2
 *
 * Commercial licensing: http://swedishembedded.com/code
 * Contact: info@swedishembedded.com
 **/
#pragma once

#include <stdint.h>

#include "serial.h"

struct linux_uart;

struct linux_uart *linux_uart_new();
void linux_uart_delete(struct linux_uart *self);
int linux_uart_write(struct linux_uart *self, const void *_frame, size_t size, uint32_t tout);
int linux_uart_read(struct linux_uart *self, void *frame, size_t max_size, uint32_t tout_ms);
const char *linux_uart_get_ptsname(const struct linux_uart *self);
serial_device_t linux_uart_as_serial_device(struct linux_uart *self);
