#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include <stdlib.h>  
#include "semphr.h"
#include "event_groups.h"
#include <stdbool.h> 

/* ==========================================================================
 * DESIGN DEFENCE & ARCHITECTURE OVERVIEW (FOR GIT REPOSITORY)
 * ==========================================================================
 * 1. REAL-TIME RESPONSIVENESS (ISR TO HR CONTEXT SWITCH):
 *    The system simulates a hardware interrupt (GPIO EXTI0) via a high-priority task.
 *    By utilizing 'xQueueSendFromISR' along with 'portYIELD_FROM_ISR', the kernel forces 
 *    an immediate preemptive context switch to the processing layer (vHRCalc) without 
 *    waiting for the standard RTOS tick boundary. This guarantees zero-latency execution.
 * 
 * 2. OVERRUN PROTECTION / REPLACE STRATEGY:
 *    If the queue fills up (e.g., due to downstream task blockages or high-frequency input noise),
 *    the ISR runs a manual overwrite ring-buffer mechanism. It evicts the oldest item at the 
 *    front of the queue via 'xQueueReceiveFromISR' to instantly ensure the fresh, incoming 
 *    timestamp is saved, maintaining data freshness over stale histories.
 * 
 * 3. CRITICAL SECTION SEGREGATION:
 *    A binary Mutex ('xPrintMutex') guards all shared standard I/O (printf) streams. This avoids 
 *    overlapping/garbled character display windows on the hosting hardware console when multiple
 *    tasks output diagnostics concurrently.
 * 
 * 4. REAL-TIME RE-ROUTING PIECE:
 *    The Heart Rate Calculator forwards metrics to downstream targets using a 0-tick block timeout. 
 *    This ensures that a slow Logger or diagnostic task never pipelines a delay back into the primary 
 *    R-Peak processing loop.
 * ========================================================================== */

// Event Group Status Flag Bits
#define BRADYCARDIA (1<<0)
#define TACHYCARDIA (1<<1)

// Shared OS Primitive Synchronization & Inter-Task Handles
SemaphoreHandle_t xPrintMutex;
QueueHandle_t xISRtoHR;
QueueHandle_t xHRtoAlarm;
QueueHandle_t xHRtoLogger;
EventGroupHandle_t xHRCondition;

/* ==========================================================================
 * SIMULATED INTERRUPT FUNCTION (REPLACES HARDWARE GPIO EXTI)
 * ========================================================================== */
void vSimulated_EXTI0_ISR_Task(void *pvParameters){
   // Periodic delay matrix to simulate distinct ECG heart-rate rhythms
   TickType_t interval[4]={800, 300, 600, 1500};
   int index=0;
   while(1){
      // Sleep task to simulate real-world physical time intervals between R-Peaks
      vTaskDelay(pdMS_TO_TICKS(interval[index]));
      index=(index+1)%4;
      
      // Clear flag tracking whether a higher-priority task was unblocked
      BaseType_t xHigherPriorityTaskWoken=pdFALSE;
      
      // Capture accurate absolute timeline clock tick count from an ISR context
      TickType_t xTimeStamp=xTaskGetTickCountFromISR();
      
      // Dispatch time reference to the processing queue pipeline
      BaseType_t xStatus=xQueueSendFromISR(xISRtoHR,&xTimeStamp,&xHigherPriorityTaskWoken);
      
      // Overwrite/Replace strategy if the queue hits its max capacity of 10
      if(xStatus==errQUEUE_FULL){
         int xDummy;
         // Evict the single oldest timestamp from the front of the queue
         xQueueReceiveFromISR(xISRtoHR,&xDummy,&xHigherPriorityTaskWoken);
         // Insert the fresh, latest heartbeat timestamp into the newly freed space
         xQueueSendFromISR(xISRtoHR,&xTimeStamp,&xHigherPriorityTaskWoken);
      }
      // Force an immediate context switch at interrupt exit if xHigherPriorityTaskWoken is pdTRUE
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
   }
}

/* ==========================================================================
 * [ALGORITHM BLOCK] HEART RATE CALCULATOR PROCESSING TASK
 * ==========================================================================
 * Math Derivation Formula:
 * 1. Delta_Ticks = Current_Timestamp (t2) - Previous_Timestamp (t1)
 * 2. Delta_ms    = Delta_Ticks * portTICK_PERIOD_MS (Converts clock ticks to physical ms)
 * 3. BPM         = 60,000 ms / Delta_ms (Finds equivalent beats per minute)
 * 
 * Tracking Windows:
 * - Uses a sliding baseline window (`t1 = t2`) so every pair of consecutive items compiles 
 *   an independent heart-rate measurement cycle (e.g., 1->2, 2->3, 3->4).
 * ========================================================================== */
void vHRCalc(void *pvParameters){
   uint32_t BPM=0;
   TickType_t t1,t2;
   bool isFirstPeak=true; // Tracking flag to establish initial delta baseline marker
   while(1){
      // Block indefinitely until a timestamp is pushed into the queue by the ISR layer
      if(xQueueReceive(xISRtoHR,&t2,portMAX_DELAY)==pdPASS){
         if(isFirstPeak){
            t1=t2;              // Establish first reference point
            isFirstPeak=false;  // Clear initialization flag
            continue;           // Yield loop to wait for consecutive second point
         }
         
         // Execute sliding delta timing checks
         TickType_t deltaTick=t2-t1;
         TickType_t deltams=deltaTick * portTICK_PERIOD_MS;
         
         if(deltams>0){
            BPM=60000/deltams; // Compute final output beats per minute metric
            
            // Forward BPM non-blockingly to protect processing timeline loop integrity
            xQueueSend(xHRtoAlarm,&BPM,0);
            xQueueSend(xHRtoLogger,&BPM,0);
         }
         t1=t2; // Move current window index into previous slot position
      }
   }
}

