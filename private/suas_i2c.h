#ifndef __SUAS_I2C_H
#define __SUAS_I2C_H

#include <hal_i2c.h>

// Macros to ease function access
#define suas_write_byte_i2c(device, data, address) \
    hal_i2c_write_block(device, data, 1, 1, address)

#define suas_read_byte_i2c(device, data, address) \
    hal_i2c_read_block(device, data, 1, 1, address)

void suas_initialize_i2c();

#endif