#include <stdio.h>
//#include <string>
//#include <cstring>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/uart.h"

#include "cfg/uartConfig.h"


// Set to 0 if no blinking
#define LED_BLINK_DURATION 100 

#if LED_BLINK_DURATION > 0
#include "hw/led.h"
#endif


#define UART0_TX_PIN 0
#define UART0_RX_PIN 1

#if WAIT_FOR_NEWLINE
#define BUFFER_SIZE 512
#endif


volatile uint64_t lastRecvTime = 0;


void serial2uart() {
#if WAIT_FOR_NEWLINE
    static char buffer[BUFFER_SIZE];
    static uint32_t idx = 0;
#endif

    int c = getchar_timeout_us(100);
    if (c != PICO_ERROR_TIMEOUT) {
#if WAIT_FOR_NEWLINE
        buffer[idx++] = (char)c;
        if (c == '\n' || idx >= BUFFER_SIZE) {
            for (uint32_t i = 0; i < idx; i++)
                uart_putc_raw(uart0, (char)buffer[i]);
            idx = 0;
        }
#else
        uart_putc_raw(uart0, (char)c);
#endif
        lastRecvTime = time_us_64(); 
    }
}

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

    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);

    uart_init(uart0, UART_BAUDRATE);
    uart_set_format(uart0, UART_DATABITS, UART_STOPBITS, UART_PARITY);


#if LED_BLINK_DURATION > 0
    led_init(LED_PIN);
    led_put(LED_PIN, 1);
    sleep_ms(500);
    led_put(LED_PIN, 0);
#endif

    // Launch core1 task which forwards uart input to serial output
    multicore_launch_core1(core1task);

    // On core 0, forward serial output to uart input and handle LED blinking 
    for (;;) {
        serial2uart();
#if LED_BLINK_DURATION > 0
        handleBlink();
#endif
    }



    return 0;
}