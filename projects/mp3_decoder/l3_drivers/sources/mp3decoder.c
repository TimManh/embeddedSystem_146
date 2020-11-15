#include "mp3decoder.h"

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
static void mp3_max_clock(uint32_t SPI_clock){
    SPI_clock = SPI_clock *1000 *1000;
    /* c) Setup prescalar register to be <= SPI_clock-(Input) */
  const uint32_t CPU_CLK = clock__get_core_clock_hz(); // 96-MHz
  for (uint8_t divider = 2; divider <= 254; divider += 2) {
    if ((CPU_CLK / divider) <= SPI_clock) {
      // fprintf(stderr, "Pre_Scale: %d \n", divider);
      break;
    }
    /* Setup PreScale Control[7:0] */
    LPC_SSP0->CPSR = divider;
  }
}

/* -------------------------------------------------------------------------- */
/*                       NOTE: Config pin                                     */
/* @brief: config function that pin will be used by decoder                   */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
static void mp3_pin_config(){

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

static void mp3_vol_helper(uint16_t vol){
    mp3_write_sci(SCI_VOL,vol);
}

/* ---------------NOTE: Helper function handle 16 bit data for SCI --------------- */
static void mp3_transfer_2bytes(uint16_t data){
    mp3_transfer_byte((data>>8) & 0xFF);
    mp3_transfer_byte((data >> 0) & 0xFF);
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/*                         ANCHOR: PUBLIC FUNCTION                            */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                        NOTE: Chip select SCI mode                          */
/* @brief: activate CS pin(set to low) and select slave to use                */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void CS(){
    LPC_GPIO2->CLR |= (1 << 1);
}

/* -------------------------------------------------------------------------- */
/*                         NOTE: Deselect Chip SCI mode                       */
/* @brief: deactivate CS pin(set to high) and release slave                   */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void DS(){
 LPC_GPIO2->SET |= (1 << 1);
}

/* -------------------------------------------------------------------------- */
/*                         NOTE: Chip select SDI mode                         */
/* @brief: activate XDCS pin(set to low) and select slave to use              */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void CDS(){
    LPC_GPIO2->CLR |= (1 << 2);
}

/* -------------------------------------------------------------------------- */
/*                        NOTE: Deselect Chip SDI mode                        */
/* @brief: deactivate XDCS pin(set to high) and release slave                 */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void DDS(){
    LPC_GPIO2->SET |= (1 << 2);
}

/* -------------------------------------------------------------------------- */
/*                        NOTE: Reset on                                      */
/* @brief: Turn on reset and reset the default setup for decoder              */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void RST_ON(){
    LPC_GPIO2->CLR |= (1 << 4);
}

/* -------------------------------------------------------------------------- */
/*                        NOTE: Reset off                                     */
/* @brief: Turn off reset after turn on reset pin                             */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void RST_OFF(){
    LPC_GPIO2->SET |= (1 << 4); 
}

/* -------------------------------------------------------------------------- */
/*                        NOTE: MP3 set up                                    */
/* @brief: config clock will be used for SPI bus                              */
/* @param: clock value                                                        */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void mp3_setup_clock_info(uint32_t max_clock_mhz){
      max_clock_mhz = max_clock_mhz * 1000 * 1000;
  /* a) Power on Peripheral */
  lpc_peripheral__turn_on_power_to(LPC_PERIPHERAL__SSP2);

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
uint8_t mp3_transfer_byte(uint8_t transfer_byte){
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
void mp3_adjust_vol(uint16_t volume){
    mp3_vol_helper(VOLUME[volume]);
}

/* -------------------------------------------------------------------------- */
/*                        NOTE: Write using SCI                               */
/* @brief: write data to SCI register                                         */
/* @param: register value, data will be written                               */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void mp3_write_sci(uint8_t reg_address,uint16_t data){
    /* Wait until DREQ turn high*/
    while(!gpio1__get_level(2,0)){
        ;
    }
    CS(); //Turn on chip select for sci
    mp3_transfer_byte(0x02); //send OP code to write
    mp3_transfer_byte(reg_address); //Send register address to write on
    /* REVIEW: This sci receive 16bits of data for each transaction from datasheet in page 20/89 */
    mp3_transfer_2bytes(data);

    /* Wait until transaction finish*/
    while(!gpio1__get_level(2,0)){
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
uint16_t mp3_read_sci(uint8_t reg_address){
    /* Wait until DREQ turn high*/
    while(!gpio1__get_level(2,0)){
        ;
    }
    CS(); //Turn on chip select for sci
    mp3_transfer_byte(0x03); //send OP code to read
    mp3_transfer_byte(reg_address); //Send register address to write on
    /* REVIEW: This sci receive 16bits of data for each transaction from datasheet in page 20/89 */
    uint8_t MSB_data = mp3_transfer_byte(0xFF); // send dummy byte to get value in return

    /* Wait until DREQ turn high*/
    while(!gpio1__get_level(2,0)){
        ;
    }

    uint8_t LSB_data = mp3_transfer_byte(0xFF); // send dummy byte to get value in return

    /* Wait until DREQ turn high*/
    while(!gpio1__get_level(2,0)){
        ;
    }
    DS(); // Turn off chip select for sci
    uint16_t total_data = 0;
    total_data |= ((MSB_data<<8) | (LSB_data<<0));
    return total_data;
}

/* -------------------------------------------------------------------------- */
/*                       FIXME : Set up pins                                  */
/* @brief: Set up pin that connect to the decoder                             */
/* @param: NA                                                                 */
/* @return: void                                                              */
/* -------------------------------------------------------------------------- */
void mp3_setup(){
    
    mp3_pin_config(); //config all the pins to use for decoder

    RST_OFF(); // turn off the reset pin

    mp3_setup_clock_info(1); //Insert 1MHz clock for SPI0 bus

    mp3_transfer_byte(0xFF); //Send dummy byte

    /* Set high bit for XDCS and CS */
    DS();
    DDS();

    /* Set default volume */
    mp3_adjust_vol(0x3535);

    /* -------------------------------------------------------------------------- */
    /*                           Check Status of VS1053                           */
    /* -------------------------------------------------------------------------- */

    uint16_t mp3_stat = mp3_read_sci(SCI_STATUS);
    uint16_t mp3_ver = (mp3_stat >>4) & 0x000F;
    fprintf(stderr, "MP3 decoder version: %d \n", mp3_ver);

    /* -------------------------------------------------------------------------- */
    /*                           Check Clock of VS1053                            */
    /* -------------------------------------------------------------------------- */
    mp3_max_clock(4);
    uint16_t mp3_val_clock = mp3_read_sci(SCI_CLOCKF);
    fprintf(stderr, "clock speed: %x\n", mp3_val_clock);


}
