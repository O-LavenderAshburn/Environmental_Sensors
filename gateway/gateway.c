#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "thread.h"
#include "shell.h"
#include "ztimer.h"
#include "nimble_scanner.h"
#include "nimble_scanlist.h"
#include "application.h"
#include "ble_handler.h"
#include "gateway.h"
#include "evaluation.h"
// default scan interval 


void check_scan_timeout(void) {
    if (!scan_timeout_active || scan_start_time == 0) return;
    uint32_t elapsed = ztimer_now(ZTIMER_MSEC) - scan_start_time;
    if (elapsed >= SCAN_TIMEOUT_MS) {
        printf("[TIMEOUT] Scan timeout after %lu ms - no %s sensor found\n",
               elapsed, target_type ? target_type : "unknown");

        nimble_scanner_stop();
        scan_timeout_active = false;

        if (active_response) {
            active_response->success = false;
            snprintf(active_response->error_message, sizeof(active_response->error_message),
                     "Scan timeout - no %s sensor found after %lu ms",
                     target_type ? target_type : "sensor", elapsed);
            print_sensor_response(active_response);
            free(active_response);
            active_response = NULL;
        }

        target_type = NULL;
        scan_start_time = 0;
    }
}
//**Query Sensor */
void ble_query_sensor(const int sensor_type ) {
    if (!active_response) {
        active_response = malloc(sizeof(sensor_response_t));
        memset(active_response, 0, sizeof(sensor_response_t));
    }

    generate_request_id(active_response->request_id, sizeof(active_response->request_id));

    switch (sensor_type) {
        case SENSOR_TEMP:
            strcpy(active_response->unit, "Celsius");
            target_type = "TEMP";
            break;

        case SENSOR_HUM:
            strcpy(active_response->unit, "Percent");
            target_type = "HUM";
            break;

        default:
            printf("[ERR] Unknown sensor type ID: %d\n", sensor_type);
            return;
    }

    scan_start_time = ztimer_now(ZTIMER_MSEC);
    scan_timeout_active = true;

    int rc = nimble_scanner_start();
    if (rc != 0) {
        printf("[ERROR] Failed to start scanner, rc: %d\n", rc);
        scan_timeout_active = false;
        active_response->success = false;
        snprintf(active_response->error_message, sizeof(active_response->error_message),
                 "Failed to start scanner: %d", rc);
        print_sensor_response(active_response);
        free(active_response);
        active_response = NULL;
        return;
    }

    printf("[INFO] Scanner started, looking for %s sensor...\n", target_type);
}

/**Shell commands */
int cmd_get_temp(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("Querying temperature sensor...\n");
    ble_query_sensor(SENSOR_TEMP);
    return 0;
}

int cmd_get_humid(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("Querying humidity sensor...\n");
    ble_query_sensor(SENSOR_HUM);
    return 0;
}

int cmd_help(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("BLE Sensor Gateway Application\n");
    printf("Available commands:\n");
    printf(" get_temp  - Query temperature sensor\n");
    printf(" get_humid - Query humidity sensor\n");
    printf(" help      - Show this help message\n");
    printf(" eval_temp  - Run temperature evaluation 100 times\n");
    printf(" eval_humid - Run humidity evaluation 100 times\n");

    return 0;
}

const shell_command_t shell_commands[] = {
    { "get_temp", "Query temperature sensor", cmd_get_temp },
    { "get_humid", "Query humidity sensor", cmd_get_humid },
    { "help", "Show help message", cmd_help },
    { "eval_temp", "Run temperature evaluation (100 runs)", cmd_eval_temp },
    { "eval_humid", "Run humidity evaluation (100 runs)", cmd_eval_humid },
    { NULL, NULL, NULL }
};

/**Timeout */
static void *timeout_thread(void *arg) {
    (void)arg;
    while (1) {
        check_scan_timeout();
        ztimer_sleep(ZTIMER_MSEC, 100);
    }
    return NULL;
}

//**Main function */
int main(void) {
    printf("=== BLE Sensor Gateway Application Started ===\n");
    printf("Available commands:\n");
    printf(" get_temp  - Query temperature sensor\n");
    printf(" get_humid - Query humidity sensor\n");
    printf(" help      - Show this help message\n");

    nimble_scanner_cfg_t params = {
        .itvl_ms = DEFAULT_SCAN_INTERVAL_MS,
        .win_ms = DEFAULT_SCAN_INTERVAL_MS,
        #if IS_USED(MODULE_NIMBLE_PHY_CODED)
                .flags = NIMBLE_SCANNER_PHY_1M | NIMBLE_SCANNER_PHY_CODED,
        #else
                .flags = NIMBLE_SCANNER_PHY_1M,
        #endif
    };


    int rc = nimble_scanner_init(&params, scan_cb);
    if (rc != 0) {
        printf("[ERROR] Failed to initialize scanner, rc: %d\n", rc);
        return 1;
    }


    static char timeout_stack[THREAD_STACKSIZE_SMALL];
        thread_create(timeout_stack, sizeof(timeout_stack),
                    THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
                    timeout_thread, NULL, "timeout_checker");

        char line_buf[SHELL_DEFAULT_BUFSIZE];
        shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
        return 0;
}