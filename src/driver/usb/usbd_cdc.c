/**
 * SPDX-License-Identifier: GPLv2
 *   ____                       ____            _             _
 *  / ___|_ __ __ _ _ __   ___ / ___|___  _ __ | |_ _ __ ___ | |
 * | |   | '__/ _` | '_ \ / _ \ |   / _ \| '_ \| __| '__/ _ \| |
 * | |___| | | (_| | | | |  __/ |__| (_) | | | | |_| | | (_) | |
 *  \____|_|  \__,_|_| |_|\___|\____\___/|_| |_|\__|_|  \___/|_|
 *
 * Copyright (c) 2015-2022, Martin K. Schröder, All Rights Reserved
 *
 * CraneControl is distributed under GPLv2
 *
 * Embedded Systems Training: https://swedishembedded.com/training
 * Free Embedded Insights: https://swedishembedded.com/tag/insights
 **/

#include <errno.h>

#include <libfirmware/driver.h>
#include <libfirmware/usb.h>
#include <libfirmware/usb_cdc.h>
#include <libfirmware/serial.h>

#include <libfdt/libfdt.h>

#define USB_CDC_DEFAULT_VENDOR_ID 0x25AE
#define USB_CDC_DEFAULT_PRODUCT_ID 0x24AB
#define USB_CDC_CMD_PACKET_SIZE 8 /* Control Endpoint Packet size */
#define USB_CDC_DATA_FS_CMD_PACKET_SIZE 16 /* Endpoint IN & OUT Packet size */
#define USB_CDC_DATA_FS_MAX_PACKET_SIZE 64 /* Endpoint IN & OUT Packet size */

// endpoint indices
#define USB_CDC_DATA_IN_ENDP 2
#define USB_CDC_DATA_OUT_ENDP 3
#if 0
static const struct usb_device_descriptor _default_device_desc = {
    .bLength = 18,                        //    bLength
    .bDescriptorType = USB_DESC_TYPE_DEVICE,      //    bDescriptorType
    .bcdUSBL = 0x10,                      //    bcdUSB
    .bcdUSBH = 0x01,                      //    bcdUSB
    .bDeviceClass = 0,                    //    bDeviceClass
    .bDeviceSubClass = 0,                 //    bDeviceSubClass
    .bDeviceProtocol = 0,                         //    bDeviceProtocol
    .bMaxPacketSize0 = USB_CDC_CMD_PACKET_SIZE,    //    bMaxPacketSize0
    .idVendorL = USB_LOBYTE(USB_CDC_DEFAULT_VENDOR_ID),  //    idVendor
    .idVendorH = USB_HIBYTE(USB_CDC_DEFAULT_VENDOR_ID),  //    idVendor
    .idProductL = USB_LOBYTE(USB_CDC_DEFAULT_PRODUCT_ID), //    idProduct
    .idProductH = USB_HIBYTE(USB_CDC_DEFAULT_PRODUCT_ID), //    idProduct
    .bcdDeviceL = 0x00,                      //    bcdDevice
    .bcdDeviceH = 0x01,                      //    bcdDevice
    .iManufacturer = 1,                      //    iManufacturer
    .iProduct = 2,                           //    iProduct
    .iSerialNumber = 3,                      //    iSerialNumbert
    .bNumConfigurations = 1                  //    bNumConfigurations
};
#endif
const uint8_t USB_DEVICE_QR_DESC[] = {
	(uint8_t)10, //    bLength
	(uint8_t)0x06, //    bDescriptorType
	(uint8_t)0x00, //    bcdUSB
	(uint8_t)0x02, //    bcdUSB
	(uint8_t)USB_DEV_CLASS_COMM, //    bDeviceClass
	(uint8_t)0, //    bDeviceSubClass
	(uint8_t)0, //    bDeviceProtocol
	(uint8_t)8, //    bMaxPacketSize0
	(uint8_t)1, //    bNumConfigurations
	(uint8_t)0 // reserved
};

struct usb_cdc {
	struct serial_device dev;
	usbd_device_t usbd;
};

#if 0
static int _usb_cdc_command_isr(struct usbd_device *dev, struct usb_setup_packet *packet) {
    switch (packet->wRequestAndType) {
    case USB_DEVICE_CDC_REQUEST_SET_LINE_CODING:
        //usbd_write(self->usbd, 0, 0, 0);
        break;
    case USB_DEVICE_CDC_REQUEST_SET_CONTROL_LINE_STATE:
        //usbd_write(self->usbd, 0, 0, 0);
        break;
    default:
        return -EINVAL;
    }
    return 0;
}
#endif
static int _serial_write(serial_port_t dev, const void *data, size_t size, uint32_t timeout)
{
	struct usb_cdc *self = container_of(dev, struct usb_cdc, dev.ops);
	if (!self)
		return -1;

	// send data to the host
	//return usbd_write(self->usbd, USB_CDC_IN_EP, data, size, timeout);
	return 0;
}

static int _serial_read(serial_port_t dev, void *data, size_t size, uint32_t timeout)
{
	struct usb_cdc *self = container_of(dev, struct usb_cdc, dev.ops);
	if (!self)
		return -1;

	// receive data in full if possible or timeout at 1 or more received bites
	//return usbd_read(self->usbd, USB_CDC_OUT_EP, data, size, timeout);
	return 0;
}

static const struct serial_device_ops _serial_ops = { .read = _serial_read,
						      .write = _serial_write };

static int _usb_cdc_probe(void *fdt, int fdt_node)
{
	int usb_node = fdt_find_node_by_ref(fdt, fdt_node, "usb");
	if (usb_node < 0) {
		dbg_printk("USBCDC: no usb node!\n");
		return -1;
	}

	usbd_device_t usbd = usbd_find_by_node(usb_node);
	if (!usbd) {
		dbg_printk("USBCDC: nousb\n");
		return -1;
	}

	struct usb_cdc *self = kzmalloc(sizeof(struct usb_cdc));
	self->usbd = usbd;

	serial_device_init(&self->dev, fdt, fdt_node, &_serial_ops);
	serial_device_register(&self->dev);

	return 0;
}

static int _usb_cdc_remove(void *fdt, int fdt_node)
{
	return 0;
}

DEVICE_DRIVER(usbd_serial, "fw,usbd_cdc", _usb_cdc_probe, _usb_cdc_remove)
