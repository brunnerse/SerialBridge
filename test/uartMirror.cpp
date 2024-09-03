#include <cstring>
#include <string>

#include "pico/stdlib.h"

#include "cfg/uartConfig.h"
#include "hw/led.h"


#if USE_STDIO == 1
    #include <stdio.h>
#else
    #include "hardware/uart.h"
    #include "hardware/irq.h"

#endif

#define CONVERT_CRLF 1

#ifndef WAIT_FOR_NEWLINE
#define WAIT_FOR_NEWLINE 0
#endif


#define LED_BLINK_DURATION 100 


#if ADD_INFO
    #define PRE_STR "Received: "
    #if WAIT_FOR_NEWLINE
        #define POST_STR ""
    #else
        #define POST_STR "\r\n"
    #endif
#else
    #define PRE_STR ""
    #define POST_STR ""
#endif



#define BUFFER_SIZE 512


void send(const char* buffer)
{
#if USE_STDIO
//    puts_raw(buffer);
      printf("%s%s%s", PRE_STR, buffer, POST_STR);
#else
    uart_puts(uart_inst, PRE_STR);
    uart_puts(uart_inst, buffer);
    uart_puts(uart_inst, POST_STR);
#endif
}

void send(const char c)
{
#if USE_STDIO
    printf("%s%c%s", PRE_STR, c, POST_STR);
#else
    uart_puts(uart_inst, PRE_STR);
    uart_putc_raw(uart_inst, c);
    uart_puts(uart_inst, POST_STR);
#endif
}

void send_raw(const char c) {
#if USE_STDIO
    putchar(c);
#else
    uart_putc_raw(uart_inst, c);
#endif
}



int main() {

#if USE_STDIO
    //stdio_init_all();
    // while (!stdio_usb_connected())
    //    sleep_ms(100);
    stdio_uart_init_full(uart_inst, UART0_BAUDRATE, UART_TX_PIN, UART_RX_PIN);
    uart_set_format(uart_inst, UART_DATABITS, UART_STOPBITS, UART_PARITY);
#else
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);

    uart_init(uart_inst, UART_BAUDRATE);
    uart_init(uart_inst, UART_BAUDRATE);
    uart_set_format(uart_inst, UART_DATABITS, UART_STOPBITS, UART_PARITY);
    uart_set_translate_crlf(uart_inst, false);
#endif

    led_init(LED_PIN);
    uint64_t lastRecvTime = time_us_64();

    uint64_t lastSendTime = time_us_64();


#if WAIT_FOR_NEWLINE
    char sendBuffer[BUFFER_SIZE];
    uint32_t bufIdx = 0;
#endif


    while (1) {
        uint64_t time = time_us_64();
        if (time - lastRecvTime > LED_BLINK_DURATION * 1000)
            led_put(LED_PIN, 0);

        // Send ping if not sent something for 1 second 
        if (time - lastSendTime > 1000*1000) {
            send_raw('.'); 
            lastSendTime = time_us_64();
        }
#if USE_STDIO
        int cVal = getchar_timeout_us(100);
        if (cVal != PICO_ERROR_TIMEOUT) {
            char c = (char)cVal;
#else
        if (uart_is_readable_within_us(uart_inst, 100)) {
            char c = uart_getc(uart_inst);
#endif
            lastRecvTime = time_us_64();
            led_put(LED_PIN, 1);


#if WAIT_FOR_NEWLINE
            sendBuffer[bufIdx++] = (char)c;

            if (c == '\r' || c == '\n' || bufIdx >= BUFFER_SIZE-3) {
#if CONVERT_CRLF
                sendBuffer[bufIdx-1] = '\r';
                sendBuffer[bufIdx++] = '\n';
#endif
                sendBuffer[bufIdx] = '\0';
                send(sendBuffer);
                lastSendTime = time_us_64();
                bufIdx = 0;
            } 
#else
#if CONVERT_CRLF
            if (c == '\r' || c == '\n') {
                send('\r'); send('\n');
            } else {
                send(c);
            }
#else
            send(c);
#endif
            lastSendTime = time_us_64();
#endif // WAIT_FOR_NEWLINE
        }
    }

    return 0;
}