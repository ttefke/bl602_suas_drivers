#ifndef __SUAS_I2C_H
#define __SUAS_I2C_H

#include <hal_i2c.h>

// Macros to ease function access
#define suas_i2c_write_byte(device, data, address) \
    hal_i2c_write_block(device, data, 1, 1, address)

#define suas_i2c_read_byte(device, data, address) \
    hal_i2c_read_block(device, data, 1, 1, address)

void suas_i2c_init();

#endif