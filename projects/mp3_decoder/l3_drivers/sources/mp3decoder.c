#include "mp3decoder.h"
const uint16_t VOLUME[10] = {0X7D7D, 0X6464, 0x4B4B, 0x3C3C, 0x3535, 0x3030, 0x2525, 0x2020, 0x1515};

/* ---------------------------------------------------------------------------*/
/*ANCHOR: This MP3 decoder use SSP0 as SPI bus to connect from MCU to decoder */
/* ---------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/*                        ANCHOR: PRIVATE FUNCTION                            */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                     NOTE: Set full speed clock for decoder                 */
/* @brief: config function that pin will be used by decoder                   */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
static void mp3_max_clock(uint32_t SPI_clock) {
  SPI_clock = SPI_clock * 1000 * 1000;
  uint8_t divider = 2;
  /* c) Setup prescalar register to be <= SPI_clock-(Input) */
  const uint32_t CPU_CLK = clock__get_core_clock_hz(); // 96-MHz
  while (SPI_clock < (CPU_CLK / divider) && divider <= 254) {
    divider += 2;
  }
  /* Setup PreScale Control[7:0] */
  LPC_SSP0->CPSR = divider;
}

/* -------------------------------------------------------------------------- */
/*                       NOTE: Config pin                                     */
/* @brief: config function that pin will be used by decoder                   */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
static void mp3_pin_config() {

  /* -------------------------------- SPI0 PIN -------------------------------- */

  gpio__construct_with_function(0, 15, GPIO__FUNCTION_2); // CLK
  gpio__construct_with_function(0, 17, GPIO__FUNCTION_2); // MISO
  gpio__construct_with_function(0, 18, GPIO__FUNCTION_2); // MOSI

  /* ------------------------------ GPIO Control ------------------------------ */
  gpio__construct_with_function(2, 0, GPIO__FUNCITON_0_IO_PIN);
  gpio1__set_as_input(2, 0); // setup DREQ

  gpio__construct_with_function(2, 1, GPIO__FUNCITON_0_IO_PIN);
  gpio1__set_as_output(2, 1); // setup CS (low active)

  gpio__construct_with_function(2, 2, GPIO__FUNCITON_0_IO_PIN);
  gpio1__set_as_output(2, 2); // setup DCS or XDCS (low active)

  gpio__construct_with_function(2, 4, GPIO__FUNCITON_0_IO_PIN);
  gpio1__set_as_output(2, 4); // setup Reset (low active)
}

/* ------------------NOTE: Helper function for set up volume ----------------*/

static void mp3_vol_helper(uint16_t vol) { mp3_write_sci(SCI_VOL, vol); }

/* ---------------NOTE: Helper function handle 16 bit data for SCI --------------- */
static void mp3_transfer_2bytes(uint16_t data) {
  mp3_transfer_byte((data >> 8) & 0xFF);
  mp3_transfer_byte((data >> 0) & 0xFF);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/*                         ANCHOR: PUBLIC FUNCTION                            */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* (P2_0) Input */
bool get_DREQ_HighActive() { return LPC_GPIO2->PIN & (1 << 0) ? true : false; }

/* -------------------------------------------------------------------------- */
/*                        NOTE: Chip select SCI mode                          */
/* @brief: activate CS pin(set to low) and select slave to use                */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void CS() { LPC_GPIO2->CLR |= (1 << 1); }

/* -------------------------------------------------------------------------- */
/*                         NOTE: Deselect Chip SCI mode                       */
/* @brief: deactivate CS pin(set to high) and release slave                   */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void DS() { LPC_GPIO2->SET |= (1 << 1); }

/* -------------------------------------------------------------------------- */
/*                         NOTE: Chip select SDI mode                         */
/* @brief: activate XDCS pin(set to low) and select slave to use              */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void CDS() { LPC_GPIO2->CLR |= (1 << 2); }

/* -------------------------------------------------------------------------- */
/*                        NOTE: Deselect Chip SDI mode                        */
/* @brief: deactivate XDCS pin(set to high) and release slave                 */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void DDS() { LPC_GPIO2->SET |= (1 << 2); }

/* -------------------------------------------------------------------------- */
/*                        NOTE: Reset on                                      */
/* @brief: Turn on reset and reset the default setup for decoder              */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void RST_ON() { LPC_GPIO2->CLR |= (1 << 4); }

/* -------------------------------------------------------------------------- */
/*                        NOTE: Reset off                                     */
/* @brief: Turn off reset after turn on reset pin                             */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void RST_OFF() { LPC_GPIO2->SET |= (1 << 4); }

/* -------------------------------------------------------------------------- */
/*                        NOTE: MP3 set up                                    */
/* @brief: config clock will be used for SPI bus                              */
/* @param: clock value                                                        */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void mp3_setup_clock_info(uint32_t max_clock_mhz) {
  max_clock_mhz = max_clock_mhz * 1000 * 1000;
  /* a) Power on Peripheral */
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP0);

  /* b) Setup control registers CR0 and CR1 */
  LPC_SSP0->CR0 = 7;        // 8-Bit transfer
  LPC_SSP0->CR1 = (1 << 1); // SSP Control Enable

  /* c) Setup prescalar register to be <= max_clock_mhz-(Input) */
  const uint32_t CPU_CLK = clock__get_core_clock_hz(); // 96-MHz
  for (uint8_t divider = 2; divider <= 254; divider += 2) {
    if ((CPU_CLK / divider) <= max_clock_mhz) {
      break;
    }
    /* Setup PreScale Control[7:0] */
    LPC_SSP0->CPSR = divider;
  }
}

