#include <misc.h>
#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_dma.h>

#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include "driver.h"
#include "serial.h"

#include "thread/mutex.h"
#include "thread/thread.h"
#include "thread/queue.h"
#include "thread/sem.h"
#include "thread/event.h"
#include "types/types.h"

#include <libfdt/libfdt.h>

#define EVT_BIT_HT (1 << 0)
#define EVT_BIT_TC (1 << 1)
#define EVT_BIT_IDLE (1 << 2)

struct stm32_uart {
	struct serial_device dev;
	USART_TypeDef *hw;
	struct thread_queue tx_queue;
	struct thread_queue rx_queue;
	struct mutex wr_lock, rd_lock;
	struct semaphore dma_tx_tc_sem;
	struct thread_event rx_evt;
	char *dma_rx_buf, *dma_tx_buf;
	size_t dma_rx_buf_size, dma_tx_buf_size;
	int crlf;
};

#define UART_NUM_DEVICES 8
static struct stm32_uart *_uart_ptr[UART_NUM_DEVICES] = {0, 0, 0, 0, 0, 0};

static int _serial_write(
		serial_port_t serial,
		const void *data,
		size_t size,
        uint32_t timeout) {
	struct stm32_uart *self = container_of(
			serial,
			struct stm32_uart,
			dev.ops);
	if(!self)
		return -1;
	uint8_t *buf = (uint8_t *)data;
	int sent = 0;

	DMA_InitTypeDef dma;
	DMA_StructInit(&dma);
	DMA_Stream_TypeDef *DMAx = DMA2_Stream7;

	dma.DMA_Channel = DMA_Channel_4;
	dma.DMA_Mode = DMA_Mode_Normal;
	dma.DMA_Priority = DMA_Priority_High;
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	dma.DMA_PeripheralBaseAddr = (uint32_t)&self->hw->DR;
	dma.DMA_Memory0BaseAddr = (uint32_t)self->dma_tx_buf;

	thread_mutex_lock(&self->wr_lock);
	for(size_t c = 0; c < size; c+=self->dma_tx_buf_size) {
		size_t t_size = 
			((size - c) > self->dma_tx_buf_size)
			?self->dma_tx_buf_size
			:(size - c);
		memcpy(self->dma_tx_buf, buf + c, t_size);

		// start new dma transfer
		dma.DMA_BufferSize = (uint32_t)t_size;
		DMA_Init(DMAx, &dma);
		DMA_ITConfig(DMAx, DMA_IT_TC, ENABLE);
		DMA_Cmd(DMAx, ENABLE);

		thread_sem_take_wait(&self->dma_tx_tc_sem, 2 * t_size);

		sent += t_size;
	}

	thread_mutex_unlock(&self->wr_lock);
	return sent;
}

static int _serial_read(serial_port_t serial, void *data, size_t size,
                        uint32_t timeout) {
	struct stm32_uart *self = container_of(serial, struct stm32_uart, dev.ops);
	if(!self)
		return -1;
	char *buf = (char *)data;
	int pos = 0;
	// pop characters off the queue
	thread_mutex_lock(&self->rd_lock);

	USART_ITConfig(self->hw, USART_IT_IDLE, ENABLE);

	DMA_InitTypeDef dma;
	DMA_StructInit(&dma);
	DMA_Stream_TypeDef *DMAx = DMA2_Stream5;

	dma.DMA_Channel = DMA_Channel_4;
	dma.DMA_Mode = DMA_Mode_Normal;
	dma.DMA_Priority = DMA_Priority_High;
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
	dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
	dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	dma.DMA_PeripheralBaseAddr = (uint32_t)&self->hw->DR;
	dma.DMA_Memory0BaseAddr = (uint32_t)self->dma_rx_buf;

	while(size){
		dma.DMA_BufferSize = (uint32_t)(size > self->dma_rx_buf_size)?self->dma_rx_buf_size:size;

		DMA_Cmd(DMAx, DISABLE);
		DMA_Init(DMAx, &dma);
		DMA_ITConfig(DMAx, DMA_IT_TC, ENABLE);
		DMA_Cmd(DMAx, ENABLE);

		thread_evt_wait(&self->rx_evt, EVT_BIT_TC, timeout);

		memcpy(buf + pos, self->dma_rx_buf, dma.DMA_BufferSize);
		pos+=dma.DMA_BufferSize;
		size-=dma.DMA_BufferSize;
	}

	thread_mutex_unlock(&self->rd_lock);
	if(pos == 0)
		return -ETIMEDOUT;
	return pos;
}

