#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include <stdlib.h>  
#include "semphr.h"
#include "event_groups.h"

// --- Event Group Bits for Abnormal Vitals ---
#define BIT_HR_ABNORMAL (1<<0)    // Bit 0: Heart Rate is out of safe range
#define BIT_SpO2_ABNORMAL (1<<1)  // Bit 1: SpO2 oxygen levels are dropping
#define BIT_BP_ABNORMAL (1<<2)    // Bit 2: Blood Pressure is abnormal
#define BIT_TEMP_ABNORMAL (1<<3)  // Bit 3: Temperature shows a fever

// --- Watchdog Software Status Bits ---
#define WATCHDOG_HR (1<<0)    // Bit flag to track if Heart Rate task is alive
#define WATCHDOG_SPO2 (1<<1)  // Bit flag to track if SpO2 task is alive
#define WATCHDOG_BP (1<<2)    // Bit flag to track if Blood Pressure task is alive
#define WATCHDOG_TEMP (1<<3)  // Bit flag to track if Temperature task is alive
// Combined bits: 00001111 (Shows all 4 sensor tasks checked in successfully)
#define WATCHDOG_ALL_BITS (WATCHDOG_HR|WATCHDOG_SPO2|WATCHDOG_BP|WATCHDOG_TEMP)

// --- Task Notification Flags for Display Task ---
#define NOTIFY_HR_ALERT    (1UL<<0)   // Notification flag for unusual Heart Rate
#define NOTIFY_SPO2_ALERT  (1UL << 1) // Notification flag for low SpO2
#define NOTIFY_BP_ALERT    (1UL << 2) // Notification flag for bad Blood Pressure
#define NOTIFY_TEMP_ALERT  (1UL << 3) // Notification flag for spiked Temperature

// --- Global Synchronization Variables ---
volatile uint32_t ulWatchdogKickRegister=0; // Share register for tracking task status
EventGroupHandle_t xVitalAlertGroup;        // Event group handle for critical thresholds
SemaphoreHandle_t xPrintMutex;              // Mutex to stop terminal text corruption
TaskHandle_t xAlertTaskHandle;              // Task handle for emergency alert handler
TaskHandle_t xDisplayTaskHandle;            // Task handle to pass alerts to screen display

// --- Custom Error Tracking Codes ---
typedef enum{
   ERR_NONE=0,
   ERR_QUEUE_FULL,
   ERR_MUTEX_TIMEOUT,
   ERR_INVALID_DATA,
   ERR_STACK_OVERFLOW,
   ERR_TASK_CREATE_FAILED,
   ERR_WATCHDOG_TIMEOUT
}ErrorCode_t;

// --- Data Structure for Storing Patient Readings ---
typedef struct{
   int heartRate;
   int SpO2;
   int Temp;
   int BP;
}PatientData_t;

// --- Function to Log Errors with Thread Safety ---
void vLogError(ErrorCode_t error,const char *taskName){
   // Take mutex to safely print to console without interruption
   xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(100));
   printf("[Error Log] Code: %d | Error : %s\n",error,taskName);
   xSemaphoreGive(xPrintMutex); // Relinquish mutex
}
// --- Simulating an Interrupt Service Routine (ISR) via a Task Loop ---
void vAlertISR(void *parameter){
   while(1){
      vTaskDelay(pdMS_TO_TICKS(30000)); // Triggers simulated button press every 30 secs
      BaseType_t xHighPriorityTaskWoken=pdFALSE;
      
      // Notify alert task immediately directly using FromISR mechanism
      vTaskNotifyGiveFromISR(xAlertTaskHandle,&xHighPriorityTaskWoken);
      portYIELD_FROM_ISR(xHighPriorityTaskWoken); // Force context switch if higher priority task wakes
      printf("[ISR] Emergency button pressed!\n");
   }
}

// --- Emergency Handler Task triggered by ISR ---
void vAlertTask(void *parameter){
   while(1){
      // Block indefinitely until notification arrives from the ISR
      ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
      printf("[EMERGENCY] Code Blue!\n");
   }
}

