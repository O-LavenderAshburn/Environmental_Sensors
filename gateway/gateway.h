// gateway.h
#ifndef GATEWAY_H
#define GATEWAY_H

#include <stdint.h>
#include <stdbool.h>
#include "application.h" 

#define DEFAULT_SCAN_INTERVAL_MS    30

// default scan duration (1s)
#define DEFAULT_DURATION_MS        (1 * MS_PER_SEC)
#define SENSOR_TEMP  0
#define SENSOR_HUM   1
#define SCAN_TIMEOUT_MS 2000

extern bool connecting;
extern sensor_response_t *active_response;

void ble_query_sensor(int sensor_type);

#endif
