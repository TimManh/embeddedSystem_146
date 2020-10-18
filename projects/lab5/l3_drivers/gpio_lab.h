#pragma once
#include "../lpc40xx.h"
#include <stdbool.h>
#include <stdint.h>

/// Should alter the hardware registers to set the pin as input
void gpio_set_as_input(int port_num, uint8_t pin_num);

/// Should alter the hardware registers to set the pin as output
void gpio_set_as_output(int port_num, uint8_t pin_num);

/// Should alter the hardware registers to set the pin as high
void gpio_set_high(int port_num, uint8_t pin_num);

/// Should alter the hardware registers to set the pin as low
void gpio_set_low(int port_num, uint8_t pin_num);

/**
 * Should alter the hardware registers to set the pin as low
 *
 * @param {bool} high - true => set pin high, false => set pin low
 */
void gpio_set(int port_num, uint8_t pin_num, bool high);

/**
 * Should return the state of the pin (input or output, doesn't matter)
 *
 * @return {bool} level of pin high => true, low => false
 */
bool gpio_get_level(int port_num, uint8_t pin_num);
