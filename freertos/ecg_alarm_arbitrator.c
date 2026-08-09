#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include <stdlib.h>  
#include "semphr.h"
#include "queue.h"

typedef struct{
   const char *module_name;
    const char *condition;
    uint8_t     severity;   // 1=warning, 2=critical, 3=emergency
} AlarmEvent_t;

SemaphoreHandle_t xPrintMutex;
QueueHandle_t xValueQueue;
uint8_t ucActiveAlarm[3]={0,0,0};

void vRateMonitor(void *parameter){
   TickType_t xlastWakeTime=xTaskGetTickCount();
   while(1){
      int SimulatedHR=40+(rand()%101);
      if(SimulatedHR<50 || SimulatedHR>130){
         AlarmEvent_t xEvent;
         xEvent.module_name="Rate Monitor";
         xEvent.condition=(SimulatedHR<50)?"Bradycardia Detected" :"Tachycardia Detected";
         xEvent.severity=1;
         xQueueSend(xValueQueue,&xEvent,0);
      }
      vTaskDelayUntil(&xlastWakeTime,pdMS_TO_TICKS(1000));
   }
}

void vRhythmAnalyser(void *parameter){
   TickType_t xlastWakeTime=xTaskGetTickCount();
   while(1){
      int SimulatedVariance=0+(rand()%41);
      if(SimulatedVariance>20 ){
         AlarmEvent_t xEvent;
         xEvent.module_name="Rhythm Analyser";
         xEvent.condition="Irregular Rhythm Detected";
         xEvent.severity=1;
         xQueueSend(xValueQueue,&xEvent,0);
      }
      vTaskDelayUntil(&xlastWakeTime,pdMS_TO_TICKS(1000));
   }
}

void vSTSegment(void *parameter){
   TickType_t xlastWakeTime=xTaskGetTickCount();
   while(1){
      float SimulatedST=((float)rand() / (float)(RAND_MAX)) * 4.0f;
      if(SimulatedST>2.0f ){
         AlarmEvent_t xEvent;
         xEvent.module_name="STSegment";
         xEvent.condition="ST Elevation (Possible MI";
         xEvent.severity=1;
         xQueueSend(xValueQueue,&xEvent,0);
      }
      vTaskDelayUntil(&xlastWakeTime,pdMS_TO_TICKS(1000));
   }
}

void vAlarmManager(void *parameter){
   TickType_t xWIndowStartTime=xTaskGetTickCount();
   const TickType_t xWindowDuration=pdMS_TO_TICKS(5000);
   AlarmEvent_t xReceivedAlarm;
   while(1){
      TickType_t xCurrentTime=xTaskGetTickCount();
      if(xCurrentTime-xWIndowStartTime>xWindowDuration){
         xSemaphoreTake(xPrintMutex,portMAX_DELAY);
         printf("--- [SYSTEM] 5s Window Expired. Clearing Alarm Count. ---\n");
         xSemaphoreGive(xPrintMutex);
         memset(ucActiveAlarm,0,sizeof(ucActiveAlarm));    
         xWIndowStartTime=xCurrentTime;  
      }
      TickType_t xElapsedTime=xCurrentTime-xWIndowStartTime;
      TickType_t xTicksToWait=(xElapsedTime<xWindowDuration)?(xWindowDuration-xElapsedTime):0;
      if(xQueueReceive(xValueQueue,&xReceivedAlarm,xTicksToWait)==pdTRUE){
         if(strcmp(xReceivedAlarm.module_name,"Rate Monitor")==0){
            ucActiveAlarm[0]=1;
         }
         else if(strcmp(xReceivedAlarm.module_name,"Rhythm Analyser")==0){
            ucActiveAlarm[1]=1;
         }
         else if(strcmp(xReceivedAlarm.module_name,"STSegment")==0){
            ucActiveAlarm[2]=1;
         }
      uint8_t ucTriggeredcount=ucActiveAlarm[0]+ucActiveAlarm[1]+ucActiveAlarm[2];
      xSemaphoreTake(xPrintMutex,portMAX_DELAY);
      printf("[ALERT] From: %s || Cond: %s\n",xReceivedAlarm.module_name,xReceivedAlarm.condition);
      printf(">>>> ARBITRATOR STATUS: ");
      switch (ucTriggeredcount){
         case 1:
            printf("Warning\n");
            break;
         case 2:
            printf("Critical\n");
            break;
         case 3:
            printf("Emergency-Code Blue\n");
            break;
         default:
            printf("Normal\n");
            break;
      }
      xSemaphoreGive(xPrintMutex);
      }
   }  
}
int main(void){
   xPrintMutex=xSemaphoreCreateMutex();
   if(xPrintMutex==NULL){
      printf("Print Mutex not created\n");
      while(1);
   }
   xValueQueue=xQueueCreate(10,sizeof(AlarmEvent_t));
   if(xValueQueue==NULL){
      printf("Queue not created\n");
      while(1);
   }
   BaseType_t Hr=xTaskCreate(vRateMonitor,"Rate Monotor",1024,NULL,2,NULL);
   if(Hr!=pdPASS){
      printf("Rate Monitor Task not created\n");
      while(1);
   }
   BaseType_t RA=xTaskCreate(vRhythmAnalyser,"Rhythm Analyser",1024,NULL,2,NULL);
   if(RA!=pdPASS){
      printf("Rhythm Analyser Task not created\n");
      while(1);
   }
   BaseType_t ST=xTaskCreate(vSTSegment,"ST Segment Analyser",1024,NULL,2,NULL);
   if(ST!=pdPASS){
      printf("RST Segment Analyser Task not created\n");
      while(1);
   }
   BaseType_t AlarmManager=xTaskCreate(vAlarmManager,"AlarmManager",1024,NULL,3,NULL);
   if(AlarmManager!=pdPASS){
      printf("Alarm Manager Task not created\n");
      while(1);
   }
   vTaskStartScheduler();
}
