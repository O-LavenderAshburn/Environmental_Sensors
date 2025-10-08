#ifndef HTS221_SENSOR_H
#define HTS221_SENSOR_H

#include <stdint.h>
#include "hts221.h"

#define TEMPERATURE 0
#define HUMIDITY 1 

// Function declarations
int query_temperature(hts221_t *dev, int16_t *temperature);
int query_humidity(hts221_t *dev, uint16_t *humidity);
hts221_t* create_sensor(void);
// Added cleanup function
void destroy_sensor(hts221_t* dev); 

#endif /* HTS221_SENSOR_H */