// --- Heart Rate Sensor Monitoring Task ---
void vHeartRate(void *parametr){
    PatientData_t PatientData;
    while(1){
        taskENTER_CRITICAL(); // Protect random generator parameters from context switches
        PatientData.heartRate=70+(rand()%50); // Generates simulated values between 70-119
        taskEXIT_CRITICAL();
        
        ulWatchdogKickRegister|=WATCHDOG_HR; // Signal to watchdog that Heart Rate task is running
        
        // Sanity check for sensor reading faults
        if(PatientData.heartRate<30 || PatientData.heartRate>200){
         vLogError(ERR_INVALID_DATA,"Invalid HR reading");
         vTaskDelay(pdMS_TO_TICKS(1000));
         continue; 
        }
        
        // Safely print current value to terminal 
        if(xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(100))!=pdTRUE){
         vLogError(ERR_MUTEX_TIMEOUT,"Mutex timeout");
        }
        else{
        printf("Heart Rate: %d\n",PatientData.heartRate);
        xSemaphoreGive(xPrintMutex);
        }
        
        // Medical threshold check: Warning if outside 60-100 bpm range
        if(PatientData.heartRate<60|| PatientData.heartRate>100){
         xTaskNotify(xDisplayTaskHandle,NOTIFY_HR_ALERT,eSetBits); // Send notification flag
         xEventGroupSetBits(xVitalAlertGroup,BIT_HR_ABNORMAL);    // Trip the central event bit
         
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // Refresh data twice per second
        }
    }

// --- SpO2 Oxygen Level Monitoring Task ---
void vSpO2(void *parametr){
   PatientData_t PatientData;
    while(1){
        taskENTER_CRITICAL();
        PatientData.SpO2=94+(rand()%6); // Generates simulated values between 94-99%
        taskEXIT_CRITICAL();
        
        ulWatchdogKickRegister|=WATCHDOG_SPO2; // Report alive status to watchdog register
        
        // Sanity boundary validation
        if(PatientData.SpO2<70 || PatientData.SpO2>100){
         vLogError(ERR_INVALID_DATA,"Invalid SpO2 reading");
         vTaskDelay(pdMS_TO_TICKS(1000));
         continue; 
        } 
        
        if(xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(100))!=pdTRUE){
         vLogError(ERR_MUTEX_TIMEOUT,"Mutex timeout");
        }
        else{
        printf("SpO2: %d\n",PatientData.SpO2);
        xSemaphoreGive(xPrintMutex);
        }
        
        // Critical validation: Drop under 96% requires clinical attention
        if(PatientData.SpO2<96){
          xTaskNotify(xDisplayTaskHandle,NOTIFY_SPO2_ALERT,eSetBits);
         xEventGroupSetBits(xVitalAlertGroup,BIT_SpO2_ABNORMAL);
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); // Refresh reading every 2 seconds
    }
 }

// --- Blood Pressure Monitoring Task ---
void vBP(void *parametr){
    PatientData_t PatientData;
    while(1){
        taskENTER_CRITICAL();
        PatientData.BP=80+(rand()%40); // Generates values between 80-119 mmHg
        taskEXIT_CRITICAL();
        
        ulWatchdogKickRegister|=WATCHDOG_BP; // Check in with watchdog register
        
        // Filter unexpected sensor glitches
        if(PatientData.BP<60 || PatientData.BP>180){
         vLogError(ERR_INVALID_DATA,"Invalid BP reading");
         vTaskDelay(pdMS_TO_TICKS(1000));
         continue; 
        }
        
        if(xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(100))!=pdTRUE){
         vLogError(ERR_MUTEX_TIMEOUT,"Mutex timeout");
        }
        else{
        printf("Blood Pressure: %d\n",PatientData.BP);
        xSemaphoreGive(xPrintMutex);
        }
        
        // Boundary check: Alert if systolic-equivalent dips below 80 or spikes above 120
        if(PatientData.BP<80 ||PatientData.BP>120){
          xTaskNotify(xDisplayTaskHandle,NOTIFY_BP_ALERT,eSetBits);
         xEventGroupSetBits(xVitalAlertGroup,BIT_BP_ABNORMAL);
        } 
        vTaskDelay(pdMS_TO_TICKS(1000)); // Check BP once per second
        }
    }

// --- Body Temperature Monitoring Task ---
void vTemperature(void *parametr){
    PatientData_t PatientData;
    while(1){
        taskENTER_CRITICAL();
        PatientData.Temp=96+(rand()%30)/10.0; // Generates decimal scale values from 96.0 to 98.9
        taskEXIT_CRITICAL();
        
        ulWatchdogKickRegister|=WATCHDOG_TEMP; // Set watchdog status bit for Temperature
        
        // Standard sensor filtering
        if(PatientData.Temp<90 || PatientData.Temp>110){
         vLogError(ERR_INVALID_DATA,"Invalid Temperature reading");
         vTaskDelay(pdMS_TO_TICKS(1000));
         continue; 
        }
        
        if(xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(100))!=pdTRUE){
         vLogError(ERR_MUTEX_TIMEOUT,"Mutex timeout");
        }
        else{
        printf("Temperature: %d\n",PatientData.Temp);
        xSemaphoreGive(xPrintMutex);
        }
        
        // Threshold check: Flag fever conditions over 100 degrees
        if(PatientData.Temp>100){
         xTaskNotify(xDisplayTaskHandle,NOTIFY_TEMP_ALERT,eSetBits);
         xEventGroupSetBits(xVitalAlertGroup,BIT_TEMP_ABNORMAL);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Read temperature once per second
        }
      }
