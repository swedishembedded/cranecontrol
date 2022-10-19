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

#include <math.h>

#define normalize_angle(angle) atan2f(sinf(angle), cosf(angle))

#define scale_range32(x, a, b, c, d) ((int32_t)(x) * ((d) - (c)) / ((b) - (a)))

#define deg2rad(x) ((float)(x)*3.14f / 180.f)
#define rad2deg(x) ((float)(x) / (3.14f / 180.f))

#define RAD_360 (2 * M_PI)
#define RAD_120 (2 * M_PI / 3)
#define RAD_60 (M_PI / 3)
#define RAD_30 (M_PI / 6)
