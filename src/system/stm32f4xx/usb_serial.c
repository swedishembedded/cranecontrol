#include "driver.h"

#include "console.h"

#include <usb_vcp/usbd_cdc_vcp.h>
#include <usb_vcp/usbd_desc.h>
#include <usb_vcp/usbd_usr.h>

#include "serial.h"

struct usb_serial {
	struct serial_device dev;
};

USB_OTG_CORE_HANDLE USB_OTG_dev;

static int _stm32_usb_cmd(console_device_t con, void *ptr, int argc, char **argv) {
	if(argc == 2 && strcmp(argv[1], "status") == 0) {
		static const struct {
			int bit;
			const char *name;
		} bits[] = {{.bit = 0, "SRQSCS"},  {.bit = 1, "SRQ"},      {.bit = 8, "HNGSCS"},
		            {.bit = 9, "HNPRQ"},   {.bit = 10, "HSHNPEN"}, {.bit = 11, "DHNPEN"},
		            {.bit = 16, "CIDSTS"}, {.bit = 17, "DBCT"},    {.bit = 18, "ASVLD"},
		            {.bit = 19, "BSVLD"}};
		uint32_t status = USB_OTG_dev.regs.GREGS->GOTGCTL;
		for(size_t c = 0; c < sizeof(bits) / sizeof(bits[0]); c++) {
			if(status & (uint32_t)(1 << bits[c].bit)) {
				console_printf(con, "%s: %d\n", bits[c].name, 1);
			} else {
				console_printf(con, "%s: %d\n", bits[c].name, 0);
			}
		}
	}
	return 0;
}

static int _usb_serial_read(serial_port_t dev, void *buf, size_t size,
                            uint32_t timeout) {
	uint32_t cnt = VCP_DataRx((uint8_t *)buf, size);
	if(cnt <= 0)
		return -1;
	return (int)cnt;
}

static int _usb_serial_write(serial_port_t dev, const void *buf, size_t size,
                             uint32_t timeout) {
	uint32_t cnt = 0;
	while(cnt < size) {
		size_t c = VCP_DataTx(buf, (uint8_t)size);
		cnt += c;
	}

	return (int)cnt;
}

static const struct serial_device_ops _usb_serial_ops = {.read = _usb_serial_read,
                                                         .write = _usb_serial_write};

static int _stm32_usb_serial_probe(void *fdt, int fdt_node) {
	console_device_t console = console_find_by_ref(fdt, fdt_node, "console");
	int core = fdt_get_int_or_default(fdt, fdt_node, "core", USB_OTG_FS_CORE_ID);

	if(console) {
		console_add_command(console, NULL, fdt_get_name(fdt, fdt_node, NULL), "", "",
		                    _stm32_usb_cmd);
	}

	USB_OTG_CORE_HANDLE *hw = &USB_OTG_dev;

#ifdef VBUS_SENSING_ENABLED
	// IOConfigGPIO(IOGetByTag(IO_TAG(VBUS_SENSING_PIN)), IOCFG_IN_FLOATING);
#endif

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_PWR, ENABLE);

	EXTI_ClearITPendingBit(EXTI_Line0);

	hw->dev.usr_device = &USR_desc;
	hw->dev.class_cb = &USBD_CDC_cb;
	hw->dev.usr_cb = &USR_cb;

	DCD_Init(hw, core);

	hw->dev.usr_cb->Init();

	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannelPreemptionPriority = 2;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;

	if(core == USB_OTG_HS_CORE_ID) {
		nvic.NVIC_IRQChannel = OTG_HS_IRQn;
		NVIC_Init(&nvic);
		printk("USB: high speed mode (480Mbit)\n");
	} else if(core == USB_OTG_FS_CORE_ID) {
		nvic.NVIC_IRQChannel = OTG_FS_IRQn;
		NVIC_Init(&nvic);
		printk("USB: full speed mode (12Mbit)\n");
	} else {
		printk("usb: unrecognized core option %d\n", core);
		return -EINVAL;
	}

	struct usb_serial *self = kzmalloc(sizeof(struct usb_serial));

	serial_device_init(&self->dev, fdt, fdt_node, &_usb_serial_ops);
	serial_device_register(&self->dev);

	printk(PRINT_SUCCESS "USB: ready\n");
	return 0;
}

static int _stm32_usb_serial_remove(void *fdt, int fdt_node) {
	return -1;
}

DEVICE_DRIVER(stm32_usb, "st,usb-serial", _stm32_usb_serial_probe,
              _stm32_usb_serial_remove)
