#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include <stdlib.h>  
#include "semphr.h"
#include "event_groups.h"

#define BIT_FLASHMUTEX_TIMEOUT (1<<0)
#define BIT_MOTORMUTEX_TIMEOUT (1<<1)
#define BIT_DEADLOCK           (1<<2)
#define MUTEX_FAULT            (BIT_FLASHMUTEX_TIMEOUT|BIT_MOTORMUTEX_TIMEOUT)

#define WATCHDOG_INFUSION (1<<0)
#define WATCHDOG_TELEMETRY (1<<1)
#define WATCHDOG_ALL_BITS (WATCHDOG_INFUSION|WATCHDOG_TELEMETRY)

volatile uint32_t ulWatachDogKickRegister =0;
SemaphoreHandle_t xMotorMutex;
SemaphoreHandle_t xFlashMutex;
SemaphoreHandle_t xPrintMutex;
EventGroupHandle_t xAlertGroup;

void ShutDown(void){
   xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(100));
   printf("[SYSTEM] Entering Unrecoverable Freeze State. Motor Off.\n");
   xSemaphoreGive(xPrintMutex);
   exit(0);
}

void vInfusionTask (void *parameter){
   TickType_t xLastWakeTime=xTaskGetTickCount();
   while(1){
      if(xSemaphoreTake(xMotorMutex,pdMS_TO_TICKS(100))==pdTRUE){
         if(xSemaphoreTake(xFlashMutex,pdMS_TO_TICKS(50))==pdTRUE){
            ulWatachDogKickRegister|=WATCHDOG_INFUSION;
            xSemaphoreGive(xFlashMutex);
         }
         else{
            xSemaphoreGive(xMotorMutex);
            xEventGroupSetBits(xAlertGroup,BIT_FLASHMUTEX_TIMEOUT);
            continue;
         }
         xSemaphoreGive(xMotorMutex);
      }
      vTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(200));
   }
}

void vTelemetryTask(void *parameter){
   TickType_t xLastWakeTime=xTaskGetTickCount();
   while(1){
      if(xSemaphoreTake(xFlashMutex,pdMS_TO_TICKS(100))==pdTRUE){
         if(xSemaphoreTake(xMotorMutex,pdMS_TO_TICKS(50))==pdTRUE){
            ulWatachDogKickRegister|=WATCHDOG_TELEMETRY;
            xSemaphoreGive(xMotorMutex);
         }
         else{
            xSemaphoreGive(xFlashMutex);
            xEventGroupSetBits(xAlertGroup,BIT_MOTORMUTEX_TIMEOUT);
            continue;
         }
         xSemaphoreGive(xFlashMutex);
      }
      vTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(500));
   }
}

void vWatchDogTimer(void *parameter){
   while(1){
      vTaskDelay(pdMS_TO_TICKS(2000));
      if((ulWatachDogKickRegister&WATCHDOG_ALL_BITS) != WATCHDOG_ALL_BITS){
         xEventGroupSetBits(xAlertGroup,BIT_DEADLOCK);
      }
      else{
         xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(0));
         printf("[Watchdog Check-In]: All Infusion Pump tasks operating normally.\n");
         xSemaphoreGive(xPrintMutex);
         ulWatachDogKickRegister=0;
      }
   }
}

void vSafetyHandlerTask(void *parameter){
   while(1){
      EventBits_t bits=xEventGroupWaitBits(xAlertGroup,BIT_FLASHMUTEX_TIMEOUT|BIT_MOTORMUTEX_TIMEOUT|BIT_DEADLOCK,pdTRUE,pdFALSE,portMAX_DELAY);
      if(bits&BIT_DEADLOCK){
         xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(100));
         printf("[EMERGENCY] RTOS Deadlock Detected! Hard safety shutdown of Motor Driver. CALL MAINTENANCE.\n");
         xSemaphoreGive(xPrintMutex);
         ShutDown();
      }
      else if((bits&MUTEX_FAULT)==MUTEX_FAULT){
         xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(100));
         printf("Both Resource Not Available\n");
         xSemaphoreGive(xPrintMutex);
      }
      else if(bits&BIT_MOTORMUTEX_TIMEOUT){
         xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(100));
         printf("Motor Mutex Blocked\n");
         xSemaphoreGive(xPrintMutex);
      }
      else if(bits&BIT_FLASHMUTEX_TIMEOUT){
         xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(100));
         printf("Flash Mutex Blocked\n");
         xSemaphoreGive(xPrintMutex);
      }
   }
}

int main(void){
   xAlertGroup=xEventGroupCreate();
   if(xAlertGroup==NULL){
      printf("Event Group not created\n");
      while(1);
   }
   xPrintMutex=xSemaphoreCreateMutex();
   if(xPrintMutex==NULL){
      printf("Print mutex not created\n");
      while(1);
   }
   xMotorMutex=xSemaphoreCreateMutex();
   if(xMotorMutex==NULL){
      printf("Motor Mutex not created\n");
      while(1);
   }
   xFlashMutex=xSemaphoreCreateMutex();
   if(xFlashMutex==NULL){
      printf("Flash Mutex not created\n");
      while(1);
   }
   BaseType_t IT=xTaskCreate(vInfusionTask,"Infusion Task",1024,NULL,3,NULL);
   if(IT!=pdPASS){
      printf("Infusion Task not created\n");
      while(1);
   }
   BaseType_t TT=xTaskCreate(vTelemetryTask,"Telemetry Task",1024,NULL,2,NULL);
   if(TT!=pdPASS){
      printf("Telemetry Task not created\n");
      while(1);
   }
   BaseType_t WT=xTaskCreate(vWatchDogTimer,"WatchDog Timer",1024,NULL,4,NULL);
   if(WT!=pdPASS){
      printf("WatchDog Tmer Task not created\n");
      while(1);
   }
   BaseType_t SHT=xTaskCreate(vSafetyHandlerTask,"Safety Handler",1024,NULL,5,NULL);
   if(SHT!=pdPASS){
      printf("Safety Handler Task not created\n");
      while(1);
   }
   vTaskStartScheduler();
}
