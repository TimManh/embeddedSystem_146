#include "FreeRTOS.h"
#include "pwm1.h"
#include "task.h"
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

void pwm_task(void *p) {
  pwm1__init_single_edge(1000);
  // Set port 2 and pin 0 as pwm
  pin_configure_pwm_channel_as_io_pin(2, 0);

  // We only need to set PWM configuration once, and the HW will drive
  // the GPIO at 1000Hz, and control set its duty cycle to 50%
//   pwm1__set_duty_cycle(PWM1__2_0, 0);

  // Continue to vary the duty cycle in the loop
  uint8_t percent = 0;
  while (1) {
    pwm1__set_duty_cycle(PWM1__2_0, percent);

    if (++percent > 100) {
      printf("%d\n", percent);
      percent = 0;
    }

    vTaskDelay(100);
  }
}

void main(void) {
  xTaskCreate(pwm_task, "start pwm", 4096, NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
}

/* -------------- Set the function pwm for port and pin number -------------- */

void pin_configure_pwm_channel_as_io_pin(uint32_t port, uint32_t pin) { gpio__construct_with_function(port, pin, 1); }
