#include "uart_lab.h"

/* -------------------------------------------------------------------------- */
/*                               Initialize uart                              */
/* -------------------------------------------------------------------------- */

void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {

  if (uart) {
    // Set up power for uart 3
    LPC_SC->PCONP |= (1 << 25);

    // uart 3 set tx
    gpio__construct_with_function(4, 28, GPIO__FUNCTION_2);

    // uart 3 set rx
    gpio__construct_with_function(4, 29, GPIO__FUNCTION_2);

    // Enable DLAB bit in uart_LCR register
    LPC_UART3->LCR |= (3 << 0) | (1 << 7);

    // Enable FIFO
    LPC_UART3->FCR |= (1 << 0);
    uint16_t divider = peripheral_clock / (16 * baud_rate);

    // MSB of 15-8 bit of divisor
    LPC_UART3->DLM = (divider >> 8) & 0xFF;
    // LSB of 7-0 bit of divisor
    LPC_UART3->DLL = (divider >> 0) & 0xFF;
  } else {
    // Set up power for uart 2
    LPC_SC->PCONP |= (1 << 24);
    gpio__construct_with_function(2, 8, GPIO__FUNCTION_2);

    // uart 2 set rx
    gpio__construct_with_function(2, 9, GPIO__FUNCTION_2);
    // Enable DLAB bit in uart_LCR register
    LPC_UART2->LCR |= (3 << 0) | (1 << 7);

    // Enable FIFO
    LPC_UART2->FCR |= (1 << 0);
    uint16_t divider = peripheral_clock / (16 * baud_rate);
    // MSB of 15-8 bit of divisor
    LPC_UART2->DLM = (divider >> 8) & 0xFF;

    // LSB of 7-0 bit of divisor
    LPC_UART2->DLL = (divider >> 0) & 0xFF;

    // LPC_UART2->LCR &= ~(1 << 7);
  }
}

/* -------------------------------------------------------------------------- */
/*                                Receive Data                                */
/* -------------------------------------------------------------------------- */
bool uart_lab__polled_get(uart_number_e uart, char *input_byte) {
  // fprintf(stderr, "get func\n");

  if (uart) {
    LPC_UART3->LCR &= ~(1 << 7);
    // Check LSR for Receive Data Ready
    while (!(LPC_UART3->LSR & (1 << 0))) {
      // fprintf(stderr, "inside loop");
      ;
    }
    // Copy data from RBR register to input_byte
    *input_byte = LPC_UART3->RBR;
    // fprintf(stderr, "%c\n", *input_byte);
    // fprintf(stderr, "Not Crash\n");

    return (*input_byte != 0) ? true : false;
  } else {
    LPC_UART2->LCR &= ~(1 << 7);
    while (!(LPC_UART2->LSR & (1 << 0))) {
      ;
    }
    *input_byte = LPC_UART2->RBR;
    // fprintf(stderr, "%c\n", *input_byte);
    return (*input_byte != 0) ? true : false;
  }

  // Copy data from RBR register to input_byte
}

/* -------------------------------------------------------------------------- */
/*                                  Send Data                                 */
/* -------------------------------------------------------------------------- */

bool uart_lab__polled_put(uart_number_e uart, char output_byte) {

  // fprintf(stderr, "put function\n");
  if (uart) {
    // Check LSR for Transmit Hold Register Empty
    while (!(LPC_UART3->LSR & (1 << 5))) {
      ;
    }

    LPC_UART3->THR = output_byte;
    // fprintf(stderr, "finished put function\n");
    return (output_byte != 0) ? true : false;
  } else {
    // Check LSR for Transmit Hold Register Empty
    LPC_UART3->LCR &= ~(1 << 7);
    while (!(LPC_UART2->LSR & (1 << 5))) {
      ;
    }

    LPC_UART2->THR = output_byte;
    return (output_byte != 0) ? true : false;
  }
  // Copy output_byte to THR register
}