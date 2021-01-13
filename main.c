/* Copyright 2019 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <metal/clock.h>
#include <metal/cpu.h>
#include <metal/gpio.h> //include GPIO library, https://sifive.github.io/freedom-metal-docs/apiref/gpio.html
#include <metal/init.h>
#include <metal/machine.h>
#include <metal/spi.h>
#include <metal/time.h>       //include Time library

#include <stdio.h>      //include Serial Library

#define RTC_FREQ    32768
#define BOARD_PIN_10_SS 2

struct metal_cpu *cpu;
struct metal_interrupt *cpu_intr, *tmr_intr;
int tmr_id;
volatile uint32_t timer_isr_flag;

struct metal_gpio *gpio;

struct metal_spi *spi;
struct metal_spi_config *conf;
unsigned char spi_tx_buf[600] = { 0 };
unsigned char spi_rx_buf[600] = { 0 };

#define SS_DISABLE	metal_gpio_set_pin(gpio, BOARD_PIN_10_SS, 1);
#define SS_ENABLE	metal_gpio_set_pin(gpio, BOARD_PIN_10_SS, 0);

void timer_isr(int id, void *data)
{
    metal_interrupt_disable(tmr_intr, tmr_id); // Disable Timer interrupt
    timer_isr_flag = 1; // Flag showing we hit timer ISR
}

unsigned char crc7shl1or1(const unsigned char * const buf, const unsigned int len)
{
	const unsigned char poly = 0x89;
	unsigned char crc = 0;
	for (unsigned int i = 0; i < len; i++) {
		crc ^= buf[i];
		for (int j = 0; j < 8; j++) {
			crc = (crc & 0x80U) ? ((crc << 1) ^ (poly << 1)) : (crc << 1);
		}
	}
	return crc | 0x01;
}

void sd_CMD(const unsigned char cmd_num, uint32_t arg)
{
	spi_tx_buf[0] = 0x40 | cmd_num; // CMD0
	spi_tx_buf[4] = 0xFF & arg; arg >>= 8; // SD spi bigE
	spi_tx_buf[3] = 0xFF & arg; arg >>= 8;
	spi_tx_buf[2] = 0xFF & arg; arg >>= 8;
	spi_tx_buf[1] = 0xFF & arg; arg >>= 8;
	spi_tx_buf[5] = crc7shl1or1(spi_tx_buf, 5);

	SS_ENABLE;
	metal_spi_transfer(spi, conf, 6, spi_tx_buf, spi_rx_buf);

	printf("[T+%08lu] [CMD%d] ", (unsigned long) metal_time(), (signed int) cmd_num);
	for (unsigned short i = 0; i < 6; ++i) printf("%02X ", (unsigned char)spi_tx_buf[i]); printf(" |  ");
	for (unsigned short i = 0; i < 6; ++i) printf("%02X ", (unsigned char)spi_rx_buf[i]); printf("\r\n");

	spi_tx_buf[0] = 0xFF;
	do {
		metal_spi_transfer(spi, conf, 1, spi_tx_buf, spi_rx_buf);
		printf("%02X ", (unsigned char)spi_tx_buf[0]); printf(" |  ");
		printf("%02X ", (unsigned char)spi_rx_buf[0]); printf("\r\n");
	} while (spi_rx_buf[0] & 0x80);

	printf("[T+%08lu] [CMD%d] Response!\r\n", (unsigned long) metal_time(), (signed int) cmd_num);

	SS_DISABLE;
	while (1);
}

void sd_init()
{
	/* wait 1s */
	printf("[T+%08lu] Initializing SD card...\r\n", (unsigned long) metal_time());
	timer_isr_flag = 0; // clear global timer ISR flag
	metal_cpu_set_mtimecmp(cpu, metal_cpu_get_mtime(cpu) + RTC_FREQ/1000); // plus a second
	metal_interrupt_enable(tmr_intr, tmr_id); // Enable Timer interrupt
	while (timer_isr_flag == 0); // do nothing while waiting
	timer_isr_flag = 0; // clear global timer ISR flag

	/* start initialization SS high and at least 74 SCK cycles */
	SS_DISABLE; // pin should still be high from constructor but we have this here just in case
	metal_spi_transfer(spi, conf, 100, spi_tx_buf, spi_rx_buf); // this sends 800 cycles, which satisfies the requirements

	sd_CMD(0, 0x00000000);
}



int main() {
	printf("[T+%08lu] *** RED-V SD over SPI Test ***\r\n", (unsigned long) metal_time());

	cpu = metal_cpu_get(0);
	cpu_intr = metal_cpu_interrupt_controller(cpu);
	tmr_intr = metal_cpu_timer_interrupt_controller(cpu);
	tmr_id = metal_cpu_timer_get_interrupt_id(cpu);

	gpio = metal_gpio_get_device(0);
	spi = metal_spi_get_device(1); // spi should already be initialized by constructor in init.c

	// https://github.com/westerndigitalcorporation/RISC-V-Linux/blob/master/linux/Documentation/spi/spi-summary
	// CPOL = 0, CPHA = 0, MSB-first, CS active low
	struct metal_spi_config sd_conf =
	{
		.protocol = METAL_SPI_SINGLE,
		.polarity = 0,
		.phase = 0,
		.cs_active_high = 0,
		.csid = 0,
		.cmd_num = 0,
		.addr_num = 0,
		.dummy_num = 0
	};
	conf = &sd_conf;

	/* clear SPI buffers */
	for (size_t i = 0; i < 600; ++i){
		spi_tx_buf[i] = spi_rx_buf[i] = 0xFF;
	}

	sd_init();

	return 0;
}

