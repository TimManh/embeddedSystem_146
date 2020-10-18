#include <stdint.h>

/* ------------------------- Initialize SSP2 for SPI ------------------------ */

void ssp2__init_lab(uint32_t max_clock_mhz);

/* --------------------------- Setup pin for ssp2 --------------------------- */
void ssp2_setup(void);

/* ------------------------- Exchange byte from SPI ------------------------- */

uint8_t ssp2__exchange_byte_lab(uint8_t data_out);