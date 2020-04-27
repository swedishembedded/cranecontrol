#pragma once

#define CANOPEN_FB_DEVICE_STATUS					0x200000
// fault bit is set if drv8302 fault has occured
#define CANOPEN_FB_DEVICE_STATUS_FAULT_BIT			(1 << 0)

/**
 * This registers is a value -1000 to 1000 specifying pitch/yaw demand
 * 0 means center point
 */

#define CANOPEN_FB_PITCH_DEMAND						0x200100
#define CANOPEN_FB_YAW_DEMAND						0x200200
#define CANOPEN_FB_PITCH_CURRENT					0x200300
#define CANOPEN_FB_YAW_CURRENT						0x200400
#define CANOPEN_FB_PITCH_POSITION					0x200500
#define CANOPEN_FB_YAW_POSITION						0x200600
#define CANOPEN_FB_MICROS							0x200700
#define CANOPEN_FB_VMOT								0x200800
