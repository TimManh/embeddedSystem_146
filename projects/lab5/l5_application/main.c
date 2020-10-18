#include "FreeRTOS.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "ssp2_lab.h"
#include "task.h"
#include <stdio.h>
void adesto_cs(void);
void adesto_ds(void);

typedef struct {
  uint8_t manufacturer_id;
  uint8_t device_id_1;
  uint8_t device_id_2;
  uint8_t extended_device_id;
} adesto_flash_id_s;

/* -------------------------- Get info from adesto -------------------------- */

adesto_flash_id_s adesto_read_signature(void) {
  adesto_flash_id_s data = {0};

  adesto_cs();

  ssp2__exchange_byte_lab(0x9F);
  data.manufacturer_id = ssp2__exchange_byte_lab(0xFF);
  data.device_id_1 = ssp2__exchange_byte_lab(0xFF);
  data.device_id_2 = ssp2__exchange_byte_lab(0xFF);
  data.extended_device_id = ssp2__exchange_byte_lab(0xFF);
  adesto_ds();

  return data;
}

/* ----------------------------- Create spi task ---------------------------- */

void spi_task(void *p) {
  const uint32_t spi_clock_mhz = 24000000;

  // Initialize ssp2
  ssp2__init_lab(spi_clock_mhz);

  // set up pin for ssp2
  ssp2_setup();

  while (1) {
    adesto_flash_id_s id = adesto_read_signature();

    fprintf(stderr, "manufacture ID %x \n", id.manufacturer_id);
    fprintf(stderr, "device id 1 %x \n", id.device_id_1);
    fprintf(stderr, "device id 2  %.2x \n", id.device_id_2);
    fprintf(stderr, "xtend device id %x \n\n", id.extended_device_id);

    vTaskDelay(500);
  }
}

/* -------------------------------------------------------------------------- */
/*                                    MAIN                                    */
/* -------------------------------------------------------------------------- */

void main(void) {

  /* --------------------------------- Part 1 --------------------------------- */

  xTaskCreate(spi_task, "Read ADESTO", 4096, NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
}

/* ---------------------------- Selecting adesto ---------------------------- */

void adesto_cs(void) {
  gpio__construct_as_output(1, 10);
  gpio_set_low(1, 10);
}

/* --------------------------- Deselecting adesto --------------------------- */

void adesto_ds(void) {
  gpio__construct_as_output(1, 10);
  gpio_set_high(1, 10);
}