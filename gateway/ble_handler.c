#include "ble_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "periph/rtc.h"
#include "ztimer.h"
#include "host/ble_gatt.h"
#include "host/ble_hs_adv.h"

// Globals
uint32_t scan_start_time = 0;
bool scan_timeout_active = false;
const char *target_type = NULL;
sensor_response_t *active_response = NULL;
static bool connecting = false;
static uint16_t conn_handle = 0;
static bool ess_found = false;


typedef struct packet_t {
    int16_t reading;
    uint32_t timestamp;
} packet_t;


/*
*Generate request id
*/
void generate_request_id(char *request_id, size_t size) {
    static uint32_t counter = 0;
    snprintf(request_id, size, "REQ_%lu_%lu", counter++, ztimer_now(ZTIMER_MSEC));
}

// Add this function to get elapsed scan time
uint32_t get_scan_elapsed_ms(void) {
    if (scan_start_time == 0) return 0;
    return ztimer_now(ZTIMER_MSEC) - scan_start_time;
}


int gatt_read_cb(uint16_t conn, const struct ble_gatt_error *error,
                 struct ble_gatt_attr *attr, void *arg)
{
    (void)arg;

    if (error->status != 0) {
        printf("[ERR] GATT read failed: %d\n", error->status);
        return 0;
    }

    if (!attr || !attr->om) {
        printf("[WARN] No attribute data received\n");
        return 0;
    }

    if (!active_response) {
        printf("[ERR] active_response is NULL\n");
        return 0;
    }

    size_t om_len = OS_MBUF_PKTLEN(attr->om);

    // Check if we have at least the 2-byte reading
    if (om_len < sizeof(int16_t)) {
        printf("[WARN] Not enough data (%zu bytes)\n", om_len);
        return 0;
    }

    uint8_t buf[6] = {0}; // Max size for reading + timestamp (2 + 4)
    size_t copy_len = om_len < sizeof(buf) ? om_len : sizeof(buf);
    os_mbuf_copydata(attr->om, 0, copy_len, buf);

    int16_t reading = buf[0] | (buf[1] << 8);
    uint32_t timestamp = 0;

    // If we have more than 2 bytes, assume the next 4 are the timestamp
    if (copy_len >= 6) {
        timestamp = buf[2] | (buf[3] << 8) | (buf[4] << 16) | (buf[5] << 24);
    } else {
        // Fallback: use local time
        timestamp = ztimer_now(ZTIMER_MSEC);
    }

    active_response->value = reading / 100.0;

    if (strcmp(active_response->unit, "Celsius") == 0) {
        printf("[INFO] Temperature: %.2f °C @ %lu \n",
               active_response->value, (unsigned long)timestamp);
    } else if (strcmp(active_response->unit, "Percent") == 0) {
        printf("[INFO] Humidity: %.2f %% @ %lu \n",
               active_response->value, (unsigned long)timestamp);
    } else {
        printf("[WARN] Unknown unit: %s\n", active_response->unit);
        return 0;
    }

    // Save success and timestamp
    active_response->success = true;
    active_response->timestamp = timestamp;

    print_sensor_response(active_response);

    // Disconnect safely
    ble_gap_terminate(conn, BLE_ERR_REM_USER_CONN_TERM);

    return 0;
}

// Characteristic Discovery 
int chr_disc_cb(uint16_t conn, const struct ble_gatt_error *error,
                const struct ble_gatt_chr *chr, void *arg)
{
    (void)arg;

    if (error->status == BLE_HS_EDONE) {
        printf("[INFO] Characteristic discovery complete\n");
        return 0;
    }

    if (error->status != 0) {
        printf("[ERROR] Characteristic discovery failed: %d\n", error->status);
        return 0;
    }

    if (!chr) return 0;

    if (chr->uuid.u.type == BLE_UUID_TYPE_16) {
        uint16_t uuid = chr->uuid.u16.value;
        printf("[DEBUG] Found characteristic UUID: 0x%04X, handle: %d\n",
               uuid, chr->val_handle);

        ble_gattc_read(conn, chr->val_handle, gatt_read_cb, NULL);
    }

    return 0;
}

// Service Discovery
int svc_disc_cb(uint16_t conn, const struct ble_gatt_error *error,
                const struct ble_gatt_svc *svc, void *arg)
{
    (void)arg;

    if (error->status == BLE_HS_EDONE) {
        if (!ess_found) {
            printf("[WARN] ESS service not found on this device (search completed)\n");
            ble_gap_terminate(conn, BLE_ERR_REM_USER_CONN_TERM);
        } else {
            printf("[INFO] Service discovery complete\n");
        }
        return 0;
    }

    if (!svc) {
        printf("[WARN] No service data received\n");
        return 0;
    }

    printf("[DEBUG] Found service: UUID=");
    if (svc->uuid.u.type == BLE_UUID_TYPE_16) {
        printf("0x%04X", svc->uuid.u16.value);
    } else {
        printf("(128-bit UUID)");
    }
    printf(", Handle range: %d to %d\n", svc->start_handle, svc->end_handle);

    if (svc->uuid.u.type == BLE_UUID_TYPE_16 &&
        svc->uuid.u16.value == ENV_SENSING_SERVICE_UUID) {
        ess_found = true;
        printf("[SUCCESS] Found ESS service! Handle range: %d to %d\n",
               svc->start_handle, svc->end_handle);

        int rc = ble_gattc_disc_all_chrs(conn, svc->start_handle, svc->end_handle,
                                         chr_disc_cb, NULL);
        if (rc != 0) {
            printf("[ERROR] Characteristic discovery initiation failed: %d\n", rc);
            ble_gap_terminate(conn, BLE_ERR_REM_USER_CONN_TERM);
        }
    }

    return 0;
}

