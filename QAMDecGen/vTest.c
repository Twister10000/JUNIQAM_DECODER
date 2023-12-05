/*
 * vTest.c
 *
 * Created: 05.12.2023 16:47:15
 *  Author: mikaj
 */ 

#include "avr_compiler.h"
#include "pmic_driver.h"
#include "TC_driver.h"
#include "clksys_driver.h"
#include "sleepConfig.h"
#include "port_driver.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "semphr.h"
#include "stack_macros.h"

#include "mem_check.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"

#include "qaminit.h"
#include "qamdec.h"
#include "string.h"
#include "main.h"

extern uint16_t ringbuffer[256];
extern uint16_t * p_Writing;
extern uint16_t * p_Reading;

void vTest(void *pvParameters){
	
	(void) pvParameters;
	
	for (;;)
	{
		vDisplayWriteStringAtPos(1,0,"Value Wr %d", *p_Writing);
		vDisplayWriteStringAtPos(2,0,"Value RR %d", *p_Reading);
		
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
	
}