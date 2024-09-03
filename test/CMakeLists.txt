# Build  executable uartMirror for testing SerialUartBridge

set (UART_MIRROR_USE_STDIO 0)
set (UART_MIRROR_WAIT_FOR_NEWLINE 0)
set (UART_MIRROR_ADD_INFO 1)


add_executable(uartMirror
	uartMirror.cpp
)

target_compile_options(uartMirror PRIVATE
	-DUSE_STDIO=${UART_MIRROR_USE_STDIO}
	-DWAIT_FOR_NEWLINE=${UART_MIRROR_WAIT_FOR_NEWLINE}
	-DADD_INFO=${UART_MIRROR_ADD_INFO}
)

target_include_directories(uartMirror PRIVATE 
	${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR})


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