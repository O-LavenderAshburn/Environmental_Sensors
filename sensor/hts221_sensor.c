// hts221_sensor.c
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "hts221.h"
#include "hts221_params.h"
#include "hts221_sensor.h"

#define TEMPERATURE 0
#define HUMIDITY 1

/**
 * Query the temperature data of the HTS221 sensor.
 * returns: status of the read, 0 on success.
 */
int query_temperature(hts221_t *dev, int16_t *temperature) {

    if (hts221_one_shot(dev) != HTS221_OK) {
        puts("Error: HTS221 one-shot trigger failed");
        return -1;
    }
    int status;
    status = hts221_read_temperature(dev, temperature);
    return status;
}

/**
 * Query the humidity data of the HTS221 sensor.
 * returns: status of the read, 0 on success.
 */
int query_humidity(hts221_t *dev, uint16_t *humidity) {

    if (hts221_one_shot(dev) != HTS221_OK) {
        puts("Error: HTS221 one-shot trigger failed");
        return -1;
    }
    int status;

    status = hts221_read_humidity(dev, humidity);
    return status;
}

hts221_t* create_sensor(void) {
    hts221_t* dev = malloc(sizeof(hts221_t));
    if (dev == NULL) {
        puts("Error: Memory allocation failed");
        return NULL;
    }
    
    // Initialize the device
    if (hts221_init(dev, &hts221_params[0]) != HTS221_OK) {
        puts("Error: HTS221 init failed");
        free(dev);
        return NULL;
    }
    
    // Power on the device
    if (hts221_power_on(dev) != HTS221_OK) {
        puts("Error: HTS221 power on failed");
        free(dev);
        return NULL;
    }
    return dev;
}

/**
 * Cleanup function to properly destroy the sensor
 */
void destroy_sensor(hts221_t* dev) {
    if (dev != NULL) {
        // Power off the device before cleanup
        hts221_power_off(dev);
        free(dev);
    }
}
