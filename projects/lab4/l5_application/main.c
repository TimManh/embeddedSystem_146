#include "FreeRTOS.h"
#include "adc.h"
#include "pwm1.h"
#include "queue.h"
#include "task.h"
#include <gpio.h>
#include <stdio.h>
void pin_configure_pwm_channel_as_io_pin(uint32_t port, uint32_t pin);

/* -------------------------------------------------------------------------- */
/*                                   PART 0                                   */
/* @param   void                                                              */
/* @return  void                                                              */
/* @description   function generate pwm for port2 and pin0 with single edge   */
/*                configuration with MR0 and  and increase duty cycle in the  */
/*                  loop until it hit 100%                                    */
/* -------------------------------------------------------------------------- */

// void pwm_task(void *p) {
//   pwm1__init_single_edge(1000);
//   // Set port 2 and pin 0 as pwm
//   pin_configure_pwm_channel_as_io_pin(2, 0);

//   // We only need to set PWM configuration once, and the HW will drive
//   // the GPIO at 1000Hz, and control set its duty cycle to 50%
//   //   pwm1__set_duty_cycle(PWM1__2_0, 0);

//   // Continue to vary the duty cycle in the loop
//   uint8_t percent = 0;
//   while (1) {
//     pwm1__set_duty_cycle(PWM1__2_0, percent);

//     if (++percent > 100) {
//       percent = 0;
//     }

//     vTaskDelay(100);
//   }
// }

/* -------------------------------------------------------------------------- */
/*                                   part 1                                   */
/* -------------------------------------------------------------------------- */
void pin_configure_adc_channel_as_io_pin(uint32_t port, uint32_t pin, gpio__function_e func);
uint16_t adc__get_channel_reading_with_burst_mode(adc_channel_e channel_adc);
// void adc_task(void *p) {
//   adc__initialize();
//   LPC_GPIO0->DIR |= (1 << 25);
//   LPC_IOCON->P0_25 &= ~(1 << 7);
//   // You can configure burst mode for just the channel you are using
//   adc__enable_burst_mode(ADC__CHANNEL_2);

//   // Configure a pin, such as P1.31 with FUNC 011 to route this pin as ADC channel 5
//   // You can use gpio__construct_with_function() API from gpio.h
//   pin_configure_adc_channel_as_io_pin(0, 25, GPIO__FUNCTION_1);

//   while (1) {
//     // Get the ADC reading using a new routine you created to read an ADC burst reading
//     const uint16_t adc_value = adc__get_channel_reading_with_burst_mode(ADC__CHANNEL_2);
//     double temp = (double)(adc_value) / 4095.0 * 3.3;
//     fprintf(stderr, "%f\n", temp);
//     vTaskDelay(500);
//   }
// }

/* -------------------------------------------------------------------------- */
/*                                   part 2                                   */
/* -------------------------------------------------------------------------- */
// This is the queue handle we will need for the xQueue Send/Receive API
static QueueHandle_t adc_to_pwm_task_queue;

void adc_task(void *p) {
  adc__initialize();
  LPC_GPIO0->DIR |= (1 << 25);
  LPC_IOCON->P0_25 &= ~(1 << 7);
  // You can configure burst mode for just the channel you are using
  adc__enable_burst_mode(ADC__CHANNEL_2);

  // Configure a pin, such as P1.31 with FUNC 011 to route this pin as ADC channel 5
  // You can use gpio__construct_with_function() API from gpio.h
  pin_configure_adc_channel_as_io_pin(0, 25, GPIO__FUNCTION_1);

  int adc_reading = 0; // Note that this 'adc_reading' is not the same variable as the one from adc_task
  while (1) {
    // Implement code to send potentiometer value on the queue
    // a) read ADC input to 'int adc_reading'
    adc_reading = adc__get_channel_reading_with_burst_mode(ADC__CHANNEL_2);
    double temp = (double)(adc_reading) / 4095.0 * 3.3;
    fprintf(stderr, "%f\n", temp);
    // b) Send to queue: xQueueSend(adc_to_pwm_task_queue, &adc_reading, 0);
    xQueueSend(adc_to_pwm_task_queue, &adc_reading, 0);

    vTaskDelay(100);
  }
}

void pwm_task(void *p) {
  // NOTE: Reuse the code from Part 0
  pwm1__init_single_edge(1000);
  // Set port 2 and pin 0 as pwm
  pin_configure_pwm_channel_as_io_pin(2, 0);
  int adc_reading = 0;

  while (1) {
    // Implement code to receive potentiometer value from queue
    if (xQueueReceive(adc_to_pwm_task_queue, &adc_reading, 100)) {
      double percent = (double)(adc_reading) / 4095.0 * 100;
      pwm1__set_duty_cycle(PWM1__2_0, percent);
    }

    // We do not need task delay because our queue API will put task to sleep when there is no data in the queue
    // vTaskDelay(100);
  }
}

/* -------------------------------------------------------------------------- */
/*                                    MAIN                                    */
/* -------------------------------------------------------------------------- */

void main(void) {

  /* --------------------------------- part 0 --------------------------------- */

  //   xTaskCreate(pwm_task, "start pwm", 4096, NULL, PRIORITY_LOW, NULL);

  /* --------------------------------- part 1 --------------------------------- */
  //   xTaskCreate(adc_task, "start adc", 4096, NULL, PRIORITY_LOW, NULL);

  /* --------------------------------- part 2 --------------------------------- */
  adc_to_pwm_task_queue = xQueueCreate(1, sizeof(int));

  xTaskCreate(adc_task, "start adc", 2048, NULL, PRIORITY_LOW, NULL);
  xTaskCreate(pwm_task, "start pwm", 2048, NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
}

/* -------------- Set the function pwm for port and pin number -------------- */

void pin_configure_pwm_channel_as_io_pin(uint32_t port, uint32_t pin) { gpio__construct_with_function(port, pin, 1); }

/* -------------- Set the function adc for port and pin number -------------- */

void pin_configure_adc_channel_as_io_pin(uint32_t port, uint32_t pin, gpio__function_e func) {
  gpio__construct_with_function(port, pin, func);
}

/* ----------------------- Set the burst mode  for adc ---------------------- */

uint16_t adc__get_channel_reading_with_burst_mode(adc_channel_e channel_adc) {
  uint16_t result = 0;
  if (LPC_ADC->DR[channel_adc] & (1 << 31)) {
    result = ((LPC_ADC->DR[channel_adc] & (0x0FFF << 4)) >> 4);
  }

  return result;
}