/* -------------------------------------------------------------------------- */
/*                        NOTE: Transfer Byte                                 */
/* @brief: Transfer each byte from MCU to decoder                             */
/* @param: byte need to be transfer                                           */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
uint8_t mp3_transfer_byte(uint8_t transfer_byte) {
  /* 16-Bits Data Register [15:0] */
  LPC_SSP0->DR = transfer_byte;

  /* Status Register-BUSY[4] */
  while (LPC_SSP0->SR & (1 << 4)) {
    ; /* Wait while it is busy(1), else(0) BREAK */
  }
  /* READ 16-Bits Data Register [15:0] */
  return (uint8_t)(LPC_SSP0->DR & 0xFF);
}

/* -------------------------------------------------------------------------- */
/*                        NOTE: Volume Adjustment                             */
/* @brief: Adjust the volume                                                  */
/* @param: volume value                                                       */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void mp3_adjust_vol(uint16_t volume) { mp3_vol_helper(VOLUME[volume]); }

/* -------------------------------------------------------------------------- */
/*                        NOTE: Write using SCI                               */
/* @brief: write data to SCI register                                         */
/* @param: register value, data will be written                               */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void mp3_write_sci(uint8_t reg_address, uint16_t data) {
  /* Wait until DREQ turn high*/
  while (!gpio1__get_level(2, 0)) {
    ;
  }
  CS();                           // Turn on chip select for sci
  mp3_transfer_byte(0x02);        // send OP code to write
  mp3_transfer_byte(reg_address); // Send register address to write on
  /* REVIEW: This sci receive 16bits of data for each transaction from datasheet in page 20/89 */
  mp3_transfer_2bytes(data);

  /* Wait until transaction finish*/
  while (!gpio1__get_level(2, 0)) {
    ;
  }
  DS(); // Turn off chip select for sci
}

/* -------------------------------------------------------------------------- */
/*                        NOTE: Read using SCI                                */
/* @brief: read data to SCI register                                          */
/* @param: register value                                                     */
/* @return: data will be read                                                 */
/* -------------------------------------------------------------------------- */
uint16_t mp3_read_sci(uint8_t reg_address) {
  /* Wait until DREQ turn high*/
  while (!gpio1__get_level(2, 0)) {
    ;
  }
  CS();                           // Turn on chip select for sci
  mp3_transfer_byte(0x03);        // send OP code to read
  mp3_transfer_byte(reg_address); // Send register address to write on
  /* REVIEW: This sci receive 16bits of data for each transaction from datasheet in page 20/89 */
  uint8_t MSB_data = mp3_transfer_byte(0xFF); // send dummy byte to get value in return

  /* Wait until DREQ turn high*/
  while (!gpio1__get_level(2, 0)) {
    ;
  }

  uint8_t LSB_data = mp3_transfer_byte(0xFF); // send dummy byte to get value in return

  /* Wait until DREQ turn high*/
  while (!gpio1__get_level(2, 0)) {
    ;
  }
  DS(); // Turn off chip select for sci
  uint16_t total_data = 0;
  total_data |= ((MSB_data << 8) | (LSB_data << 0));
  return total_data;
}

