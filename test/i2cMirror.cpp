#include <cstring>
#include <string>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "cfg/i2cConfig.h"
#include "hw/led.h"


#define DEBUG

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
    i2c_write_raw_blocking(i2c_inst, PRE_STR, strlen(PRE_STR));
    i2c_write_raw_blocking(i2c_inst, buffer, strlen(buffer));
    i2c_write_raw_blocking(i2c_inst, POST_STR, strlen(POST_STR));
}

void send(const char c)
{
    i2c_write_raw_blocking(i2c_inst, PRE_STR, strlen(PRE_STR));
    i2c_write_raw_blocking(i2c_inst, c, sizeof(c));
    i2c_write_raw_blocking(i2c_inst, POST_STR, strlen(POST_STR));
}

void send_raw(const char c) {
    i2c_write_raw_blocking(i2c_inst, &c, sizeof(c));
}



int main() {

    // stdio_init_all();

    i2c_init(i2c_inst, I2C_BAUDRATE);

    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    // gpio_pull_up(I2C_SDA_PIN); 
    // gpio_pull_up(I2C_SCL_PIN); 

    i2c_set_slave_mode(i2c_inst, true, I2C_SLAVE_ADDR);


    led_init(LED_PIN);
    uint64_t lastRecvTime = time_us_64();

    uint64_t lastSendTime = time_us_64();


#if WAIT_FOR_NEWLINE
    char sendBuffer[BUFFER_SIZE];
    uint32_t bufIdx = 0;
#endif


    while (1) {
        uint64_t time = time_us_64();
        // handle LED
        if (time - lastRecvTime <= LED_BLINK_DURATION * 1000)
            led_put(LED_PIN, 1);
        else
            led_put(LED_PIN, 0);


        // Send ping if not sent something for 1 second 
        if (time - lastSendTime > 1000*1000) {
            // Intentionally do not use a blocking function so this write is ignored if 
            // the Tx queue is full
            i2c_write_byte_raw('.'); 
            lastSendTime = time_us_64();
        }

        if (i2c_get_read_available(i2c_inst)) {
            unsigned char c = i2c_read_byte_raw(uart0);
            lastRecvTime = time_us_64();

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
                send("\r\n");
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