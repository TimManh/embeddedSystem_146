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
  } else {
    LPC_UART2->LCR &= ~(1 << 7);
    while (!(LPC_UART2->LSR & (1 << 0))) {
      ;
    }
    // Copy data from RBR register to input_byte
    *input_byte = LPC_UART2->RBR;
    // fprintf(stderr, "%c\n", *input_byte);
  }

  return (*input_byte != 0) ? true : false;
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

  } else {
    // Check LSR for Transmit Hold Register Empty
    while (!(LPC_UART2->LSR & (1 << 5))) {
      ;
    }
    // Copy output_byte to THR register
    LPC_UART2->THR = output_byte;
  }
  return (output_byte != 0) ? true : false;
}

/* -------------------------------------------------------------------------- */
/*                              PRIVATE FUNCTION                              */
/* -------------------------------------------------------------------------- */

// Private queue handle of our uart_lab.c
static QueueHandle_t your_uart_rx_queue;

// Private function of our uart_lab.c
static void your_receive_interrupt(void) {
  // NOTE: check IIR status and LSR register if there is interrupt and data to be read
  if ((((LPC_UART3->IIR >> 1) & 0xF) == 0x2) && (LPC_UART3->LSR & (1 << 0))) {
    // Store data to variable byte from register RBR
    const char byte = LPC_UART3->RBR;
    // send data to the queue
    xQueueSendFromISR(your_uart_rx_queue, &byte, NULL);
  }
}

/* -------------------------------------------------------------------------- */
/*                               PUBLIC FUNCTION                              */
/* -------------------------------------------------------------------------- */

// Public function to enable UART interrupt
void uart__enable_receive_interrupt(uart_number_e uart_number, int queue_size) {

  if (uart_number) {
    // NOTE: Use lpc_peripherals.h to attach your interrupt
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART3, your_receive_interrupt, NULL);
    NVIC_EnableIRQ(UART3_IRQn);

    LPC_UART3->LCR &= ~(1 << 7);
    // Enable RDA interrrupt
    LPC_UART3->IER |= (1 << 0);

  } else {
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART2, your_receive_interrupt, NULL);
    NVIC_EnableIRQ(UART2_IRQn);

    LPC_UART2->LCR &= ~(1 << 7);
    // Enable RDA interrrupt
    LPC_UART2->IER |= (1 << 0);
  }
  // Create your RX queue
  your_uart_rx_queue = xQueueCreate(queue_size, sizeof(char));
}

// Public function to get a char from the queue (this function should work without modification)
bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  return xQueueReceive(your_uart_rx_queue, input_byte, timeout);
}