/* -------------------------------------------------------------------------- */
/*                        NOTE: Write data using SDI                          */
/* @brief: read data to SDI register                                          */
/* @param: data will be send every byte                                       */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void mp3_write_sdi(uint8_t data) {
  /* Wait until DREQ turn high*/
  while (!gpio1__get_level(2, 0)) {
    ;
  }
  CDS();
  mp3_transfer_byte(data);
  DDS();
}

/* -------------------------------------------------------------------------- */
/*                       NOTE : Set up pins                                  */
/* @brief: Set up pin that connect to the decoder                             */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void mp3_setup() {

  mp3_pin_config(); // config all the pins to use for decoder

  RST_OFF(); // turn off the reset pin

  mp3_setup_clock_info(1); // Insert 1MHz clock for SPI0 bus

  mp3_transfer_byte(0xFF); // Send dummy byte

  /* Set high bit for XDCS and CS */
  DS();
  DDS();

  /* Set default volume */
  mp3_adjust_vol(0);

  /* -------------------------------------------------------------------------- */
  /*                           Check Status of VS1053                           */
  /* -------------------------------------------------------------------------- */

  uint16_t mp3_stat = mp3_read_sci(SCI_STATUS);
  uint16_t mp3_ver = (mp3_stat >> 4) & 0x000F;
  fprintf(stderr, "MP3 decoder version: %d \n", mp3_ver);

  uint16_t mp3_clk = mp3_read_sci(SCI_CLOCKF);
  delay__ms(100);
  mp3_write_sci(SCI_CLOCKF, 0x6000);
  uint16_t mp3_clk_2 = mp3_read_sci(SCI_CLOCKF);
  uint16_t mp3mode = mp3_read_sci(SCI_MODE);

  /* -------------------------------------------------------------------------- */
  /*                           Check Clock of VS1053                            */
  /* -------------------------------------------------------------------------- */
  mp3_max_clock(4);
  uint16_t mp3_val_clock = mp3_read_sci(SCI_CLOCKF);
  printf("clock speed: %x \n", mp3_val_clock);
}

char *remove_dot_mp3(char *current_song) {
  trackname_t song_name_copy;
  char *copy_song;
  copy_song = song_name_copy;
  //   display(song_name_copy);
  strcpy(song_name_copy, current_song);
  //   display(song_name_copy);
  copy_song = strtok(song_name_copy, ".");
  return copy_song;
}

/* -------------------------------------------------------------------------- */
/*                                 GENRE TABLE                                */
/* -------------------------------------------------------------------------- */

char *genre_decoder(uint8_t genre) {
  char *ger;
  switch (genre) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
    ger = "Genre: Pop";
    break;
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  case 30:
  case 31:
  case 32:
  case 33:
  case 34:
  case 35:
  case 36:
  case 37:
  case 38:
  case 39:
  case 40:
  case 41:
  case 42:
  case 43:
  case 44:
  case 45:
  case 46:
  case 47:
  case 48:
  case 49:
  case 50:
  case 51:
  case 52:
  case 53:
  case 54:
  case 55:
  case 56:
  case 57:
  case 58:
  case 59:
  case 60:
  case 61:
  case 62:
  case 63:
  case 64:
  case 65:
  case 66:
  case 67:
  case 68:
  case 69:
  case 70:
  case 71:
  case 72:
  case 74:
  case 75:
  case 76:
  case 77:
  case 78:
  case 79:
  case 80:
  case 81:
  case 82:
  case 83:
  case 84:
  case 85:
  case 86:
  case 87:
  case 88:
  case 89:
  case 90:
  case 91:
  case 92:
  case 93:
  case 94:
  case 95:
  case 96:
  case 97:
  case 98:
  case 99:
  case 100:
  case 101:
  case 102:
  case 103:
  case 104:
  case 105:
  case 106:
  case 107:
  case 108:
  case 109:
  case 110:
  case 111:
  case 112:
  case 113:
  case 114:
  case 115:
  case 116:
    ger = "Ballad";
    break;
  default:
    ger = "No Genre";
    break;
  }
  return ger;
}