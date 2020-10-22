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
  char temp4 = '\n';

  while (1) {
    /* send a value */
    uart_lab__polled_put(UART_3, temp0);
    uart_lab__polled_put(UART_3, temp1);
    uart_lab__polled_put(UART_3, temp2);
    uart_lab__polled_put(UART_3, temp3);
    uart_lab__polled_put(UART_3, temp4);
    // NOTE: Race condition will happen and it need to be solved by queue
    vTaskDelay(100);
  }
}

/* ----------------------- Read task from ISR ----------------------- */

void uart_read_task_fromISR(void *p) {
  while (1) {
    char input = ' ';
    // printf("Before checking into ISR\n");
    if (uart_lab__get_char_from_queue(&input, 100)) {
      fprintf(stderr, "%c", input);
    }
    // if we want to see no writing data to the queue behavior
    else {
      fprintf(stderr, "No data for 100ms\n");
    }
  }
}

int lanchay = 0;
// This task is done for you, but you should understand what this code is doing
void board_1_sender_task(void *p) {
  char number_as_string[16] = {0};
  while (true) {
    lanchay++;
    const int number = rand() % 1000;
    printf("RANDOM NUMBER IS: %d\n", number);
    sprintf(number_as_string, "%i", number);
    // Send one char at a time to the other board including terminating NULL char
    for (int i = 0; i <= strlen(number_as_string); i++) {
      uart_lab__polled_put(UART_3, number_as_string[i]);
      // printf("Sent: %c\n", number_as_string[i]);
    }

    // printf("Sent: %i over UART to the other board\n", number);
    vTaskDelay(3000);
  }
}

void board_2_receiver_task(void *p) {
  char number_as_string[16] = {0};
  int counter = 0;

  while (true) {
    char byte = 0;
    uart_lab__get_char_from_queue(&byte, portMAX_DELAY);
    printf("Received: %c\n", byte);
    // sprintf(number_as_string, "", byte);
    // printf("STRING: %s\n", number_as_string);

    // This is the last char, so print the number
    if ('\0' == byte) {
      number_as_string[counter] = '\0';
      counter = 0;
      printf("Received this number from UART 3: %s\n\n\n", number_as_string);
    }
    // // We have not yet received the NULL '\0' char, so buffer the data
    else {
      // TODO: Store data to number_as_string[] array one char at a time
      // Hint: Use counter as an index, and increment it as long as we do not reach max value of 16
      number_as_string[counter] = byte;
      printf("loading: %s\n", number_as_string);
      counter++;
    }
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
  // uart_lab__init(UART_3, 96000000, 38400);
  // uart__enable_receive_interrupt(UART_3, 16);
  // xTaskCreate(uart_read_task_fromISR, "read", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  // xTaskCreate(uart_write_task, "write", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);

  /* -------------------------------------------------------------------------- */
  /*                                   Task 3                                   */
  /* -------------------------------------------------------------------------- */
  uart_lab__init(UART_3, 96000000, 38400);
  uart_lab__init(UART_2, 96000000, 38400);
  uart__enable_receive_interrupt(UART_2, 16);

  xTaskCreate(board_2_receiver_task, "read", 2048 / sizeof(void *), NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(board_1_sender_task, "write", 2048 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);

  vTaskStartScheduler();
}