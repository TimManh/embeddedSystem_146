#include "FreeRTOS.h"
#include "adc.h"
#include "board_io.h"
#include "common_macros.h"
#include "delay.h"
#include "gpio.h"

#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"

#include "oled.h"
#include "queue.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "ssp2_lab.h"
#include "task.h"
#include <stdio.h>

#include "string.h"
#include "uart_lab.h"
char *test(char *newstring);
/* ------------------------------- OLED ------------------------------- */
void oled_task() {
  while (1) {
    turn_on_lcd();
  }
}

/***************************************** MAIN LOOP *********************************************
**************************************************************************************************/
int main(void) {

  /* ----------------------------- Initialization ----------------------------- */
  puts("Starting RTOS\n");
  // sj2_cli__init();

  /* --------------------------------- Part 1 --------------------------------- */
  char temp[] = "Nang Am Xa Dan";

  // fprintf(stderr, "%s", temp);
  oled_print(test(temp));
  horizontal_scrolling();
  delay__ms(5000);
  horizontal_scrolling();

  while (1) {
  }

  // vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}