static inline struct stm32_uart *_get_hw(uint8_t id) {
	if(id == 0 || id > UART_NUM_DEVICES)
		return NULL;
	return _uart_ptr[id - 1];
}

static int32_t _uart_irq(struct stm32_uart *self) {
	if(!self) {
		return 0;
	}

	int32_t wake = 0;

	// check for idle line detected (reception completed)
	if(USART_GetITStatus(self->hw, USART_IT_IDLE)) {
		USART_ClearITPendingBit(self->hw, USART_IT_IDLE);
		USART_ITConfig(self->hw, USART_IT_IDLE, DISABLE);
		thread_evt_set_from_isr(&self->rx_evt, EVT_BIT_IDLE, &wake);
	}

	// we check for incoming data on this device, ack the interrupt and copy data into
	// the queue
	if(USART_GetITStatus(self->hw, USART_IT_RXNE)) {
		USART_ClearITPendingBit(self->hw, USART_IT_RXNE);
		//uint16_t t = self->hw->DR;
		//thread_queue_send_from_isr(&self->rx_queue, &t, &wake);
	}

	// we check for transmission read on this device, ack the interrupt and either send
	// next byte or turn off the interrupt
	if(USART_GetITStatus(self->hw, USART_IT_TXE)) {
		USART_ClearITPendingBit(self->hw, USART_IT_TXE);
	}
	return wake;
}

static int32_t _dma_tx_irq(struct stm32_uart *self, DMA_Stream_TypeDef *DMAx) {
	int32_t wake = 0;

	if(DMA_GetITStatus(DMAx, DMA_IT_HTIF7)) {
		DMA_ClearITPendingBit(DMAx, DMA_IT_HTIF7);
		// half transfer not used currently for transmissions
	}

	if(DMA_GetITStatus(DMAx, DMA_IT_TEIF7)) {
		DMA_ClearITPendingBit(DMAx, DMA_IT_TEIF7);
		// TODO: handle error here
		thread_sem_give_from_isr(&self->dma_tx_tc_sem, &wake);
	}

	if(DMA_GetITStatus(DMAx, DMA_IT_TCIF7)) {
		DMA_ClearITPendingBit(DMAx, DMA_IT_TCIF7);
		// signal transfer completed
		thread_sem_give_from_isr(&self->dma_tx_tc_sem, &wake);
	}

	return wake;
}

static int32_t _dma_rx_irq(struct stm32_uart *self, DMA_Stream_TypeDef *DMAx) {
	int32_t wake = 0;
	if(DMA_GetITStatus(DMAx, DMA_IT_HTIF5)) {
		DMA_ClearITPendingBit(DMAx, DMA_IT_HTIF5);
		thread_evt_set_from_isr(&self->rx_evt, EVT_BIT_HT, &wake);
	}

	if(DMA_GetITStatus(DMAx, DMA_IT_TCIF5)) {
		DMA_ClearITPendingBit(DMAx, DMA_IT_TCIF5);
		thread_evt_set_from_isr(&self->rx_evt, EVT_BIT_TC, &wake);
	}
	return wake;
}

void USART1_IRQHandler(void) {
	struct stm32_uart *hw = _get_hw(1);
	int32_t __unused wake = _uart_irq(hw);
	thread_yield_from_isr(wake);
}

void USART2_IRQHandler(void) {
	struct stm32_uart *hw = _get_hw(2);
	int32_t __unused wake = _uart_irq(hw);
	thread_yield_from_isr(wake);
}

void USART3_IRQHandler(void) {
	struct stm32_uart *hw = _get_hw(3);
	int32_t __unused wake = _uart_irq(hw);
	thread_yield_from_isr(wake);
}

void UART4_IRQHandler(void) {
	struct stm32_uart *hw = _get_hw(4);
	int32_t __unused wake = _uart_irq(hw);
	thread_yield_from_isr(wake);
}

void UART5_IRQHandler(void) {
	struct stm32_uart *hw = _get_hw(5);
	int32_t __unused wake = _uart_irq(hw);
	thread_yield_from_isr(wake);
}

