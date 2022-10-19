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

#include "../serial.h"

struct linux_serial;

struct linux_serial *linux_serial_new();
void linux_serial_delete(struct linux_serial *self);

int linux_serial_connect(struct linux_serial *self, const char *serial_dev, unsigned baud);
int linux_serial_disconnect(struct linux_serial *self);

int linux_serial_read(struct linux_serial *self, void *data, size_t size, unsigned timeout_us);
int linux_serial_write(struct linux_serial *self, const void *data, size_t size, unsigned tout);

serial_device_t linux_serial_as_serial_device(struct linux_serial *self);
