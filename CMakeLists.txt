cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall)

#include build functions from Pico SDK
set(PICO_BOARD "pico")
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(SerialBridge C CXX)


# UART Configuration
set(UART_BAUDRATE 57600)
set(UART_DATABITS 8) # 5..8
set(UART_PARITY EVEN) # ODD, NONE, EVEN
set(UART_STOPBITS 1) # 1..2

configure_file(cfg/uartConfig.h.in cfg/uartConfig.h)

set (UART_SERIAL_WAIT_FOR_NEWLINE 0)


# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()


# Add executables 
add_executable(SerialUartBridge
        src/SerialUartBridge.cpp
)

target_include_directories(SerialUartBridge PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})

target_compile_options(SerialUartBridge PRIVATE
	-DWAIT_FOR_NEWLINE=${UART_SERIAL_WAIT_FOR_NEWLINE}
)

pico_enable_stdio_usb(SerialUartBridge 1)
#pico_enable_stdio_uart(SerialUartBridge 0)

# create map/bin/hex/uf2 files.
pico_add_extra_outputs(SerialUartBridge)

target_link_libraries(SerialUartBridge
    hardware_uart
	pico_multicore
	pico_stdlib
	hardware_gpio
	$<$<STREQUAL:${PICO_BOARD},pico_w>:pico_cyw43_arch_none>
)



# Build second executable uartMirror for testing SerialUartBridge

set (UART_MIRROR_USE_STDIO 0)
set (UART_MIRROR_WAIT_FOR_NEWLINE 0)
set (UART_MIRROR_ADD_INFO 1)


add_executable(uartMirror
	test/uartMirror.cpp
)

target_compile_options(uartMirror PRIVATE
	-DUSE_STDIO=${UART_MIRROR_USE_STDIO}
	-DWAIT_FOR_NEWLINE=${UART_MIRROR_WAIT_FOR_NEWLINE}
	-DADD_INFO=${UART_MIRROR_ADD_INFO}
)

target_include_directories(uartMirror PRIVATE 
	${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})


if (UART_MIRROR_USE_STDIO)
	pico_enable_stdio_usb(uartMirror 0) 
	pico_enable_stdio_uart(uartMirror 1)
	target_link_libraries(uartMirror
		pico_stdlib
		hardware_gpio
		$<$<STREQUAL:${PICO_BOARD},pico_w>:pico_cyw43_arch_none>
	)
else()
	pico_enable_stdio_usb(uartMirror 1)
	pico_enable_stdio_uart(uartMirror 0)
	target_link_libraries(uartMirror
		hardware_uart
		pico_stdlib 
		hardware_gpio
		$<$<STREQUAL:${PICO_BOARD},pico_w>:pico_cyw43_arch_none>
	)
endif()

pico_add_extra_outputs(uartMirror)