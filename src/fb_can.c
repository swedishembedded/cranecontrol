/** :ms-top-comment
 *  ____            _        _   ____  _     ____
 * |  _ \ ___   ___| | _____| |_|  _ \| |   / ___|
 * | |_) / _ \ / __| |/ / _ \ __| |_) | |  | |
 * |  _ < (_) | (__|   <  __/ |_|  __/| |__| |___
 * |_| \_\___/ \___|_|\_\___|\__|_|   |_____\____|
 *
 * Copyright (c) 2020, Martin K. Schröder, All Rights Reserved
 *
 * RocketPLC is distributed under GPLv2
 *
 * Commercial licensing: http://swedishembedded.com/rocketplc
 * Contact: info@swedishembedded.com
 * Secondary email: mkschreder.uk@gmail.com
 **/
#include "fb_can.h"
#include "fb.h"

static int _fb_can_reinit_slaves(struct fb *self) {
	struct canopen_pdo_config conf = {
	    .cob_id = 0x200 | FB_CANOPEN_SLAVE_ADDRESS,
	    .index = 0,
	    .type = CANOPEN_PDO_TYPE_CYCLIC(1),
	    .inhibit_time = 100,
	    .event_time = 100,
	    .map = {CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_PITCH_DEMAND, CANOPEN_PDO_SIZE_16),
	            CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_YAW_DEMAND, CANOPEN_PDO_SIZE_16), 0}};

	if(canopen_pdo_rx(self->canopen_mem, FB_CANOPEN_SLAVE_ADDRESS, &conf) < 0) {
		return -EIO;
	}

	if(canopen_pdo_tx(self->canopen_mem, FB_CANOPEN_MASTER_ADDRESS, &conf) < 0) {
		return -EIO;
	}

	conf = (struct canopen_pdo_config){
	    .cob_id = 0x200 | FB_CANOPEN_MASTER_ADDRESS,
	    .index = 1,
	    .type = CANOPEN_PDO_TYPE_CYCLIC(1),
	    .inhibit_time = 100,
	    .event_time = 100,
	    .map = {CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_PITCH_POSITION, CANOPEN_PDO_SIZE_16),
	            CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_YAW_POSITION, CANOPEN_PDO_SIZE_16),
	            CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_PITCH_CURRENT, CANOPEN_PDO_SIZE_16),
	            CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_YAW_CURRENT, CANOPEN_PDO_SIZE_16),
	            0}};

	if(canopen_pdo_rx(self->canopen_mem, FB_CANOPEN_MASTER_ADDRESS, &conf) < 0) {
		return -EIO;
	}

	if(canopen_pdo_tx(self->canopen_mem, FB_CANOPEN_SLAVE_ADDRESS, &conf) < 0) {
		return -EIO;
	}

	conf.cob_id = 0x210 | FB_CANOPEN_MASTER_ADDRESS, conf.index = 2;
	conf.map[0] = CANOPEN_PDO_MAP_ENTRY(CANOPEN_FB_VMOT, CANOPEN_PDO_SIZE_16);
	conf.map[1] = 0;

	if(canopen_pdo_rx(self->canopen_mem, FB_CANOPEN_MASTER_ADDRESS, &conf) < 0) {
		return -EIO;
	}

	if(canopen_pdo_tx(self->canopen_mem, FB_CANOPEN_SLAVE_ADDRESS, &conf) < 0) {
		return -EIO;
	}

	return 0;
}

static ssize_t _comm_range_read(regmap_range_t range, uint32_t addr,
                                regmap_value_type_t type, void *data, size_t size) {
	// comm_range.ops);
	uint32_t id = addr & 0x00ffff00;
	// uint32_t sub = addr & 0xff;

	// thread_mutex_lock(&self->lock);

	if(id == CANOPEN_REG_DEVICE_TYPE) {
		regmap_convert_u32(402, type, data, size);
		goto success;
	}

	return -ENOENT;
success:
	// thread_mutex_unlock(&self->lock);
	return (ssize_t)size;
}

static ssize_t _comm_range_write(regmap_range_t range, uint32_t addr,
                                 regmap_value_type_t type, const void *data,
                                 size_t size) {
	return -ENOENT;
}

static struct regmap_range_ops _comm_range_ops = {.read = _comm_range_read,
                                                  .write = _comm_range_write};

