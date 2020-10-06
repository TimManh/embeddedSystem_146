#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "gpio_lab.h"
#include "periodic_scheduler.h"
#include "sj2_cli.h"
#include "task.h"
#include <delay.h>
#include <lpc40xx.h>
#include <stdio.h>
typedef struct {
  int port;
  uint8_t pin;
} port_pin_s;
static void led_task(void *pvParameters);
void main(void) {
  static port_pin_s led1 = {1, 26};
  static port_pin_s led2 = {1, 24};
  // NOTE: Completed first task
  // xTaskCreate(led_task, "LED", 4026 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(led_task, "LED", 4026 / sizeof(void *), &led1, PRIORITY_HIGH, NULL);

  xTaskCreate(led_task, "LED", 4026 / sizeof(void *), &led2, PRIORITY_LOW, NULL);

  vTaskStartScheduler();
}

/** NOTE: Function is Blinking one LED
static void led_task(void *pvParameters) {
  gpio_set_as_output(2, 3);
  while (true) {
    //  2) Set PIN register bit to 0 to turn ON LED (led may be active low)
    gpio_set_low(2, 3);
    vTaskDelay(500);

    //  3) Set PIN register bit to 1 to turn OFF LED
    gpio_set_high(2, 3);
    vTaskDelay(500);
  }
}*/
/** NOTE: Function is blinking one LED with parameter*/
void led_task(void *task_parameter) {
  // Type-cast the paramter that was passed from xTaskCreate()
  const port_pin_s *led = (port_pin_s *)(task_parameter);
  gpio_set_as_output(led->port, led->pin);
  while (true) {
    gpio_set_high(led->port, led->pin);
    vTaskDelay(500);

    gpio_set_low(led->port, led->pin);
    vTaskDelay(600);
  }
}
