#include "FreeRTOS.h"
#include "gpio.h"
#include "stdio.h"
#include "task.h"
#include "uart_lab.h"

/* -------------------------------- Read Task ------------------------------- */

void uart_read_task(void *p) {

  while (1) {
    char temp = ' ';
    uart_lab__polled_get(UART_3, &temp);
    fprintf(stderr, "%c", temp);

    vTaskDelay(100);
  }
}

/* ------------------------------- Write Task ------------------------------- */

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

    // NOTE: Race condition will happen and it need to be solved by queue
    vTaskDelay(100);
  }
}

/* ----------------------- Task to read uart from ISR ----------------------- */

void uart_read_task_fromISR(void *p) {
  while (1) {
    char *input;
    // printf("Before checking into ISR\n");
    if (uart_lab__get_char_from_queue(&input, 100)) {
      // uart_lab__polled_get(UART__3, &input);

      fprintf(stderr, "%c", input);
    }
    // if we want to see no writing data to the queue behavior
    else {
      fprintf(stderr, "No data for 100ms\n");
    }
    // vTaskDelay(500);
  }
}
void main(void) {

  /* -------------------------------------------------------------------------- */
  /*                                   Task 1                                   */
  /* -------------------------------------------------------------------------- */

  // uart_lab__init(UART_3, 96000000, 38400);
  // xTaskCreate(uart_write_task, "Write", 1024, NULL, 2, NULL);
  // xTaskCreate(uart_read_task, "Read", 1024, NULL, 1, NULL);

  /* -------------------------------------------------------------------------- */
  /*                                   Task 2                                   */
  /* -------------------------------------------------------------------------- */
  uart_lab__init(UART_3, 96000000, 38400);
  uart__enable_receive_interrupt(UART_3, 16);
  xTaskCreate(uart_read_task_fromISR, "read", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(uart_write_task, "write", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  vTaskStartScheduler();
}