static ssize_t _mfr_range_read(regmap_range_t range, uint32_t addr,
                               regmap_value_type_t type, void *data, size_t size) {
	struct fb *self = container_of(range, struct fb, mfr_range.ops);
	uint32_t id = addr & 0x00ffff00;
	int ret = -ENOENT;
	switch(id) {
		case CANOPEN_FB_PITCH_DEMAND: {
			if(self->mode == FB_MODE_MASTER) {
				ret = regmap_convert_u32((uint32_t)(int16_t)(self->output.pitch * 1000),
				                         type, data, size);
			} else {
				ret = regmap_convert_u16((uint16_t)self->remote.pitch, type, data, size);
			}
		} break;
		case CANOPEN_FB_YAW_DEMAND: {
			if(self->mode == FB_MODE_MASTER) {
				ret = regmap_convert_u32((uint32_t)(int16_t)(self->output.yaw * 1000), type,
				                         data, size);
			} else {
				ret = regmap_convert_u16((uint16_t)self->remote.yaw, type, data, size);
			}
		} break;
		case CANOPEN_FB_PITCH_CURRENT: {
			ret = regmap_convert_u16((uint16_t)(int16_t)(self->measured.ia_pitch * 1000),
			                         type, data, size);
		} break;
		case CANOPEN_FB_YAW_CURRENT: {
			ret = regmap_convert_u16((uint16_t)(int16_t)(self->measured.ia_yaw * 1000),
			                         type, data, size);
		} break;
		case CANOPEN_FB_PITCH_POSITION: {
			ret = regmap_convert_u16((uint16_t)(int16_t)(self->measured.pitch * 1000),
			                         type, data, size);
		} break;
		case CANOPEN_FB_YAW_POSITION: {
			ret = regmap_convert_u16((uint16_t)(int16_t)(self->measured.yaw * 1000), type,
			                         data, size);
		} break;
		case CANOPEN_FB_MICROS: {
			timestamp_t ts = timestamp();
			ret = regmap_convert_u32(ts.usec, type, data, size);
		} break;
		case CANOPEN_FB_VMOT: {
			uint16_t vmot = (uint16_t)constrain_i32((int32_t)(self->measured.vmot * 1000),
			                                        0, UINT16_MAX);
			ret = regmap_convert_u16(vmot, type, data, size);
		} break;
	}
	return ret;
}

static ssize_t _mfr_range_write(regmap_range_t range, uint32_t addr,
                                regmap_value_type_t type, const void *data,
                                size_t size) {
	struct fb *self = container_of(range, struct fb, mfr_range.ops);
	uint32_t id = addr & 0x00ffffff;
	int ret = -ENOENT;
	timestamp_t update_timeout = timestamp_from_now_us(FB_REMOTE_TIMEOUT);
	switch(id) {
		case CANOPEN_FB_PITCH_CURRENT: {
			int16_t val = 0;
			ret = regmap_mem_to_u16(type, data, size, (uint16_t *)&val);
			thread_mutex_lock(&self->slave.lock);
			self->slave.i_pitch = val;
			self->slave.valid_bits |= FB_SLAVE_VALID_I_PITCH;
			thread_mutex_unlock(&self->slave.lock);
		} break;
		case CANOPEN_FB_YAW_CURRENT: {
			int16_t val = 0;
			ret = regmap_mem_to_u16(type, data, size, (uint16_t *)&val);
			thread_mutex_lock(&self->slave.lock);
			self->slave.i_yaw = val;
			self->slave.valid_bits |= FB_SLAVE_VALID_I_YAW;
			thread_mutex_unlock(&self->slave.lock);
		} break;
		case CANOPEN_FB_PITCH_POSITION: {
			int16_t val = 0;
			ret = regmap_mem_to_u16(type, data, size, (uint16_t *)&val);
			thread_mutex_lock(&self->slave.lock);
			self->slave.pitch = val;
			self->slave.valid_bits |= FB_SLAVE_VALID_PITCH;
			thread_mutex_unlock(&self->slave.lock);
		} break;
		case CANOPEN_FB_YAW_POSITION: {
			int16_t val = 0;
			ret = regmap_mem_to_u16(type, data, size, (uint16_t *)&val);
			thread_mutex_lock(&self->slave.lock);
			self->slave.yaw = val;
			self->slave.valid_bits |= FB_SLAVE_VALID_YAW;
			thread_mutex_unlock(&self->slave.lock);
		} break;
		case CANOPEN_FB_MICROS: {
			uint32_t val = 0;
			ret = regmap_mem_to_u32(type, data, size, &val);
			thread_mutex_lock(&self->slave.lock);
			self->slave.micros = val;
			self->slave.valid_bits |= FB_SLAVE_VALID_MICROS;
			thread_mutex_unlock(&self->slave.lock);
		} break;
		case CANOPEN_FB_PITCH_DEMAND: {
			int16_t pitch = 0;
			regmap_mem_to_u16(type, data, size, (uint16_t *)&pitch);
			self->remote.pitch = constrain_i16(pitch, -1000, 1000);
			self->remote.pitch_update_timeout = update_timeout;
			ret = (int)size;
		} break;
		case CANOPEN_FB_YAW_DEMAND: {
			int16_t yaw = 0;
			regmap_mem_to_u16(type, data, size, (uint16_t *)&yaw);
			self->remote.yaw = constrain_i16(yaw, -1000, 1000);
			self->remote.yaw_update_timeout = update_timeout;
			ret = (int)size;
		} break;
		case CANOPEN_FB_VMOT: {
			uint16_t vmot = 0;
			regmap_mem_to_u16(type, data, size, (uint16_t *)&vmot);
			thread_mutex_lock(&self->slave.lock);
			self->slave.vmot = vmot;
			thread_mutex_unlock(&self->slave.lock);
		} break;
	}

	return ret;
}

