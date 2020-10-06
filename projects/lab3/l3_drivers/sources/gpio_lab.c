#include "gpio_lab.h"

void gpio_set_as_input(int port_num, uint8_t pin_num) {
  switch (port_num) {
  case 0:
    LPC_GPIO0->DIR &= ~(1 << pin_num);
    break;
  case 1:
    LPC_GPIO1->DIR &= ~(1 << pin_num);
    break;
  case 2:
    LPC_GPIO2->DIR &= ~(1 << pin_num);
    break;
  case 3:
    LPC_GPIO3->DIR &= ~(1 << pin_num);
    break;
  case 4:
    LPC_GPIO4->DIR &= ~(1 << pin_num);
    break;
  case 5:
    LPC_GPIO5->DIR &= ~(1 << pin_num);
    break;
  default:
    break;
  }
}

void gpio_set_as_output(int port_num, uint8_t pin_num) {
  switch (port_num) {
  case 0:
    LPC_GPIO0->DIR |= (1 << pin_num);
    break;
  case 1:
    LPC_GPIO1->DIR |= (1 << pin_num);
    break;
  case 2:
    LPC_GPIO2->DIR |= (1 << pin_num);
    break;
  case 3:
    LPC_GPIO3->DIR |= (1 << pin_num);
    break;
  case 4:
    LPC_GPIO4->DIR |= (1 << pin_num);
    break;
  case 5:
    LPC_GPIO5->DIR |= (1 << pin_num);
    break;
  default:
    break;
  }
}

void gpio_set_high(int port_num, uint8_t pin_num) {
  switch (port_num) {
  case 0:
    LPC_GPIO0->SET |= (1 << pin_num);
    break;
  case 1:
    LPC_GPIO1->SET |= (1 << pin_num);
    break;
  case 2:
    LPC_GPIO2->SET |= (1 << pin_num);
    break;
  case 3:
    LPC_GPIO3->SET |= (1 << pin_num);
    break;
  case 4:
    LPC_GPIO4->SET |= (1 << pin_num);
    break;
  case 5:
    LPC_GPIO5->SET |= (1 << pin_num);
    break;
  default:
    break;
  }
}

void gpio_set_low(int port_num, uint8_t pin_num) {
  switch (port_num) {
  case 0:
    LPC_GPIO0->CLR |= (1 << pin_num);
    break;
  case 1:
    LPC_GPIO1->CLR |= (1 << pin_num);
    break;
  case 2:
    LPC_GPIO2->CLR |= (1 << pin_num);
    break;
  case 3:
    LPC_GPIO3->CLR |= (1 << pin_num);
    break;
  case 4:
    LPC_GPIO4->CLR |= (1 << pin_num);
    break;
  case 5:
    LPC_GPIO5->CLR |= (1 << pin_num);
    break;
  default:
    break;
  }
}

void gpio_set(int port_num, uint8_t pin_num, bool high) {
  if (high) {
    gpio_set_high(pin_num, port_num);
  } else {
    gpio_set_low(pin_num, port_num);
  }
}

bool gpio_get_level(int port_num, uint8_t pin_num) {
  bool on = false;
  switch (port_num) {
  case 0:
    /* code */
    {
      if ((LPC_GPIO0->SET & (1 << pin_num)) != 0) {
        on = true;
      }
      break;
    }
  case 1:
    /* code */
    {
      if ((LPC_GPIO1->SET & (1 << pin_num)) != 0) {
        on = true;
      }
      break;
    }
  case 2:
    /* code */
    {
      if ((LPC_GPIO2->SET & (1 << pin_num)) != 0) {
        on = true;
      }
      break;
    }
  case 3:
    /* code */
    {
      if ((LPC_GPIO3->SET & (1 << pin_num)) != 0) {
        on = true;
      }
      break;
    }
  case 4:
    /* code */
    {
      if ((LPC_GPIO4->SET & (1 << pin_num)) != 0) {
        on = true;
      }
      break;
    }
  case 5:
    /* code */
    {
      if ((LPC_GPIO5->SET & (1 << pin_num)) != 0) {
        on = true;
      }
      break;
    }
  default:
    break;
  }
  return on;
}