// --- Centralized Critical Alert Processing Task via Event Groups ---
void vCriticalAlert(void *parameter){
   while(1){
      // Blocks until ANY of the listed vital status flags are set inside the event group
      EventBits_t bits=xEventGroupWaitBits(xVitalAlertGroup,BIT_HR_ABNORMAL|BIT_SpO2_ABNORMAL|BIT_BP_ABNORMAL|BIT_TEMP_ABNORMAL,pdTRUE,pdFALSE,portMAX_DELAY);
      
      // Decipher exactly which parameter failed to fire targeted outputs
      if(bits & BIT_HR_ABNORMAL){
         printf("[CRITICAL] Heart Rate abnormal!\n");
        }
      if(bits & BIT_SpO2_ABNORMAL){
         printf("[CRITICAL] SpO2 abnormal!\n");
         }
      if(bits & BIT_BP_ABNORMAL){
         printf("[CRITICAL] BP abnormal!\n");
      }
      if(bits & BIT_TEMP_ABNORMAL){
         printf("[CRITICAL] Temperature abnormal!\n");
      }
   }
}

// --- Safety Backup Failure Trap Handler ---
void vSafeShutdown(void){
   printf("[SYSTEM] Critical failure detected!\n");
   printf("[SYSTEM] *** ALERT — STAFF NEEDED ***\n");
   printf("[SYSTEM] Restarting system...\n");
   while(1); // Infinite loop to simulate device locking or fallback state
}

// --- Independent Watchdog Supervisor Task ---
void vWatchDogTask(void *parameter){
   while(1){
      vTaskDelay(pdMS_TO_TICKS(5000)); // Evaluates system safety loop every 5 seconds
      
      // Look for tasks that failed to check in inside the status register mask
      if(ulWatchdogKickRegister!=WATCHDOG_ALL_BITS){
         if(!(ulWatchdogKickRegister&WATCHDOG_HR)){
            vLogError(ERR_WATCHDOG_TIMEOUT,"[Watchdog] HR task not responding");
         }
         if(!(ulWatchdogKickRegister&WATCHDOG_SPO2)){
            vLogError(ERR_WATCHDOG_TIMEOUT,"[Watchdog] SpO2 task not responding");
         }
         if(!(ulWatchdogKickRegister&WATCHDOG_BP)){
            vLogError(ERR_WATCHDOG_TIMEOUT,"[Watchdog] BP task not responding");
         }
         if(!(ulWatchdogKickRegister&WATCHDOG_TEMP)){
            vLogError(ERR_WATCHDOG_TIMEOUT,"[Watchdog] TEMP task not responding");
         }
         vSafeShutdown(); // Trigger safety shutdown sequence if any module hung
      }
      else{
         // If all tasks are alive, print clearance report safely
         if(xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(100))!=pdTRUE){
         vLogError(ERR_MUTEX_TIMEOUT,"Mutex timeout");
        }
        else{
         printf("All tasks are alive\n");
         xSemaphoreGive(xPrintMutex);
        }
      }
      ulWatchdogKickRegister=0; // Zero out register to require fresh check-ins for next cycle
   }
}

// --- Interface/Display Updates Task ---
void vDisplayTask(void * parameter){
   uint32_t ulNotificationValue;
   while(1){
      // Blocks until a direct task notification arrives, saving bits inside variable
      xTaskNotifyWait(0,0xFFFFFFFF,&ulNotificationValue,portMAX_DELAY);
      
      // Display warnings based on active parsed notification flags
      if(ulNotificationValue&NOTIFY_HR_ALERT){
         printf("[Notify] Heart Rate needs attention\n");
      }
      if(ulNotificationValue&NOTIFY_SPO2_ALERT){
         printf("[Notify] SpO2 needs attention\n");
      }
      if(ulNotificationValue&NOTIFY_BP_ALERT){
         printf("[Notify] BP needs attention\n");
      }
      if(ulNotificationValue&NOTIFY_TEMP_ALERT){
         printf("[Notify] Temperature needs attention\n");
      }
   }
}

