/*
 * ringbuffer.h
 *
 *  Created on: 02-Nov-2026
 *      Author: sakth
 */

#ifndef INC_RINGBUFFER_H_
#define INC_RINGBUFFER_H_

#include <stdint.h>

#define RING_BUFFER_SIZE 10

typedef struct{
	uint8_t buffer[RING_BUFFER_SIZE];
	volatile uint16_t head;
	volatile uint16_t tail;
}RingBuffer;

uint8_t RingBuffer_Write(RingBuffer *rb,uint8_t data);
uint8_t RingBuffer_Read_Safe(RingBuffer *rb,uint8_t *data);
uint8_t RingBuffer_Available(RingBuffer *rf);
uint8_t RingBuffer_Count(RingBuffer *rb);

#endif /* INC_RINGBUFFER_H_ */