/**Gap Event */
int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {

        case BLE_GAP_EVENT_CONNECT:
            connecting = false;
            if (event->connect.status == 0) {
                conn_handle = event->connect.conn_handle;
                printf("[SUCCESS] Connected! Handle: %d\n", conn_handle);
                
                // Start service discovery immediately after connection
                if(DEBUG){
                    printf("[DEBUG] Starting ESS service discovery...\n");                
                    printf("[DEBUG] Discovering ALL services...\n");
                }
          
                int rc = ble_gattc_disc_all_svcs(conn_handle, svc_disc_cb, NULL);
                
                if (rc != 0) {
                    printf("[ERROR] Service discovery initiation failed: %d\n", rc);
                    // Disconnect if we can't start discovery
                    ble_gap_terminate(conn_handle, BLE_ERR_REM_USER_CONN_TERM);
                }
            } else {
                printf("[ERROR] Connection failed: %d\n", event->connect.status);
                nimble_scanner_start();
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            printf("[INFO] Disconnected: reason=%d\n", event->disconnect.reason);
            connecting = false;
            conn_handle = 0;
            break;

        default:
            printf("[DEBUG] Unhandled GAP event: %d\n", event->type);
            break;
    }


    return 0;
}

/**Scanner callback function */
void scan_cb(uint8_t type, const ble_addr_t *addr,
             const nimble_scanner_info_t *info,
             const uint8_t *ad, size_t ad_len)
{
    (void)type;


    uint16_t my_sensor_uuid = 0;

    if (strcmp(target_type, "TEMP") == 0) {
        my_sensor_uuid = TEMPERATURE_CHARACTERISTIC_UUID;  
        printf("[INFO] Looking for sensor type: %s\n", target_type);

    } else if (strcmp(target_type, "HUM") == 0) {
        my_sensor_uuid = HUMIDITY_CHARACTERISTIC_UUID;     
        printf("[INFO] Looking for sensor type: %s\n", target_type);

    } else {
        printf("[ERR] Unknown sensor type: %s\n", target_type);
        return;
    }

    if (connecting) return;

    struct ble_hs_adv_fields fields;
    if (ble_hs_adv_parse_fields(&fields, ad, ad_len) != 0) {
        printf("[WARN] Failed to parse advertisement\n");
        return;
    }

    if(DEBUG)
        printf("[DEBUG] Device found: RSSI=%d dBm\n", info->rssi);

    for (int i = 0; i < fields.num_uuids16; i++) {
        if(DEBUG)
            printf("[DEBUG] UUID16: 0x%04X\n", fields.uuids16[i].value);

        // look for the correct UUID based on the sensor
        if (fields.uuids16[i].value == my_sensor_uuid) {
            uint32_t scan_time_ms = get_scan_elapsed_ms();


            printf("[INFO] Found target sensor in %lu ms (RSSI: %d dBm)\n", 
                scan_time_ms, info->rssi);
            active_response->discovery_latency_ms = scan_time_ms;
            if(DEBUG)
                printf("[DEBUG] Found ESS device (RSSI: %d dBm)\n", info->rssi);
            connecting = true;
            scan_timeout_active =false;
            

            nimble_scanner_stop();

            struct ble_gap_conn_params conn_params = {
                .scan_itvl = 0x0010,    
                .scan_window = 0x0010,  
                .itvl_min = BLE_GAP_INITIAL_CONN_ITVL_MIN,
                .itvl_max = BLE_GAP_INITIAL_CONN_ITVL_MAX,
                .latency = 0,
                .supervision_timeout = BLE_GAP_INITIAL_SUPERVISION_TIMEOUT,
                .min_ce_len = BLE_GAP_INITIAL_CONN_MIN_CE_LEN,
                .max_ce_len = BLE_GAP_INITIAL_CONN_MAX_CE_LEN,
            };
            
            int rc = ble_gap_connect(BLE_OWN_ADDR_RANDOM, addr, 2500, &conn_params, gap_event_cb, NULL);
            if (rc != 0) {
                printf("[ERROR] Connection failed: %d\n", rc);
                connecting = false;
                nimble_scanner_start();
            }
            return;
        }
    }
}