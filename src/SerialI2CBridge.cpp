#include <stdio.h>
//#include <string>
//#include <cstring>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/i2c.h"

#include "cfg/i2cConfig.h"

// Set to 0 if no blinking
#define LED_BLINK_DURATION 100 

#if LED_BLINK_DURATION > 0
#include "hw/led.h"
#endif


#define BUFFER_SIZE 512


char buffer_in [BUFFER_SIZE];
volatile size_t head = 0, tail = 0;


volatile uint64_t lastRecvTime = 0;


void serialIn() {

    // Block until there is at least one space in the buffer
    while (head+1 == tail)
        ;

    int c = getchar_timeout_us(100);
    if (c != PICO_ERROR_TIMEOUT) {
        buffer_in[head++] = (char)c;
        // wrap head
        if (head == BUFFER_SIZE)
            head = 0;
        lastRecvTime = time_us_64(); 
    }
}

/*
void uart2serial() {
#if WAIT_FOR_NEWLINE
    static char buffer[BUFFER_SIZE];
    static uint32_t idx = 0;
#endif

    if (uart_is_readable_within_us(uart0, 100)) {
        char c = uart_getc(uart0);
#if WAIT_FOR_NEWLINE
        buffer[idx++] = c;
        if (c == '\n' || idx >= BUFFER_SIZE) {
            for (uint32_t i = 0; i < idx; i++)
                putchar(buffer[i]);
            idx = 0;
        }
#else
        putchar(c);
#endif
        lastRecvTime = time_us_64(); 
   }
}

void core1task() {
    for (;;)
        uart2serial();
}
*/


#if LED_BLINK_DURATION > 0
void handleBlink() {
    int64_t time = time_us_64();
    if (time - lastRecvTime <= LED_BLINK_DURATION * 1000) 
        led_put(LED_PIN, 1);
    else 
        led_put(LED_PIN, 0);
}
#endif


int main() {
    stdio_init_all();

    i2c_init(i2c_inst, I2C_BAUDRATE);

    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA_PIN); 
    gpio_pull_up(I2C_SCL_PIN); 

    i2c_set_slave_mode(i2c_inst, false, 0x00);

#if LED_BLINK_DURATION > 0
    led_init(LED_PIN);
    led_put(LED_PIN, 1);
    sleep_ms(500);
    led_put(LED_PIN, 0);
#endif

    // Launch core1 task which forwards uart input to serial output
//    multicore_launch_core1(core1task);

    // On core 0, forward serial output to uart input and handle LED blinking 
    for (;;) {
        serialIn();
#if LED_BLINK_DURATION > 0
        handleBlink();
#endif
    }



    return 0;
}