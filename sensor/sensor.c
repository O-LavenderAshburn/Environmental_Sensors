#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hts221_sensor.h"
#include "nimble_riot.h"
#include "nimble_autoadv.h"

#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

// UUIDs for Environmental Sensing Service & Characteristics 
#define ENV_SENSING_SERVICE_UUID     0x181A
#define TEMPERATURE_CHAR_UUID        0x2A6E
#define HUMIDITY_CHAR_UUID           0x2A6F

#ifndef SENSOR_TYPE
#define SENSOR_TYPE 0  
#endif


/**Compile time Initilization */
#if SENSOR_TYPE == 0
    #define SENSOR_CHAR_UUID 0x2A6E
    #define SENSOR_ACCESS_CB gatt_svr_chr_access_temperature
#elif SENSOR_TYPE == 1
    #define SENSOR_CHAR_UUID 0x2A6F
    #define SENSOR_ACCESS_CB gatt_svr_chr_access_humidity
#else
    #error "Unknown SENSOR_TYPE"
#endif

/**BLE Packet */
typedef struct packet_t {
    int16_t reading;
    uint32_t timestamp;
} packet_t;

static hts221_t *sensor_dev = NULL;
static int init_sensor(void)
{
    sensor_dev = create_sensor();
    if (sensor_dev == NULL) {
        printf("Warning: Failed to initialize HTS221 sensor, using fallback values\n");
        return -1;
    }
    printf("HTS221 sensor initialized successfully\n");
    return 0;
}



//TODO: Make gatt_svr_chr_access_temperature & gatt_svr_chr_access_humidity as they are basically the same bar the sensor type use.
/** Access callback for temperature characteristic */
static int __attribute__((unused)) gatt_svr_chr_access_temperature(uint16_t conn_handle,
                                           uint16_t attr_handle,
                                           struct ble_gatt_access_ctxt *ctxt,
                                           void *arg)
{
    (void)conn_handle;
    (void)attr_handle;
    (void)arg;
    
    int rc = 0;
    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
    {
        packet_t pkt = {0};

        if (sensor_dev != NULL) {
            int status = query_temperature(sensor_dev, &pkt.reading);
            if (status == 0) {
                printf("Temperature: %d \n", pkt.reading);
            }
            else {
                printf("Sensor read failed, fallback temp: %d\n", pkt.reading);
            }
        }

        /* get timestamp in ms */
        pkt.timestamp = ztimer_now(ZTIMER_MSEC);

        rc = os_mbuf_append(ctxt->om, &pkt, sizeof(pkt));
    }
    break;
    }
    return rc;
}

/** Access callback for humidity characteristic */
static int __attribute__((unused)) gatt_svr_chr_access_humidity(uint16_t conn_handle,
                                        uint16_t attr_handle,
                                        struct ble_gatt_access_ctxt *ctxt,
                                        void *arg)
{
    (void)conn_handle;
    (void)attr_handle;
    (void)arg;
        
    int rc = 0;
    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
    {
        packet_t pkt = {0};

        if (sensor_dev != NULL) {
            int status = query_humidity(sensor_dev, (uint16_t *)&pkt.reading);
            if (status == 0) {
                printf("Humidity: %d \n", pkt.reading);
            }
            else {
                printf("Sensor read failed, fallback humidity: %d\n", pkt.reading);
            }
        }

        pkt.timestamp = ztimer_now(ZTIMER_MSEC);

        rc = os_mbuf_append(ctxt->om, &pkt, sizeof(pkt));
    }
    break;
    }
    return rc;
}

/** GATT service definition */
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(ENV_SENSING_SERVICE_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                /* Temperature characteristic */
                .uuid = BLE_UUID16_DECLARE(SENSOR_CHAR_UUID),
                .access_cb = SENSOR_ACCESS_CB,
                .flags = BLE_GATT_CHR_F_READ,
            },
            { 0 } 
        }
    },
    { 0 } 
};

// GAP event handler - called for connection/disconnection events 
static int gap_event_handler(struct ble_gap_event *event, void *arg)
{

    (void)arg;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            printf("Device connected\n");
        } else {
            printf("Connection failed; status=%d\n", event->connect.status);
            /* Restart advertising on connection failure */
            nimble_autoadv_start(NULL);
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        printf("Device disconnected; reason=%d\n", event->disconnect.reason);
        /* Restart advertising after disconnection */
        nimble_autoadv_start(NULL);
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        printf("Advertising complete; reason=%d\n", event->adv_complete.reason);
        break;       
    default:
        break;
    }
    return 0;
}

/**
* Application main
*/
int main(void)
{
    int rc;

    init_sensor();
    // Verify and add our custom GATT services
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    assert(rc == 0);
    
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    assert(rc == 0);

    // Set the device name 
    ble_svc_gap_device_name_set("RIOT-EnvSensor");
    ble_gatts_start();
    /* Set GAP event handler using the correct API */
    nimble_autoadv_set_gap_cb(gap_event_handler, NULL);
    
    // Add advertising data fields 
    // Flags field 
    uint8_t flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    nimble_autoadv_add_field(BLE_HS_ADV_TYPE_FLAGS, &flags, sizeof(flags));
    
    // Service UUID 
    uint16_t service_uuid = SENSOR_CHAR_UUID;
    nimble_autoadv_add_field(BLE_HS_ADV_TYPE_COMP_UUIDS16, &service_uuid, sizeof(service_uuid));
    
    // Device name in scan response
    const char* device_name = "RIOT-EnvSensor";
    nimble_autoadv_add_field(BLE_HS_ADV_TYPE_COMP_NAME, device_name, strlen(device_name));

    // Start advertising using nimble_autoadv
    nimble_autoadv_start(NULL);
    
    printf("Advertising Started");
    return 0;
}
