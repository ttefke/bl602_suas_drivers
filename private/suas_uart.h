#ifndef __SUAS_UART_H
#define __SUAS_UART_H

// UART HAL
#include <bl_uart.h>

// Define some wrappers easing the HAL use
#define suas_uart_init(baud_rate) \
    bl_uart_init( \
        1 /* Channel 0 = USB, Channel 1 = user defined UART*/, \
        3, /* TX pin*/ \
        4, /* RX pin*/ \
        255, /* Unused*/ \
        255, /* Unused */ \
        baud_rate /* Baud rate */ \
    )

#define suas_uart_write_byte(data) \
    bl_uart_data_send ( \
        1, /* Channel */ \
        data \
    )

#define suas_uart_write_bytes(data, len) \
    bl_uart_datas_send ( \
        1, /* Channel */ \
        data, \
        len \
    )

#define suas_uart_read_byte() \
    bl_uart_data_recv( \
        1 /* Channel */ \
    )

#endif /* __SUAS_UART_H */