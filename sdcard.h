#ifndef SDCARD_H
#define SDCARD_H

#include "global_handle.h"
#include "sd_cmd.h"

#include <metal/gpio.h>

#include <stdint.h>

#define SD_BLOCKSIZE 512
#define SD_BLOCKSIZE_NBITS 9

#define SS_ASSERT	metal_gpio_set_pin(gpio0, BOARD_PIN_10_SS, 0);
#define SS_DEASSERT	metal_gpio_set_pin(gpio0, BOARD_PIN_10_SS, 1);

/* User functions */
int sd_initialize(sd_context_t *sdc);
int sd_read_block (sd_context_t *sdc, unsigned int blockaddr, char * const data);
int sd_write_block (sd_context_t *sdc, unsigned int blockaddr, char * const data);
void sd_wait_notbusy (sd_context_t *sdc);

/* Internal functions, used for SD card communications. */
void sd_packarg(unsigned char *argument, uint32_t value);
int sd_set_blocklen (sd_context_t *sdc, uint32_t length);
int sd_send_command(sd_context_t *sdc, unsigned char cmd, unsigned int response_type, unsigned char *response, int arg);
void sd_delay(sd_context_t *sdc, const unsigned int number);

#endif
