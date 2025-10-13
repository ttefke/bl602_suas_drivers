// FreeRTOS includes (required to lock the critical section)
#include <FreeRTOS.h>
#include <task.h>

// Required to have access to the float macros
#include <float.h>

// Temperature / relative humidity variables
static float dht22_rh = FLT_MIN;
static float dht22_tmp = FLT_MIN;

// Internal function to query the sensor
static void suas_dht22_get_readings(uint8_t data_pin) {
  // Allocate data array and initialize to zero
  uint8_t sensor_data[5] = {0};

  // Add prototype for our assembler function in the .S file so that the
  // compiler knows about it
  extern void suas_dht22_get_readings_asm(uint8_t data_pin, uint8_t *data);

  taskENTER_CRITICAL();  // The communication is very time critical -> disable
                         // interrupts
  suas_dht22_get_readings_asm(data_pin,
                              sensor_data);  // Request data from sensor
  taskEXIT_CRITICAL();  // We are done with the communication -> restore
                        // interrupts

  // Compute temperature values if data matches checksum
  if (((sensor_data[0] + sensor_data[1] + sensor_data[2] + sensor_data[3]) &
       0xFF) == sensor_data[4]) {
    // Compute relative humidity
    uint16_t rh_data = (sensor_data[0] << 8);
    rh_data += sensor_data[1];
    dht22_rh = rh_data / 10.0f;

    // Compute temperature
    uint16_t tmp_data = (sensor_data[2] << 8);
    tmp_data += sensor_data[3];
    dht22_tmp = tmp_data / 10.0f;
  } else {
    // Set to minimal values if data is incorrect
    dht22_rh = FLT_MIN;
    dht22_tmp = FLT_MIN;
  }
}

// Used to query the temperature
float suas_dht22_get_temperature(uint8_t data_pin) {
  suas_dht22_get_readings(data_pin);
  return dht22_tmp;
}

// Used to query the humidity
float suas_dht22_get_humidity(uint8_t data_pin) {
  suas_dht22_get_readings(data_pin);
  return dht22_rh;
}