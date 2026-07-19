/*
 * Algorithm:
 * 1. Read temperature from sequence every second
 * 2. Check for fault (temp < -40 or > 150) — skip if invalid
 * 3. Check absolute alarm (temp >= MAX_SAFE_TEMP)
 * 4. Calculate rate of change from previous reading
 * 5. Check rate alarm (rate >= MAX_RATE_PER_SEC)
 * 6. Output task reads alarm states every second
 * 7. Sets GPIO HIGH/LOW and prints based on alarm combination
 */

 #include<stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define MAX_SAFE_TEMP 60.0f
#define MAX_RATE_PER_SEC 5.0f
#define TEMP_ALARM_PIN GPIO_NUM_4
#define RATE_ALARM_PIN GPIO_NUM_5

SemaphoreHandle_t xPrintMutex;
volatile int index=0;
volatile bool absoluteAlarm=false,rateAlarm=false;

void vReadValue(void *parameter){
    float tempSeq[]={36.0, 37.5, 39.0, 42.0, 48.0, 55.0, 62.0, 68.0, 57.0, 45.0};
    float previousTemp=0.0f;
    while(1){
        float temp=tempSeq[index];
        float rate=temp-previousTemp;
        if(temp<-40.0 || temp>150.0){
            xSemaphoreTake(xPrintMutex,portMAX_DELAY);
            printf("[Fault] Invalid rading\n");
            xSemaphoreGive(xPrintMutex);
            index++;
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        else if(temp>=60.0){
            taskENTER_CRITICAL();
            absoluteAlarm=true;
            taskEXIT_CRITICAL();
        }
        else if(temp<60){
            taskENTER_CRITICAL();
            absoluteAlarm=false;
            taskEXIT_CRITICAL();
        }
        if(rate>=5.0){
            taskENTER_CRITICAL();
            rateAlarm=true;
            taskEXIT_CRITICAL();
        }
        else if(rate<5.0){
            taskENTER_CRITICAL();
            rateAlarm=false;
            taskEXIT_CRITICAL();
        }
        index++;
	previousTemp = temp
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void vOutput(void *parameter){
    while(1){
        if(absoluteAlarm==true && rateAlarm==true){
            gpio_set_level(TEMP_ALARM_PIN,1);
            gpio_set_level(RATE_ALARM_PIN,1);
           if(xSemaphoreTake(xPrintMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
  	   printf("CRITICAL: THERMAL RUNAWAY DETECTED\n");
    	   xSemaphoreGive(xPrintMutex);
}        }
        else if(absoluteAlarm==true){
            gpio_set_level(TEMP_ALARM_PIN,1);
        }
        else if(rateAlarm==true){
            gpio_set_level(RATE_ALARM_PIN,1);
        }
        else if(absoluteAlarm==false && rateAlarm==false){
            gpio_set_level(TEMP_ALARM_PIN,0);
            gpio_set_level(RATE_ALARM_PIN,0);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
int main(void){
    gpio_set_direction(GPIO_NUM_4,GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_5,GPIO_MODE_OUTPUT);
    xPrintMutex=xSemaphoreCreateMutex();
    if(xPrintMutex==NULL){
        printf("[FATAL] Mutex failed\n");
        while(1);
    }
    BaseType_t ReadTask=xTaskCreate(vReadValue,"Read value",1024,NULL,4,NULL);
    if(ReadTask!=pdPASS){
        printf("[FATAL] Read task failed\n");
        while(1);
    }
    BaseType_t OutputTask=xTaskCreate(vOutput,"oUTPUT",1024,NULL,3,NULL);
    if(OutputTask!=pdPASS){
        printf("[FATAL] Output task failed\n");
        while(1);
    }
    vTaskStartScheduler();
}
