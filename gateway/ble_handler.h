#ifndef BLE_HANDLER_H
#define BLE_HANDLER_H

#ifndef DEBUG
#define DEBUG 0
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "application.h"   
#include "host/ble_uuid.h"
#include "nimble_scanner.h"
#include "host/ble_hs.h"
#define ENV_SENSING_SERVICE_UUID     0x181A
#define TEMPERATURE_CHARACTERISTIC_UUID 0x2A6E
#define HUMIDITY_CHARACTERISTIC_UUID    0x2A6F
#define PRESSURE_CHARACTERISTIC_UUID    0x2A6D

// BLE scan defaults
#define DEFAULT_SCAN_INTERVAL_MS 30
#define DEFAULT_SCAN_DURATION_MS 9000  // 9 seconds scan

extern uint32_t scan_start_time;
extern bool scan_timeout_active;


extern const char *target_type;
extern sensor_response_t *active_response;

void scan_cb(uint8_t type, const ble_addr_t *addr,
             const nimble_scanner_info_t *info,
             const uint8_t *ad, size_t ad_len);

void generate_request_id(char *request_id, size_t size);
int gap_event_cb(struct ble_gap_event *event, void *arg);
int gatt_read_cb(uint16_t conn_handle_param, const struct ble_gatt_error *error,
                 struct ble_gatt_attr *attr, void *arg);
void check_scan_timeout(void);

#endif /* BLE_HANDLER_H */
