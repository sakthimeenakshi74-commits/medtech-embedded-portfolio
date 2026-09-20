/*******************************************************************************
 * @file        main.c
 * @brief       Embedded SpO2 Circular Buffer Simulator for MedTech Applications
 * 
 * DESIGN FEATURES:
 *  - Zero Dynamic Allocation: Heap-free design to prevent fragmentation (MISRA compliant).
 *  - Strict Parameter Scoping: Read-only validations explicitly marked 'const'.
 *  - Overrun Protection: Safe, deterministic oldest-value overwrite mechanism.
******************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define BUFFER_SIZE 8 


/*******************************************************************************
 * ALGORITHM DESCRIPTION: Medical SpO2 Photodiode Buffer Simulation
 * 
 * 1. Struct-based FIFO circular queue tracking head, tail, and item count.
 * 2. Producer enqueues 12 consecutive raw photodiode readings into the buffer.
 * 3. Overwrite Mode: If a write is requested when count == BUFFER_SIZE, the oldest 
 *    sample is dropped by forcing the tail forward before updating the head.
 * 4. Threshold Consumer Burst: When the count reaches or exceeds the half-full 
 *    mark (BUFFER_SIZE / 2), the consumer triggers a loop to completely clear 
 *    and convert all buffered items to an SpO2 percentage.
 ******************************************************************************/

typedef struct{
    uint16_t data[BUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
}CircularBuffer_t;

void init(CircularBuffer_t *cb){
    cb->head=0;
    cb->tail=0;
    cb->count=0;
}

bool isFull(const CircularBuffer_t *cb){
    return cb->count==BUFFER_SIZE ;
    
}

bool isEmpty(const CircularBuffer_t *cb){
    return cb->count==0;
}

void status(const CircularBuffer_t *cb){
    printf("Buffer: Count= %u Head=%u Tail=%u \n",cb->count,cb->head,cb->tail);
}

void enqueueOverWrite(CircularBuffer_t *cb,uint16_t value){
    cb->tail=(cb->tail+1)%BUFFER_SIZE;
    cb->count--;
    printf("Old Value Discarded\n");
    cb->data[cb->head]=value;
    cb->head=(cb->head+1)%BUFFER_SIZE;
    cb->count++;
    printf("Enqueued Data: %u",value);
    status(cb);
}

void enqueue(CircularBuffer_t *cb,uint16_t value){
    if(isFull(cb)){
        enqueueOverWrite(cb,value);
    }
    else{
        cb->data[cb->head]=value;
        cb->head=(cb->head+1)%BUFFER_SIZE;
        cb->count++;
        printf("Enqueued Data: %u",value);
        status(cb);
    }
}

void SpO2Process(uint16_t value){
    float Spo2=(value/4095.0f)*100.0f;
    printf("Processing Spo2: %0.1f % \n",Spo2);
}

void dequeue(CircularBuffer_t *cb,uint16_t *value){
    if(isEmpty(cb)){
        printf("The buffer is empty\n");
    }
    else{
        *value=cb->data[cb->tail];
        cb->tail=(cb->tail+1)%BUFFER_SIZE;
        cb->count--;
        SpO2Process(*value);
    }
}

int main()
{
    CircularBuffer_t cb;
    init(&cb);
    uint16_t reading;
    const uint16_t Samples[]={2048, 2100, 2200, 2350, 2500, 2600,
                       2700, 2750, 2800, 2780, 2750, 2700};
    for(int i = 0; i < 12; i++){
        enqueue(&cb,Samples[i]);
        if(cb.count>=BUFFER_SIZE/2){
            while(!isEmpty(&cb)){
                dequeue(&cb,&reading);
            }
        }
    }
}

