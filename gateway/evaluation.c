#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "thread.h"
#include "shell.h"
#include "ztimer.h"
#include "application.h"

// External declarations from your main gateway app
extern int cmd_get_temp(int argc, char **argv);
extern int cmd_get_humid(int argc, char **argv);
extern sensor_response_t *active_response;

#define SUCCESS_THRESHOLD_MS 100  // Success = discovery faster than 100ms

// Evaluation command for temperature
int cmd_eval_temp(int argc, char **argv) {
    int num_runs = 100;
    if (argc > 1) num_runs = atoi(argv[1]);
    
    uint32_t total_latency = 0;
    uint32_t min_latency = UINT32_MAX;
    uint32_t max_latency = 0;
    int successful_runs = 0;
    int fast_discoveries = 0;  // Discoveries under 100ms
    
    printf("Starting temperature evaluation: %d runs\n", num_runs);
    printf("Success threshold: < %d ms discovery latency\n", SUCCESS_THRESHOLD_MS);
    
    for (int i = 0; i < num_runs; i++) {
        printf("Run %d/%d - ", i + 1, num_runs);
        cmd_get_temp(0, NULL);
        
        // Wait a moment for the response to be populated
        ztimer_sleep(ZTIMER_MSEC, 100);
        
        // Check if we have a valid response with discovery latency
        if (active_response != NULL) {
            printf("Discovery latency: %" PRIu32 " ms", active_response->discovery_latency_ms);
            
            // Check if this is a "success" (discovery < 100ms)
            bool is_fast_discovery = (active_response->discovery_latency_ms < SUCCESS_THRESHOLD_MS);
            
            if (active_response->success) {
                successful_runs++;
                total_latency += active_response->discovery_latency_ms;
                
                if (active_response->discovery_latency_ms < min_latency) {
                    min_latency = active_response->discovery_latency_ms;
                }
                if (active_response->discovery_latency_ms > max_latency) {
                    max_latency = active_response->discovery_latency_ms;
                }
                
                // Count fast discoveries
                if (is_fast_discovery) {
                    fast_discoveries++;
                    printf(" - FAST SUCCESS: %.2f %s\n", active_response->value, active_response->unit);
                } else {
                    printf(" - SLOW SUCCESS: %.2f %s\n", active_response->value, active_response->unit);
                }
            } else {
                printf(" - FAILED: %s\n", active_response->error_message);
            }
        } else {
            printf(" - No response received\n");
        }
        
        ztimer_sleep(ZTIMER_MSEC, 1000); // Wait 1 second between runs
    }
    
    // Print statistics
    printf("Total runs: %d\n", num_runs);
    printf("Successful readings: %d\n", successful_runs);
    printf("Fast discoveries (<%d ms): %d\n", SUCCESS_THRESHOLD_MS, fast_discoveries);
    printf("Success rate (fast discovery): %.1f%%\n", (fast_discoveries * 100.0) / num_runs);
    
    if (successful_runs > 0) {
        printf("Average discovery latency: %.2f ms\n", (double)total_latency / successful_runs);
        printf("Minimum discovery latency: %" PRIu32 " ms\n", min_latency);
        printf("Maximum discovery latency: %" PRIu32 " ms\n", max_latency);
        
        // Additional latency distribution info
        printf("Readings under %d ms: %d (%.1f%% of successes)\n", 
               SUCCESS_THRESHOLD_MS, fast_discoveries, (fast_discoveries * 100.0) / successful_runs);
    }
    
    printf("Evaluation completed.\n");
    return 0;
}

// Evaluation command for humidity
int cmd_eval_humid(int argc, char **argv) {
    int num_runs = 100;
    if (argc > 1) num_runs = atoi(argv[1]);
    
    uint32_t total_latency = 0;
    uint32_t min_latency = UINT32_MAX;
    uint32_t max_latency = 0;
    int successful_runs = 0;
    int fast_discoveries = 0;  // Discoveries under 100ms
    
    printf("Starting humidity evaluation: %d runs\n", num_runs);
    printf("Success threshold: < %d ms discovery latency\n", SUCCESS_THRESHOLD_MS);
    
    for (int i = 0; i < num_runs; i++) {
        printf("Run %d/%d - ", i + 1, num_runs);
        cmd_get_humid(0, NULL);
        
        // Wait a moment for the response to be populated
        ztimer_sleep(ZTIMER_MSEC, 100);
        
        // Check if we have a valid response with discovery latency
        if (active_response != NULL) {
            printf("Discovery latency: %" PRIu32 " ms", active_response->discovery_latency_ms);
            
            // Check if this is a "success" (discovery < 100ms)
            bool is_fast_discovery = (active_response->discovery_latency_ms < SUCCESS_THRESHOLD_MS);
            
            if (active_response->success) {
                successful_runs++;
                total_latency += active_response->discovery_latency_ms;
                
                if (active_response->discovery_latency_ms < min_latency) {
                    min_latency = active_response->discovery_latency_ms;
                }
                if (active_response->discovery_latency_ms > max_latency) {
                    max_latency = active_response->discovery_latency_ms;
                }
                
                // Count fast discoveries
                if (is_fast_discovery) {
                    fast_discoveries++;
                    printf(" - FAST SUCCESS: %.2f %s\n", active_response->value, active_response->unit);
                } else {
                    printf(" - SLOW SUCCESS: %.2f %s\n", active_response->value, active_response->unit);
                }
            } else {
                printf(" - FAILED: %s\n", active_response->error_message);
            }
        } else {
            printf(" - No response received\n");
        }
        
        ztimer_sleep(ZTIMER_MSEC, 500); 
    }
    
    // Print statistics
    printf("Total runs: %d\n", num_runs);
    printf("Successful readings: %d\n", successful_runs);
    printf("Fast discoveries (<%d ms): %d\n", SUCCESS_THRESHOLD_MS, fast_discoveries);
    printf("Success rate (fast discovery): %.1f%%\n", (fast_discoveries * 100.0) / num_runs);
    
    if (successful_runs > 0) {
        printf("Average discovery latency: %.2f ms\n", (double)total_latency / successful_runs);
        printf("Minimum discovery latency: %" PRIu32 " ms\n", min_latency);
        printf("Maximum discovery latency: %" PRIu32 " ms\n", max_latency);
        
        // Additional latency distribution info
        printf("Readings under %d ms: %d (%.1f%% of successes)\n", 
               SUCCESS_THRESHOLD_MS, fast_discoveries, (fast_discoveries * 100.0) / successful_runs);
    }
    
    printf("Evaluation completed.\n");
    return 0;
}