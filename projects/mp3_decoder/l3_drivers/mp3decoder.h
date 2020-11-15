
#include "lpc40xx.h"
#include "gpio.h"
#include "gpio_lab.h"
#include <stdio.h>
#include <stdlib.h>
const uint16_t VOLUME[10] = {0X7D7D, 0X6464, 0x4B4B, 0x3C3C, 0x3535, 0x3030, 0x2525, 0x2020, 0x1515};

/* VS1053B SCI Registers */
#define SCI_MODE 0x00
#define SCI_STATUS 0x01
#define SCI_BASS 0x02
#define SCI_CLOCKF 0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA 0x05
#define SCI_WRAM 0x06
#define SCI_WRAMADDR 0x07
#define SCI_HDAT0 0x08
#define SCI_HDAT1 0x09
#define SCI_AIADDR 0x0A
#define SCI_VOL 0x0B
#define SCI_AICTRL0 0x0C
#define SCI_AICTRL1 0x0D
#define SCI_AICTRL2 0x0E
#define SCI_AICTRL3 0x0F

/*TODO: Min Volume and Max Volume value*/
// #define MAX_VOLUME 0x0000
// #define MIN_VOLUME 0xFAFA
/*----------------------------------------------------------------------------*/
/*ANCHOR: This MP3 decoder use SSP0 as SPI bus to connect from MCU to decoder */
/*----------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------- */
/*                        NOTE: Chip select SCI mode                          */
/* @brief: activate CS pin(set to low) and select slave to use                */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void CS();

/* -------------------------------------------------------------------------- */
/*                         NOTE: Deselect Chip SCI mode                       */
/* @brief: deactivate CS pin(set to high) and release slave                   */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void DS();

/* -------------------------------------------------------------------------- */
/*                         NOTE: Chip select SDI mode                         */
/* @brief: activate XDCS pin(set to low) and select slave to use              */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void CDS();

/* -------------------------------------------------------------------------- */
/*                        NOTE: Deselect Chip SDI mode                        */
/* @brief: deactivate XDCS pin(set to high) and release slave                 */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void DDS();

/* -------------------------------------------------------------------------- */
/*                        NOTE: Reset on                                      */
/* @brief: Turn on reset and reset the default setup for decoder              */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void RST_ON();

/* -------------------------------------------------------------------------- */
/*                        NOTE: Reset off                                     */
/* @brief: Turn off reset after turn on reset pin                             */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void RST_OFF();

/* -------------------------------------------------------------------------- */
/*                        NOTE: MP3 set up clock                              */
/* @brief: config clock will be used for SPI bus                              */
/* @param: clock value                                                        */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void mp3_setup_clock_info(uint32_t max_clock_mhz);


/* -------------------------------------------------------------------------- */
/*                        NOTE: Transfer Byte                                 */
/* @brief: Transfer each byte from MCU to decoder                             */
/* @param: byte need to be transfer                                           */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
uint8_t mp3_transfer_byte(uint8_t transfer_byte);

/* -------------------------------------------------------------------------- */
/*                        NOTE: Volume adjustment                             */
/* @brief: Adjust the volume                                                  */
/* @param: volume value                                                       */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void mp3_adjust_vol(uint16_t volume);

/* -------------------------------------------------------------------------- */
/*                        NOTE: Write using SCI                               */
/* @brief: write data to SCI register                                         */
/* @param: register value, data will be written                               */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void mp3_write_sci(uint8_t reg_address,uint16_t data);

/* -------------------------------------------------------------------------- */
/*                        NOTE: Read using SCI                                */
/* @brief: read data to SCI register                                          */
/* @param: register value                                                     */
/* @return: data will be read                                                 */
/* -------------------------------------------------------------------------- */
uint16_t mp3_read_sci(uint8_t reg_address);

/* -------------------------------------------------------------------------- */
/*                       FIXME : Set up pins                                  */
/* @brief: Set up pin that connect to the decoder                             */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void mp3_setup();

