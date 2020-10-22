#pragma once
#include "FreeRTOS.h"
#include "gpio.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "queue.h"
#include "stdio.h"
#include <stdbool.h>
#include <stdint.h>
typedef enum {
  UART_2,
  UART_3,
} uart_number_e;

/* -------------------------------------------------------------------------- */
/*                                   Task 1                                   */
/* -------------------------------------------------------------------------- */

/* ----------------------------- Initialize Uart ---------------------------- */
void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate);

/* ------------------------------ Receive Data ------------------------------ */
bool uart_lab__polled_get(uart_number_e uart, char *input_byte);

/* -------------------------------- Send Data ------------------------------- */
bool uart_lab__polled_put(uart_number_e uart, char output_byte);

/* -------------------------------------------------------------------------- */
/*                                   Task 2                                   */
/* -------------------------------------------------------------------------- */

// Private queue handle of our uart_lab.c
static QueueHandle_t your_uart_rx_queue;

// Private function of our uart_lab.c
static void your_receive_interrupt(void);

// Public function to enable UART interrupt
void uart__enable_receive_interrupt(uart_number_e uart_number, int queue_size);

// Public function to get a char from the queue (this function should work without modification)
bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout);