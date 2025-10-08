#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"
#include "application.h"

// Print sensor response in formatted way
void print_sensor_response(const sensor_response_t *response) {
    printf("\n=== Sensor Query Response ===\n");
    printf("Request ID: %s\n", response->request_id);
    printf("Discovery Latency: %lu ms\n", response->discovery_latency_ms);
    
    if (response->success) {
        printf("Status: SUCCESS\n");
        printf("Value: %.1f %s\n", response->value, response->unit);
        printf("Timestamp: %lu\n", response->timestamp);
    } else {
        printf("Status: ERROR\n");
        printf("Error: %s\n", response->error_message);
    }
    printf("============================\n");
}