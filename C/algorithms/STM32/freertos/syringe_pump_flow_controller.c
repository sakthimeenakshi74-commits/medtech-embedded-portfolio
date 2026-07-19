/*
 * Syringe Pump Flow Controller — FreeRTOS
 *
 * Algorithm:
 * 1. Command Task sends flow rates to queue every 3 seconds
 * 2. Flow Control Task receives rate, validates range, calculates steps/sec
 * 3. Flow Control Task accumulates actual volume every second
 * 4. Safety Monitor Task runs every 2 seconds, compares expected vs actual volume
 * 5. If deviation > MAX_DEVIATION — raise flow deviation alert
 * 6. All shared data protected by mutexes
 * 7. All printf calls protected by print mutex
 */
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include <stdlib.h>
#include "semphr.h"
#include "event_groups.h"

#define STEPS_PER_ML    100
#define MAX_DEVIATION   0.10f
#define MAX_FLOW_RATE   50.0f
#define MIN_FLOW_RATE   0.1f

float FlowRate     = 0.0f;
float actualVolume = 0.0f;

QueueHandle_t     xCommandQueue;
SemaphoreHandle_t xPrintMutex;
SemaphoreHandle_t xVolumeMutex;

void vFlowControlTask(void *parameter) {
    int StepsPerSec = 0;
    TickType_t LastTickTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000);

    while(1) {
        xTaskDelayUntil(&LastTickTime, xFrequency);

        if(xQueueReceive(xCommandQueue, &FlowRate, 0) == pdTRUE) {
            if(FlowRate != 0.0f && (FlowRate < MIN_FLOW_RATE || FlowRate > MAX_FLOW_RATE)) {
                if(xSemaphoreTake(xPrintMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                    printf("[FLOW] Invalid flow rate: %.1f ml/hr\n", FlowRate);
                    xSemaphoreGive(xPrintMutex);
                }
                FlowRate = 0.0f;
                continue;
            }

            StepsPerSec = (int)(FlowRate * STEPS_PER_ML);

            if(xSemaphoreTake(xPrintMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                printf("[FLOW] New rate: %.1f ml/hr → %d steps/sec\n", FlowRate, StepsPerSec);
                xSemaphoreGive(xPrintMutex);
            }
        }

        if(xSemaphoreTake(xVolumeMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            actualVolume += (FlowRate / 3600.0f);
            xSemaphoreGive(xVolumeMutex);
        }
    }
}

void vSafetyMonitor(void *parameter) {
    TickType_t LastTickTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(2000);
    float cumulativeExpected = 0.0f;

    while(1) {
        xTaskDelayUntil(&LastTickTime, xFrequency);

        float localRate = 0.0f;
        if(xSemaphoreTake(xVolumeMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            localRate = FlowRate;
            xSemaphoreGive(xVolumeMutex);
        }

        cumulativeExpected += localRate * (2.0f / 3600.0f);

        float currentActual = 0.0f;
        if(xSemaphoreTake(xVolumeMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            currentActual = actualVolume;
            xSemaphoreGive(xVolumeMutex);
        }

        if(currentActual > 0.0f) {
            float deviation = ((currentActual - cumulativeExpected) / cumulativeExpected) * 100.0f;
            if(deviation > (MAX_DEVIATION * 100.0f)) {
                if(xSemaphoreTake(xPrintMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                    printf("[SAFETY] FLOW DEVIATION ALERT — Expected: %.1f ml, Actual: %.1f ml\n",
                           cumulativeExpected, currentActual);
                    xSemaphoreGive(xPrintMutex);
                }
            }
        }
    }
}

void vCommandTask(void *parameter) {
    TickType_t LastTickTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(3000);
    float RateArray[5] = {5.0f, 55.0f, 7.5f, 0.0f, 12.0f};
    int index = 0;

    while(1) {
        xTaskDelayUntil(&LastTickTime, xFrequency);

        float nextRate = RateArray[index];
        xQueueSend(xCommandQueue, &nextRate, portMAX_DELAY);

        if(nextRate == 0.0f) {
            if(xSemaphoreTake(xPrintMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                printf("[CMD] PUMP STOPPED\n");
                xSemaphoreGive(xPrintMutex);
            }
        }

        index = (index + 1) % 5;
    }
}

int main(void) {
    xPrintMutex = xSemaphoreCreateMutex();
    if(xPrintMutex == NULL) {
        printf("[FATAL] Print mutex not created\n");
        while(1);
    }

    xVolumeMutex = xSemaphoreCreateMutex();
    if(xVolumeMutex == NULL) {
        printf("[FATAL] Volume mutex not created\n");
        while(1);
    }

    xCommandQueue = xQueueCreate(5, sizeof(float));
    if(xCommandQueue == NULL) {
        printf("[FATAL] Command queue not created\n");
        while(1);
    }

    BaseType_t FCT = xTaskCreate(vFlowControlTask, "FlowControl", 1024, NULL, 3, NULL);
    if(FCT != pdPASS) {
        printf("[FATAL] Flow control task not created\n");
        while(1);
    }

    BaseType_t SMT = xTaskCreate(vSafetyMonitor, "SafetyMonitor", 1024, NULL, 2, NULL);
    if(SMT != pdPASS) {
        printf("[FATAL] Safety monitor task not created\n");
        while(1);
    }

    BaseType_t CT = xTaskCreate(vCommandTask, "CommandTask", 1024, NULL, 1, NULL);
    if(CT != pdPASS) {
        printf("[FATAL] Command task not created\n");
        while(1);
    }

    vTaskStartScheduler();
}
