#include "ssp2_lab.h"
#include "clock.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
/* -------------------------------------------------------------------------- */
/*                                   Part 0                                   */
/* -------------------------------------------------------------------------- */

void ssp2__init_lab(uint32_t max_clock_mhz) {
  // Power on Peripheral
  lpc_peripheral__is_powered_on(LPC_PERIPHERAL__SSP2);
  // Setup control registers CR0 and CR1
  // Set control register 0 as 8 bit data size
  LPC_SSP2->CR0 = 7;

  // Enable SSP as Master
  LPC_SSP2->CR1 |= (1 << 1);

  // Setup prescalar register to be <= max_clock_mhz
  // max clock speed is 24Mhz
  uint8_t divider = 2;
  const uint32_t cpu_clock_mhz = clock__get_peripheral_clock_hz();

  // Keep scaling down divider until calculated is higher
  while (max_clock_mhz < (cpu_clock_mhz / divider) && divider <= 254) {
    divider += 2;
  }

  LPC_SSP2->CPSR = divider;
}
void ssp2_setup(void) {
  gpio__construct_with_function(1, 0, GPIO__FUNCTION_4);
  gpio__construct_with_function(1, 1, GPIO__FUNCTION_4);
  gpio__construct_with_function(1, 4, GPIO__FUNCTION_4);
  gpio_set_as_output(1, 0);
  gpio_set_as_output(1, 1);
  gpio_set_as_input(1, 4);
}

uint8_t ssp2__exchange_byte_lab(uint8_t data_out) {
  // Configure the Data register(DR) to send and receive data by checking the SPI peripheral status register
  LPC_SSP2->DR = data_out;

  while (LPC_SSP2->SR & (1 << 4)) {
    ;
  }

  return (uint8_t)(LPC_SSP2->DR & 0xFF);
}