/*
 * main.h
 *
 * Created: 05.12.2023 16:46:05
 *  Author: mikaj
 */ 


#ifndef MAIN_H_
#define MAIN_H_
#include "semphr.h"

extern SemaphoreHandle_t CountingSemaphore;

extern uint16_t ringbuffer[256];
extern uint16_t * p_Writing;
extern uint16_t * p_Reading;
extern uint8_t j;
extern uint16_t Ringbuffer_Pos;

extern uint8_t data;


#endif /* MAIN_H_ */