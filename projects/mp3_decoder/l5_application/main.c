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
  fprintf(stderr, "%c", (char)178);
  char volume[16];
  char smallest_square = (char)128;
  char small_square = (char)129;
  char medium_square = (char)130;
  char full_square = (char)131;

  // int len = strlen(volume);
  // fprintf(stderr, "%d", len);

  for (int i = 0; i < 14; i++) {
    if (i <= 2) {
      volume[i] = smallest_square;
    } else if (i <= 5 && i > 2) {
      volume[i] = small_square;
    } else if (i <= 10 && i > 5) {
      volume[i] = medium_square;
    } else {
      volume[i] = full_square;
    }
  }

  /* --------------------------------- Part 1 --------------------------------- */
  oled_print(volume, 0, init);
  oled_print("check scrolling", 1, ninit);
  // oled_print("test", 2, ninit);
  // horizontal_scrolling(0, 0);

  while (1) {
  }

  // vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}