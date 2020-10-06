#include <stdio.h>

#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "gpio_lab.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "sj2_cli.h"
#include "task.h"
#include <delay.h>
void gpio_interrupt(void);
void main(void) {

  switch_pressed_signal = ... ;    // Create your binary semaphore
  
  configure_your_gpio_interrupt(); // TODO: Setup interrupt by re-using code from Part 0
  NVIC_EnableIRQ(GPIO_IRQn);       // Enable interrupt gate for the GPIO
 
  xTaskCreate(sleep_on_sem_task, "sem", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();





  // Read Table 95 in the LPC user manual and setup an interrupt on a switch connected to Port0 or Port2
  // a) For example, choose SW2 (P0_30) pin on SJ2 board and configure as input
  /**
   * NOTE: Main code for first assignment to generate interrupt from button
   *   // ANCHOR: set p0.30 as input
  gpio_set_as_input(0, 30);
  //.   Warning: P0.30, and P0.31 require pull-down resistors

  // ANCHOR: pull down resistor for p0.30
  LPC_IOCON->P0_30 &= ~(1 << 4);
  LPC_IOCON->P0_30 |= (1 << 3);
  // b) Configure the registers to trigger Port0 interrupt (such as falling edge)

  // ANCHOR: set interrupt Trigger as falling edge for the switch P0.30
  // Install GPIO interrupt function at the CPU interrupt (exception) vector
  LPC_GPIOINT->IO0IntEnF |= (1 << 30);

  // c) Hijack the interrupt vector at interrupt_vector_table.c and have it call our gpio_interrupt()
  //    Hint: You can declare 'void gpio_interrupt(void)' at interrupt_vector_table.c such that it can see this function
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio_interrupt, NULL);
  // Most important step: Enable the GPIO interrupt exception using the ARM Cortex M API (this is from lpc40xx.h)
  NVIC_EnableIRQ(GPIO_IRQn);

  // Toggle an LED in a loop to ensure/test that the interrupt is entering ane exiting
  // For example, if the GPIO interrupt gets stuck, this LED will stop blinking
  gpio_set_as_output(2, 3);
  while (1) {
    // TODO: Toggle an LED here

    delay__ms(100);
    gpio_set_high(2, 3);
    delay__ms(100);
    gpio_set_low(2, 3);
  }
   */

}


// WARNING: You can only use printf(stderr, "foo") inside of an ISR
void gpio_interrupt(void) {
  fprintf(stderr, "ISR Entry");
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
  clear_gpio_interrupt();
}

void sleep_on_sem_task(void * p) {
  while(1) {
    // Use xSemaphoreTake with forever delay and blink an LED when you get the signal
  }
}



/**NOTE: This is function generate for interrupt for first assignment
 * 
void gpio_interrupt(void) {
  // a) Clear Port0/2 interrupt using CLR0 or CLR2 registers
  // b) Use fprintf(stderr) or blink and LED here to test your ISR
  delay__ms(100);
  LPC_GPIOINT->IO0IntClr |= (1 << 30);
  // gpio_set_high(2, 3);
  // delay__ms(100);
  fprintf(stderr, "Success\n");
  // delay__ms(500);
}*/