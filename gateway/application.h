#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TARGET_DEVICE_PREFIX "EnvSensor"  // Look for devices with this name prefix
#define TEMP_SENSOR_NAME "EnvSensor-TEMP"
#define HUM_SENSOR_NAME "EnvSensor-HUM"
#define TEMP_SENSOR_ID 0
#define HUMID_SENSOR_ID 1

// Sensor response structure
typedef struct sensor_response_t{
    char request_id[32];           // Unique identifier for the request
    bool success;                  // Whether the query succeeded
    double value;                   // Sensor reading value
    uint32_t timestamp;            // Unix timestamp of the reading
    char unit[16];                 // Unit ("Celsius" or "Percent")
    uint32_t discovery_latency_ms; // Time to discover sensor
    char error_message[64];        // Error description if failed
} sensor_response_t;

void print_sensor_response(const sensor_response_t *response);


#endif /* GATEWAY_H */