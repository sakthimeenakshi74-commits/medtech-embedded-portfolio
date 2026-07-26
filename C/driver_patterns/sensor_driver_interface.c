#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

typedef struct{
    const char *name;
    const char *unit;
    bool (*init) (void);
    int (*read) (void);
    bool (*isReady) (void);
    void (*selfTest) (void);
}SensorDriver;

bool tempInit(void){
    printf("Temperature Sensor initialized\n");
    return true;
}

int tempread(void){
    return 35+rand()%2;
}

bool tempIsReady(void){
    return true;
}
 void tempSelfTest(void){
     printf("Self test passed for temperature Sensor\n");
 }
 
 bool SpO2Init(void){
    printf("Spo2 Sensor initialized\n");
    return true;
}

int SpO2read(void){
    return 95+rand()%5;
}

bool SpO2IsReady(void){
    return false;
}
 void Spo2SelfTest(void){
     printf("Self test failed for SpO2 Sensor\n");
 }
 
 void runSensor(SensorDriver *driver){
     /* 
      * =======================================================================
      * ALGORITHM: Generic Sensor Execution Pipeline
      * =======================================================================
      * Step 1: Invoke hardware initialization sequence via the driver->init pointer.
      *         If initialization fails (returns false), abort and exit immediately.
      * Step 2: Execute the device-specific hardware diagnostic routine via ->selfTest.
      * Step 3: Poll the operational readiness status of the driver via ->isReady.
      *         Log whether the hardware is ready or unready based on the boolean result.
      * Step 4: Extract the current data measurement from the transceiver register using ->read.
      * Step 5: Format and stream the finalized telemetry string to the system console,
      *         dynamically applying the driver's unique identity string and measurement units.
      * =======================================================================
      */
     if(driver->init()!=true){
         printf("%s INIT Failed\n",driver->name);
         return;
     }
     driver->selfTest();
     if(driver->isReady()==true){
         printf("%s Sensor Ready\n",driver->name);
     }
     else{
         printf("%s Sensor Not Ready\n",driver->name);
     }
     int value=driver->read();
     printf("[%s] Reading: %d %s\n", driver->name, value, driver->unit);
}
     
int main(){
    SensorDriver tempDriver={
        .name="Temperature",
        .unit="°C",
        .init=tempInit,
        .read=tempread,
        .isReady=tempIsReady,
        .selfTest=tempSelfTest
    };
    SensorDriver SpO2Driver={
        .name="SpO2",
        .unit="%",
        .init=SpO2Init,
        .read=SpO2read,
        .isReady=SpO2IsReady,
        .selfTest=Spo2SelfTest
    };
    SensorDriver Driver[]={tempDriver,SpO2Driver};
    size_t Size=sizeof(Driver)/sizeof(Driver[0]);
    for(int i=0;i<Size;i++){
        runSensor(&Driver[i]);
    }
}
