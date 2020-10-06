/* -------------------------------------------------------------------------- */
/*                     headers and declaration of functions                   */
/* -------------------------------------------------------------------------- */

#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "periodic_scheduler.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "task.h"
#include <delay.h>
#include <stdio.h>
static SemaphoreHandle_t switch_pressed_signal;
void sleep_on_sem_task(void *p);
void gpio_interrupt(void);

/* -------------------------------------------------------------------------- */
/*                                    MAIN                                    */
/* -------------------------------------------------------------------------- */

void main(void) {

  switch_pressed_signal = xSemaphoreCreateBinary(); // NOTE: Create your binary semaphore

/* ---------------------- Generate Interrupt for p0.30 ---------------------- */
  //NOTE: This part also for part 0
  gpio_set_as_input(0, 30);
  LPC_IOCON->P0_30 &= ~(1 << 4);
  LPC_IOCON->P0_30 |= (1 << 3);
  LPC_GPIOINT->IO0IntEnF |= (1 << 30);
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio_interrupt, NULL);
  NVIC_EnableIRQ(GPIO_IRQn); // Enable interrupt gate for the GPIO

/* ---------------------- Generate Taske for semaphore ---------------------- */

  xTaskCreate(sleep_on_sem_task, "sem", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();

/* ----------------------------- Exit Interrupt ----------------------------- */

  /** NOTE: Checking the exit status of the interrupt by toggling an LED
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

/* -------------------------------------------------------------------------- */
/*                             Interrupt function                             */
/* -------------------------------------------------------------------------- */
//NOTE: this for part 0 as well
void gpio_interrupt(void) {
  fprintf(stderr, "ISR Entry");
  xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
  LPC_GPIOINT->IO0IntClr |= (1 << 30);
}

/* -------------------------------------------------------------------------- */
/*                        Semaphore get signal function                       */
/* -------------------------------------------------------------------------- */

void sleep_on_sem_task(void *p) {
  while (1) {
    //NOTE: Use xSemaphoreTake with forever delay and blink an LED when you get the signal
    if (xSemaphoreTake(switch_pressed_signal, portMAX_DELAY)) {
      gpio_set_as_output(1, 18);
      gpio_set_high(1, 18);
      vTaskDelay(500);
      gpio_set_low(1, 18);
    }
  }
}

