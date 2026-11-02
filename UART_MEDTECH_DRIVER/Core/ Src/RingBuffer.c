/*
 * ringbuffer.c
 *
 *  Created on: 02-Nov-2026
 *      Author: sakth
 */

#ifndef SRC_RINGBUFFER_C_
#define SRC_RINGBUFFER_C_

#include "ringbuffer.h"
#include "main.h"

uint8_t RingBuffer_Write(RingBuffer *rb,uint8_t data){
	uint8_t nextTail=(rb->tail+1)%RING_BUFFER_SIZE;
	if(nextTail==rb->head){
		return 0;
	}
	rb->buffer[rb->tail]=data;
	rb->tail=nextTail;
	return 1;
}

uint8_t RingBuffer_Read_Safe(RingBuffer *rb,uint8_t *data){
	__disable_irq();
	if(rb->tail==rb->head){
		__enable_irq();
		return 0;
	}
	*data=rb->buffer[rb->head];
	rb->head=(rb->head+1)%RING_BUFFER_SIZE;
	__enable_irq();
	return 1;
}

uint8_t RingBuffer_Available(RingBuffer *rb){
	return (rb->head != rb->tail);
}

uint8_t RingBuffer_Count(RingBuffer *rb){
	return (rb->tail - rb-> head +RING_BUFFER_SIZE)%RING_BUFFER_SIZE;
}
#endif /* SRC_RINGBUFFER_C_ */
