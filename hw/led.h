#pragma once

#ifdef PICO_DEFAULT_LED_PIN
// Board is pico
#include "pico/stdlib.h"

#define LED_PIN PICO_DEFAULT_LED_PIN
#define led_init(x) {gpio_init((x)); gpio_set_dir((x), GPIO_OUT);}

#define led_put(x, y) gpio_put((x), (y))

#else
// Board is pico_w

#include "pico/cyw43_arch.h"

#define LED_PIN CYW43_WL_GPIO_LED_PIN
#define led_init(x) cyw43_arch_init()

#define led_put(x, y) cyw43_arch_gpio_put((x), (y))


#endif