void USART6_IRQHandler(void) {
	struct stm32_uart *hw = _get_hw(6);
	int32_t __unused wake = _uart_irq(hw);
	thread_yield_from_isr(wake);
}

void UART8_IRQHandler(void) {
	struct stm32_uart *hw = _get_hw(8);
	int32_t __unused wake = _uart_irq(hw);
	thread_yield_from_isr(wake);
}

void DMA2_Stream5_IRQHandler(void){
	struct stm32_uart *hw = _get_hw(1);
	int32_t __unused wake = _dma_rx_irq(hw, DMA2_Stream5);
	thread_yield_from_isr(wake);
}

void DMA2_Stream7_IRQHandler(void){
	struct stm32_uart *hw = _get_hw(1);
	int32_t __unused wake = _dma_tx_irq(hw, DMA2_Stream7);
	thread_yield_from_isr(wake);
}

static const struct serial_device_ops _serial_ops = {.read = _serial_read,
                                                     .write = _serial_write};


static int _stm32_uart_probe(void *fdt, int fdt_node) {
	// TODO: move this so it's only done once
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	int baud = fdt_get_int_or_default(fdt, (int)fdt_node, "baud", 9600);
	USART_TypeDef *UARTx =
	    (USART_TypeDef *)fdt_get_int_or_default(fdt, (int)fdt_node, "reg", 0);
	int irq = fdt_get_int_or_default(fdt, (int)fdt_node, "interrupt", -1);
	int irq_pre_prio = fdt_get_int_or_default(fdt, (int)fdt_node, "irq_prio", 1);
	int irq_sub_prio = fdt_get_int_or_default(fdt, (int)fdt_node, "irq_sub_prio", 0);
	int tx_queue = fdt_get_int_or_default(fdt, (int)fdt_node, "tx_queue", 64);
	int rx_queue = fdt_get_int_or_default(fdt, (int)fdt_node, "rx_queue", 64);
	DMA_Stream_TypeDef *dma_tx_stream = (DMA_Stream_TypeDef*)fdt_get_int_or_default(fdt, (int)fdt_node, "dma_tx_stream", 0);
	DMA_Stream_TypeDef *dma_rx_stream = (DMA_Stream_TypeDef*)fdt_get_int_or_default(fdt, (int)fdt_node, "dma_rx_stream", 0);
	int def_port = fdt_get_int_or_default(fdt, (int)fdt_node, "printk_port", 0);
	int crlf = fdt_get_int_or_default(fdt, (int)fdt_node, "insert-cr-before-lf", 1);

	if(UARTx == 0) {
		return -EINVAL;
	}

	int idx = -1;
	DMA_InitTypeDef dma_rx, dma_tx;
	DMA_StructInit(&dma_rx);
	DMA_StructInit(&dma_tx);
	if(UARTx == USART1) {
		idx = 1;
		dma_rx_stream = DMA2_Stream5;
		dma_tx_stream = DMA2_Stream7;
		dma_rx.DMA_Channel = DMA_Channel_4;
	} else if(UARTx == USART2) {
		idx = 2;
	} else if(UARTx == USART3) {
		idx = 3;
	} else if(UARTx == UART4) {
		idx = 4;
	} else if(UARTx == UART5) {
		idx = 5;
	} else if(UARTx == USART6) {
		idx = 6;
	} else if(UARTx == UART8) {
		idx = 8;
	}

	if(idx == -1) {
		return -EINVAL;
	}

	struct stm32_uart *self = kzmalloc(sizeof(struct stm32_uart));
	if(!self)
		return -1;

	serial_device_init(&self->dev, fdt, fdt_node, &_serial_ops);

	bool uart_queue_alloc_fail =
	    (thread_queue_init(&self->tx_queue, (size_t)tx_queue, sizeof(char)) < 0 ||
	     thread_queue_init(&self->rx_queue, (size_t)rx_queue, sizeof(char)) < 0);
	BUG_ON(uart_queue_alloc_fail);

	self->dma_rx_buf = kzmalloc(rx_queue);
	memset(self->dma_rx_buf, 'A', rx_queue);
	self->dma_rx_buf_size = rx_queue;
	self->dma_tx_buf = kzmalloc(tx_queue);
	memset(self->dma_tx_buf, 'A', tx_queue);
	self->dma_tx_buf_size = tx_queue;

	if(!self->dma_rx_buf || !self->dma_tx_buf) {
		kfree(self);
		return -1;
	}

	thread_sem_init(&self->dma_tx_tc_sem);
	thread_evt_init(&self->rx_evt);

	thread_mutex_init(&self->wr_lock);
	thread_mutex_init(&self->rd_lock);

	self->hw = UARTx;
	self->crlf = crlf;
	_uart_ptr[idx - 1] = self;

	// configure uart
	USART_InitTypeDef conf;
	USART_StructInit(&conf);
	conf.USART_BaudRate = (uint32_t)baud;
	conf.USART_WordLength = USART_WordLength_8b;
	conf.USART_StopBits = USART_StopBits_1;
	conf.USART_Parity = USART_Parity_No;
	conf.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	conf.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(UARTx, &conf);

	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = DMA2_Stream7_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 1;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
	nvic.NVIC_IRQChannel = DMA2_Stream5_IRQn;
	NVIC_Init(&nvic);

	USART_DMACmd(self->hw, USART_DMAReq_Tx, ENABLE);
	USART_DMACmd(self->hw, USART_DMAReq_Rx, ENABLE);

	// see datasheet
	if(dma_rx_stream){
		/*
		dma_rx.DMA_Mode = DMA_Mode_Circular;
		dma_rx.DMA_Priority = DMA_Priority_High;
		dma_rx.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		dma_rx.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		dma_rx.DMA_MemoryInc = DMA_MemoryInc_Enable;
		dma_rx.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		dma_rx.DMA_DIR = DMA_DIR_PeripheralToMemory;
		dma_rx.DMA_BufferSize = rx_queue;
		dma_rx.DMA_FIFOMode = DMA_FIFOMode_Disable;
		dma_rx.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
		dma_rx.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		dma_rx.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		dma_rx.DMA_PeripheralBaseAddr = (uint32_t)&UARTx->DR;
		dma_rx.DMA_Memory0BaseAddr = (uint32_t)self->dma_rx_buf;

		DMA_DeInit(dma_rx_stream);
		DMA_Init(dma_rx_stream, &dma_rx);
		DMA_Cmd(dma_rx_stream, ENABLE);
		USART_DMACmd(UARTx, USART_DMAReq_Rx, ENABLE);
		*/
	}
	if(dma_tx_stream){
		/*
		dma_tx.DMA_Mode = DMA_Mode_Circular;
		dma_tx.DMA_Priority = DMA_Priority_High;
		dma_tx.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		dma_tx.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		dma_tx.DMA_MemoryInc = DMA_MemoryInc_Enable;
		dma_tx.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		dma_tx.DMA_DIR = DMA_DIR_MemoryToPeripheral;
		dma_tx.DMA_BufferSize = rx_queue;
		dma_tx.DMA_FIFOMode = DMA_FIFOMode_Disable;
		dma_tx.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
		dma_tx.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		dma_tx.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		dma_tx.DMA_PeripheralBaseAddr = (uint32_t)&UARTx->DR;
		dma_tx.DMA_Memory0BaseAddr = (uint32_t)self->dma_tx_buf;

		DMA_DeInit(dma_tx_stream);
		DMA_Init(dma_tx_stream, &dma_tx);
		DMA_Cmd(dma_tx_stream, ENABLE);
		USART_DMACmd(UARTx, USART_DMAReq_Tx, ENABLE);
		*/
	}
	if(irq > 0) {
		nvic.NVIC_IRQChannel = (uint8_t)irq;
		nvic.NVIC_IRQChannelPreemptionPriority = (uint8_t)irq_pre_prio;
		nvic.NVIC_IRQChannelSubPriority = (uint8_t)irq_sub_prio;
		nvic.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvic);

		// enable interrupts
		//USART_ITConfig(UARTx, USART_IT_RXNE, ENABLE);
		USART_ITConfig(UARTx, USART_IT_IDLE, ENABLE);
	}

	USART_Cmd(UARTx, ENABLE);

	if(serial_device_register(&self->dev) < 0) {
		return -1;
	}

	if(def_port)
		serial_set_printk_port(&self->dev.ops);

	return 0;
}

static int _stm32_uart_remove(void *fdt, int fdt_node) {
	// TODO
	return -1;
}

DEVICE_DRIVER(stm32_uart, "st,stm32_uart", _stm32_uart_probe, _stm32_uart_remove)