// --- Periodic System Health Diagnostics Task ---
void vHealthMonitor(void *parameter){
   char pcBuffer[2048]; // Array buffer allocation to hold real-time system stats
   while(1){
      vTaskDelay(10000); // Triggers report precisely every 10 seconds
      if(xSemaphoreTake(xPrintMutex,pdMS_TO_TICKS(100))==pdTRUE){
         printf("\n========= SYSTEM HEALTH REPORT =========\n");
         vTaskList(pcBuffer); // FreeRTOS system trace logging execution states
         printf("%s\n",pcBuffer);
         printf("Memory Diagnosis\n");
         // Prints actual remaining tracking allocation from standard FreeRTOS Heap
         printf("Global heap: %u bytes\n",(unsigned int)xPortGetFreeHeapSize());
         printf("=========================================\n\n");
         xSemaphoreGive(xPrintMutex);
      }
   }
}

// --- Main Application Setup Entry Point ---
int main(void){
    // Initialise printing mutex structure
    xPrintMutex=xSemaphoreCreateMutex();
    if(xPrintMutex == NULL) {
    printf("[FATAL] Mutex creation failed!\n");
    while(1);
   }
   
    // Initialise abnormal vital signals bit event group
    xVitalAlertGroup=xEventGroupCreate();
    if(xVitalAlertGroup == NULL) {
    printf("[FATAL] Event group creation failed!\n");
    while(1);
    }
    
    // --- Creating Sensor Processing Tasks ---
    BaseType_t HR=xTaskCreate(vHeartRate,"HEARTRATE",1024,NULL,4,NULL);
    if(HR!= pdPASS){
      vLogError(ERR_TASK_CREATE_FAILED ,"HR Task not created");
      while(1);
    }
    BaseType_t SpO2=xTaskCreate(vSpO2,"SpO2",1024,NULL,4,NULL);
    if(SpO2!=pdPASS){
      vLogError(ERR_TASK_CREATE_FAILED ,"SpO2 Task not created");
      while(1);
    }
    BaseType_t BP=xTaskCreate(vBP,"Blood pressure",1024,NULL,3,NULL);
    if(BP!=pdPASS){
      vLogError(ERR_TASK_CREATE_FAILED ,"BP Task not created");
      while(1);
    }
    BaseType_t Temp=xTaskCreate(vTemperature,"Temperature",1024,NULL,2,NULL);
    if(Temp!=pdPASS){
      vLogError(ERR_TASK_CREATE_FAILED ,"Temperature Task not created");  
      while(1);  
   }
   
    // --- Creating System Infrastructure and Alert Tracking Tasks ---
    BaseType_t AT=xTaskCreate(vCriticalAlert,"Critical Alert",1024,NULL,5,NULL);
    if(AT!=pdPASS){
      vLogError(ERR_TASK_CREATE_FAILED ,"Alert Task Task not created");
      while(1);
    }
    BaseType_t WatchDog=xTaskCreate(vWatchDogTask,"Watchdog",1024,NULL,6,NULL);
    if(WatchDog!=pdPASS){
      vLogError(ERR_TASK_CREATE_FAILED,"Watchdog not created");
      while(1);
    }
    BaseType_t Display=xTaskCreate(vDisplayTask,"Display  Task",1024,NULL,1,&xDisplayTaskHandle);
    if(Display!=pdPASS){
      vLogError(ERR_TASK_CREATE_FAILED,"Display Task not created");
      while(1);
    }
    BaseType_t AlertTask=xTaskCreate(vAlertTask,"Alert  Task",1024,NULL,4,&xAlertTaskHandle);
    if(AlertTask!=pdPASS){
      vLogError(ERR_TASK_CREATE_FAILED,"Alert Task not created");
      while(1);
    }
    BaseType_t AlertISR=xTaskCreate(vAlertISR,"Alert ISR",1024,NULL,2,NULL);
    if(AlertISR!=pdPASS){
      vLogError(ERR_TASK_CREATE_FAILED,"Alert ISR Task not created");
      while(1);
    }
    BaseType_t Healthmonitor=xTaskCreate(vHealthMonitor,"Health Monitor Task",1024,NULL,1,NULL);
    if(Healthmonitor!=pdPASS){
      vLogError(ERR_TASK_CREATE_FAILED,"Health monitor task not created");
      while(1);
    }
    
    // Hand over total device execution management to FreeRTOS Kernel Engine
    vTaskStartScheduler();
 }