static struct regmap_range_ops _mfr_range_ops = {.read = _mfr_range_read,
                                                 .write = _mfr_range_write};


static bool _fb_slave_timed_out(struct fb *self) {
	timestamp_t timeout = timestamp_from_now_ms(FB_SLAVE_TIMEOUT_MS);

	uint32_t mask = FB_SLAVE_VALID_PITCH | FB_SLAVE_VALID_YAW |
	                FB_SLAVE_VALID_I_PITCH | FB_SLAVE_VALID_I_YAW;

	thread_mutex_lock(&self->slave.lock);
	if((self->slave.valid_bits & mask) == mask) {
		// if all items were received then reset the bits and update timestamp
		self->slave.valid_bits = 0;
		self->slave.timeout = timeout;
	} else {
		// otherwise wait until timeout and reconfigure slave
		if(timestamp_expired(self->slave.timeout)) {
			self->slave.valid_bits = 0;
			thread_mutex_unlock(&self->slave.lock);
			return true;
		}
	}
	thread_mutex_unlock(&self->slave.lock);
	return false;
}

void _fb_can_monitor_link_state(struct fb *self) {
	if(self->mode == FB_MODE_SLAVE) {
		if(timestamp_expired(self->remote.pitch_update_timeout) ||
		   timestamp_expired(self->remote.yaw_update_timeout)) {
			self->can.timed_out = true;

			self->remote.pitch_update_timeout = timestamp_from_now_us(FB_REMOTE_TIMEOUT);
			self->remote.yaw_update_timeout = timestamp_from_now_us(FB_REMOTE_TIMEOUT);
		} else {
			self->can.timed_out = false;
		}
	} else if(self->mode == FB_MODE_MASTER){
		if(_fb_slave_timed_out(self)) {
			self->can.timed_out = true;
			fb_can_reinit(self);
		} else {
			self->can.timed_out = false;
		}
	}
}

int fb_can_reinit(struct fb *self){
	if(self->mode == FB_MODE_MASTER){
		return _fb_can_reinit_slaves(self);
	}
	return -1;
}

void fb_can_clock(struct fb *self){
	_fb_can_monitor_link_state(self);
}

void fb_can_init(struct fb *self) {
	// add the device profile map
	regmap_range_init(&self->comm_range, CANOPEN_COMM_RANGE_START,
	                  CANOPEN_COMM_RANGE_END, &_comm_range_ops);
	regmap_add_range(self->regmap, &self->comm_range);

	regmap_range_init(&self->mfr_range, CANOPEN_MFR_RANGE_START, CANOPEN_MFR_RANGE_END,
	                  &_mfr_range_ops);
	regmap_add_range(self->regmap, &self->mfr_range);
}
