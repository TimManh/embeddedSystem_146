#include "FreeRTOS.h"
#include "gpio.h"
#include "stdio.h"
#include "task.h"
#include "uart_lab.h"
void uart_read_task(void *p) {

  while (1) {
    char *temp;
    uart_lab__polled_get(UART_3, &temp);
    fprintf(stderr, "%c", temp);

    vTaskDelay(100);
  }
}

void uart_write_task(void *p) {
  char temp0 = 'H';
  char temp1 = 'E';
  char temp2 = 'L';
  char temp3 = 'P';

  while (1) {
    /* send a value */
    uart_lab__polled_put(UART_3, temp0);
    uart_lab__polled_put(UART_3, temp1);
    uart_lab__polled_put(UART_3, temp2);
    uart_lab__polled_put(UART_3, temp3);

    vTaskDelay(100);
  }
}

void main(void) {

  /* -------------------------------------------------------------------------- */
  /*                                   Task 1                                   */
  /* -------------------------------------------------------------------------- */

  uart_lab__init(UART_3, 96000000, 38400);
  xTaskCreate(uart_write_task, "Write", 1024, NULL, 1, NULL);
  xTaskCreate(uart_read_task, "Read", 1024, NULL, 1, NULL);

  /* -------------------------------------------------------------------------- */
  /*                                   Task 2                                   */
  /* -------------------------------------------------------------------------- */

  vTaskStartScheduler();
}