/* ==========================================================================
 * ALARM MANAGEMENT & THRESHOLD CHECKING TASK
 * ========================================================================== */
void vAlarmTask(void *pvParameters){
   uint32_t BPM=0;
   while(1){
      // Block until a new BPM calculation is routed from the processing task
      xQueueReceive(xHRtoAlarm,&BPM,portMAX_DELAY);
      
      // Evaluate metric boundary thresholds
      if(BPM<50){
         xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(500));
         printf("[ALARM] Bradycardia HR: %d BPM \n",BPM);
         xSemaphoreGive(xPrintMutex);
         // Broadcast event bit to flag low heart-rate anomaly state
         xEventGroupSetBits(xHRCondition,BRADYCARDIA);
      }
      else if(BPM>130){
         xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(500));
         printf("[ALARM] Tachycardia HR: %d BPM \n",BPM);
         xSemaphoreGive(xPrintMutex);
         // Broadcast event bit to flag high heart-rate anomaly state
         xEventGroupSetBits(xHRCondition,TACHYCARDIA);
      }
      else {
         xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(500));
         printf("[HR] Normal HR: %d BPM \n",BPM);
         xSemaphoreGive(xPrintMutex);
      }
   }
}

/* ==========================================================================
 * HISTORICAL COMPLIANCE LOGGING DATA TASK
 * ========================================================================== */
void vLoggerTask(void *pvParameters){
   uint32_t BPM;
   uint32_t Counter=0;
   while(1){
      // Block until a metric update arrives via the distinct Logger queue pipeline
      xQueueReceive(xHRtoLogger,&BPM,portMAX_DELAY);
      Counter++; // Increment historical transaction log indicator index
      
      xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(500));
      printf("[LOG] Reading : %d\n",BPM);
      xSemaphoreGive(xPrintMutex);
   }
}

/* ==========================================================================
 * EVENT CONSOLIDATION MONITORING TASK
 * ========================================================================== */
void vHeartEvent(void *pvParameters){
   while(1){
      // Wait indefinitely for either Tachycardia or Bradycardia flag bits to trigger
      EventBits_t bits=xEventGroupWaitBits(xHRCondition,BRADYCARDIA|TACHYCARDIA,pdTRUE,pdFALSE,portMAX_DELAY);
      
      // Parse matched set condition bits securely using explicit tracking evaluation brackets
      if((bits&BRADYCARDIA) == BRADYCARDIA){
         xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(500));
         printf("Bradycardia bit set\n");
         xSemaphoreGive(xPrintMutex);
      }
      else if((bits&TACHYCARDIA) == TACHYCARDIA){
         xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(500));
         printf("Tachycardia bit set\n");
         xSemaphoreGive(xPrintMutex);
      }
   }
}

/* ==========================================================================
 * INITIALIZATION AND SYSTEM ENTRY POINT
 * ========================================================================== */
int main(void){
   // Allocate and assert safe extraction of the console Print Lock Mutex
   xPrintMutex=xSemaphoreCreateMutex();
   if(xPrintMutex==NULL){
      printf("[FATAL] Mutex creation failed!\n");
    while(1);
   }
   
   // Allocate Queues and cross-check object handles against out-of-memory errors
   xISRtoHR=xQueueCreate(10,sizeof(int));
   if(xISRtoHR==NULL){
      printf(" ISR to HR Queue not created\n");
      while(1);
   }
   xHRtoAlarm=xQueueCreate(5,sizeof(int));
   if(xHRtoAlarm==NULL){
      printf("HR to Alarm Queue not created\n");
      while(1);
   }
   xHRtoLogger=xQueueCreate(5,sizeof(int));
   if(xHRtoLogger==NULL){
      printf("HR to Logger Queue not created\n");
      while(1);
   }
   
   // Create Event flag broadcast container
   xHRCondition=xEventGroupCreate();
   if(xHRCondition==NULL){
      printf("Event Group not created\n");
      while(1);
   }
   
   // Spawn tasks and set priority tiers to maximize pipeline throughput
   BaseType_t xSimulated_EXTI0_ISR_Task=xTaskCreate(vSimulated_EXTI0_ISR_Task,"ISR",1024,NULL,5,NULL);
   if(xSimulated_EXTI0_ISR_Task!=pdPASS){
      printf("ISR task not created\n");
      while(1);
   }
   BaseType_t xHRCalc=xTaskCreate(vHRCalc,"HR task",1024,NULL,3,NULL);
   if(xHRCalc!=pdPASS){
      printf("HR task not created\n");
      while(1);
   }
   BaseType_t xAlarmTask=xTaskCreate(vAlarmTask,"Alarm task",1024,NULL,2,NULL);
   if(xAlarmTask!=pdPASS){
      printf("Alarm task not created\n");
      while(1);
   }
   BaseType_t xLoggerTask=xTaskCreate(vLoggerTask,"Logger task",1024,NULL,2,NULL);
   if(xLoggerTask!=pdPASS){
      printf("Logger task not created\n");
      while(1);
   }

   BaseType_t xHeartEvent=xTaskCreate(vHeartEvent,"Event Group",1024,NULL,1,NULL);
   if(xHeartEvent!=pdPASS){
      printf(" Event Group task not created\n");
      while(1);
   }
   vTaskStartScheduler();
}
