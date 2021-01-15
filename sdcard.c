#include "sdcard.h"

#include "global_handle.h"
#include "sd_cmd.h"

#define SD_DEBUG_PRINT
#ifdef SD_DEBUG_PRINT
#include <metal/time.h>
#include <stdio.h>
#endif

static unsigned char response[5];

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

int sd_initialize(sd_context_t *sdc)
{
    /* wait 1s */
#ifdef SD_DEBUG_PRINT
    printf("[T+%08lu] Initializing SD card...\r\n", (unsigned long) metal_time());
#endif

	/* wait 1s */
	timer_isr_flag = 0; // clear global timer ISR flag
	metal_cpu_set_mtimecmp(cpu0, metal_cpu_get_mtime(cpu0) + RTC_FREQ/1000); // plus a second
	metal_interrupt_enable(tmr_intr, tmr_id); // Enable Timer interrupt
	while (timer_isr_flag == 0); // do nothing while waiting
	timer_isr_flag = 0; // clear global timer ISR flag

    /* start initialization SS high and at least 74 SCK cycles */
    SS_DEASSERT; // pin should still be high from constructor but we have this here just in case
	sd_delay(sdc, 100); // this sends 800 cycles, which is more than enough

    sdc->busyflag = 0;

    /* Put the card in the idle state. Early return if unsuccessful. */
    if (sd_send_command(sdc, GO_IDLE_STATE, R_GO_IDLE_STATE, response, 0) == 0)
        return 0;

    sd_send_command(sdc, GO_IDLE_STATE, R_GO_IDLE_STATE, response, 0);
    sd_delay(sdc, 100);

    sd_send_command(sdc, SEND_IF_COND, R_SEND_IF_COND, response, 0x1AA);
    sd_delay(sdc, 100);


    sd_send_command(sdc, APP_CMD, R_APP_CMD, response, 0);
    sd_delay(sdc, 100);
    sd_send_command(sdc, SD_SEND_OP_COND, R_SD_SEND_OP_COND, response, 0x40000000);
    sd_delay(sdc, 100);
}

int sd_send_command(sd_context_t *sdc, unsigned char cmd, unsigned int response_type, unsigned char *response, int arg)
{
    sdc->tx_buf[0] = 0x40 | cmd;
	sdc->tx_buf[4] = 0xFF & arg; arg >>= 8; // SD spi big endian
	sdc->tx_buf[3] = 0xFF & arg; arg >>= 8;
	sdc->tx_buf[2] = 0xFF & arg; arg >>= 8;
	sdc->tx_buf[1] = 0xFF & arg; arg >>= 8;
	sdc->tx_buf[5] = crc7shl1or1(sdc->tx_buf, 5);

    const unsigned int response_length = SD_RESPONSE_TYPE_LENGTH_LUT[response_type];

    /* send the command */
    SS_ASSERT;
    metal_spi_transfer(spi1, sdc->spi_conf, 6, sdc->tx_buf, sdc->rx_buf);
    #ifdef SD_DEBUG_PRINT
        printf("[T+%08lu] [SPI] [CMD%d] ", (unsigned long) metal_time(), (signed int) cmd);
        for (unsigned short i = 0; i < 6; ++i) printf("%02X ", (unsigned char)sdc->tx_buf[i]); printf(" |  ");
        for (unsigned short i = 0; i < 6; ++i) printf("%02X ", (unsigned char)sdc->rx_buf[i]); printf("\r\n");
    #endif

    /* wait for a response */
    sdc->tx_buf[0] = 0xFF;
    unsigned int i = 0;
    do {
        metal_spi_transfer(spi1, sdc->spi_conf, 1, sdc->tx_buf, sdc->rx_buf);
        #ifdef SD_DEBUG_PRINT
            printf("[T+%08lu] [SPI] [WAIT] %02X  |  %02X\r\n", (unsigned long) metal_time(), (unsigned char)sdc->tx_buf[0], (unsigned char)sdc->rx_buf[0]);
        #endif

        ++i;
        if (i >= SD_CMD_TIMEOUT) {
            SS_DEASSERT;
            return 0;
        }
    } while (sdc->rx_buf[0] & 0x80);

    /* handle responses that are longer than 1 byte */
    for (i = 1; i < response_length; ++i) {
        sdc->tx_buf[i] = 0xFF;
        metal_spi_transfer(spi1, sdc->spi_conf, response_length - 1, sdc->tx_buf + 1, sdc->rx_buf + 1);
        sdc->rx_buf[i];
    }

    #ifdef SD_DEBUG_PRINT
        printf("[T+%08lu] [SPI] [RESP] ", (unsigned long) metal_time(), (signed int) cmd);
        for (unsigned short i = 0; i < response_length; ++i) printf(" %02X", (unsigned char)sdc->rx_buf[i]); printf("\r\n");
    #endif

    /* If the response is a "busy" type (R1B), then there’s some
     * special handling that needs to be done. The card will
     * output a continuous stream of zeros, so the end of the BUSY
     * state is signaled by any nonzero response. The bus idles
     * high.
     */
    if (response_type == R1b) {
        char tmp;
        #ifdef SD_DEBUG_PRINT
            printf("[T+%08lu] [SPI] [R1b] busy", (unsigned long) metal_time(), (signed int) cmd);
        #endif
        do {
            metal_spi_transfer(spi1, sdc->spi_conf, 1, sdc->tx_buf, &tmp);
            #ifdef SD_DEBUG_PRINT
                printf(".");
            #endif
        } while (!tmp);
        #ifdef SD_DEBUG_PRINT
            printf("\r\n");
        #endif
    }

    SS_DEASSERT;

    return 1;
}

void sd_delay(sd_context_t *sdc, const unsigned int number)
{
    char tmp_tx = 0xFF, tmp_rx;
    for (unsigned int i = 0; i < number; ++i)
        metal_spi_transfer(spi1, sdc->spi_conf, 1, &tmp_tx, &tmp_rx);
}
