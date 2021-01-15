#ifndef SD_CMD_H
#define SD_CMD_H

#include <metal/spi.h>

// Reference: http://chlazza.nfshost.com/sdcardinfo.html
// Reference: http://alumni.cs.ucr.edu/~amitra/sdcard/Additional/sdcard_appnote_foust.pdf

typedef struct sd_context
{
    char busyflag;

    char *tx_buf;
    char *rx_buf;
    unsigned int buf_size;

    struct metal_spi_config *spi_conf;
    unsigned int timeout_read;
    unsigned int timeout_write;
} sd_context_t;

/*** SD Card Response Types ***/
#define R1   0  // One byte in width.
#define R1b  1  // R1b is the same as R1, except the response token may be followed by zero or more bytes set to zero.
                // This is a busy signal - when the user receives a non-zero byte the card is ready for another command.
#define R2   2  // Two bytes in width. The first byte sent is identical to R1.
#define R3o7 3  // Five bytes in width. The first byte sent is identical to R1.

#define SD_RESPONSE_TYPE_LENGTH_LUT ((const unsigned int []){1, 1, 2, 5})

#define MSK_IDLE            0x01
#define MSK_ERASE_RST       0x02
#define MSK_ILL_CMD         0x04
#define MSK_CRC_ERR         0x08
#define MSK_ERASE_SEQ_ERR   0x10
#define MSK_ADDR_ERR        0x20
#define MSK_PARAM_ERR       0x40

#define SD_TOK_READ_STARTBLOCK 0xFE
#define SD_TOK_WRITE_STARTBLOCK 0xFE
#define SD_TOK_READ_STARTBLOCK_M 0xFE
#define SD_TOK_WRITE_STARTBLOCK_M 0xFC
#define SD_TOK_STOP_MULTI 0xFD
/* Error token is 111XXXXX */
#define MSK_TOK_DATAERROR 0xE0
/* Bit fields */
#define MSK_TOK_ERROR 0x01
#define MSK_TOK_CC_ERROR 0x02
#define MSK_TOK_ECC_FAILED 0x04
#define MSK_TOK_CC_OUTOFRANGE 0x08
#define MSK_TOK_CC_LOCKED 0x10
/* Mask off the bits in the OCR corresponding to voltage range 3.2V to
* 3.4V, OCR bits 20 and 21 */
#define MSK_OCR_33 0xC0
/* Number of times to retry the probe cycle during initialization */
#define SD_INIT_TRY 50
/* Number of tries to wait for the card to go idle during initialization */
#define SD_IDLE_WAIT_MAX 100
/* Hardcoded timeout for commands. 8 words, or 64 clocks. Do 10
* words instead */
#define SD_CMD_TIMEOUT 100

/*** Suported Commands in SPI Mode ***/
#define GO_IDLE_STATE               0
#define R_GO_IDLE_STATE                 R1
#define SEND_OP_COND                1
#define R_SEND_OP_COND                  R1
#define SEND_IF_COND                8
#define R_SEND_IF_COND                  R3o7
#define SEND_CSD                    9
#define R_SEND_CSD                      R1
#define SEND_CID                    10
#define R_SEND_CID                      R1
#define STOP_TRANSMISSION           12
#define R_STOP_TRANSMISSION             R1b
#define SEND_STATUS                 13
#define R_SEND_STATUS                   R2
#define SET_BLOCKLEN                16
#define R_SET_BLOCKLEN                  R1
#define READ_SINGLE_BLOCK           17
#define R_READ_SINGLE_BLOCK             R1
#define READ_MULTIPLE_BLOCK         18
#define R_READ_MULTIPLE_BLOCK           R1
#define SET_BLOCK_COUNT             23
#define R_SET_BLOCK_COUNT               R1
#define WRITE_BLOCK                 24
#define R_WRITE_BLOCK                   R1
#define WRITE_MULTIPLE_BLOCK        25
#define R_WRITE_MULTIPLE_BLOCK          R1
#define PROGRAM_CSD                 27
#define R_PROGRAM_CSD                   R1
#define SET_WRITE_PROT              28
#define R_SET_WRITE_PROT                R1b
#define CLR_WRITE_PROT              29
#define R_CLR_WRITE_PROT                R1b
#define SEND_WRITE_PROT             30
#define R_SEND_WRITE_PROT               R1
#define ERASE_WR_BLK_START_ADDR     32
#define R_ERASE_WR_BLK_START_ADDR       R1
#define ERASE_WR_BLK_END_ADDR       33
#define R_ERASE_WR_BLK_END_ADDR         R1
#define ERASE                       38
#define R_ERASE                         R1b
#define APP_CMD                     55
#define R_APP_CMD                       R1
#define READ_OCR                    58
#define R_READ_OCR                      R3o7
#define CRC_ON_OFF                  59
#define R_SEND_WRITE_PROT               R1

/*** Application specific commands ***/
#define SD_STATUS                   13
#define R_SD_STATUS                     R2
#define SEND_NUM_WR_BLOCKS          22
#define R_SEND_NUM_WR_BLOCKS            R1
#define SET_WR_BLOCK_ERASE_COUNT    23
#define R_SET_WR_BLOCK_ERASE_COUNT      R1
#define SD_SEND_OP_COND             41
#define R_SD_SEND_OP_COND               R1
#define SET_CLR_CARD_DETECT         42
#define R_SET_CLR_CARD_DETECT           R1
#define SEND_SCR                    51
#define R_SEND_SCR                      R1

#endif
