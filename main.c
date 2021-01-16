/* Copyright 2019 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include "global_handle.h"
#include "sdcard.h"

#include <metal/clock.h>
#include <metal/cpu.h>
#include <metal/gpio.h> //include GPIO library, https://sifive.github.io/freedom-metal-docs/apiref/gpio.html
#include <metal/init.h>
#include <metal/machine.h>
#include <metal/spi.h>
#include <metal/time.h>       //include Time library

#include <stdio.h>      //include Serial Library

struct metal_spi_config *conf;

unsigned char spi_tx_buf[600];
unsigned char spi_rx_buf[600];

int main() {
	printf("[T+%08lu] *** RED-V SD over SPI Test ***\r\n", (unsigned long) metal_time());

	sd_context_t sdc;

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
	sdc.spi_conf = &sd_conf;
	sdc.timeout_read = 400000 / 20; // 400kHz rate -> 400ms timeout
	sdc.timeout_write = 400000 / 8; // 400kHz rate -> 1s timeout
	sdc.busyflag = 0;
	sdc.tx_buf = spi_tx_buf;
	sdc.rx_buf = spi_rx_buf;
	sdc.buf_size = 600;

	/* clear SPI buffers */
	for (unsigned int i = 0; i < sdc.buf_size; ++i) {
		spi_tx_buf[i] = spi_rx_buf[i] = 0xFF;
	}

	sd_initialize(&sdc);

	while (1);
	return 0;
}
