/*
 * QAMDecGen.c
 *
 * Created: 20.03.2018 18:32:07
 * Author : Martin Burger
 */ 

//#include <avr/io.h>
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
#include "stack_macros.h"

#include "mem_check.h"

#include "init.h"
#include "utils.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"

#include "qaminit.h"
#include "qamgen.h"
#include "qamdec.h"
#include "main.h"
#include "vAnalyze.h"

extern void vApplicationIdleHook( void );
void vLedBlink(void *pvParameters);

TaskHandle_t ledTask;

void vApplicationIdleHook( void )
{	
	
}
 /************************************************************************/
 /*Jeweils die nicht ben�tigten Elemente auskommentieren vor dem Upload!!*/
 /************************************************************************/
int main(void)
{
	resetReason_t reason = getResetReason();

	vInitClock();
	vInitDisplay();
	
	initDAC();				//Wird f�r den Sender Gebraucht!
	initDACTimer();			//Wird f�r den Sender Gebraucht!
	initGenDMA();			//Wird f�r den Sender Gebraucht!
	initADC();				//Wird f�r den Empf�nger Gebraucht!
	initADCTimer();			//Wird f�r den Empf�nger Gebraucht!
	initDecDMA();			//Wird f�r den Empf�nger Gebraucht!
	
	LSM9DS1Init();
	
	xTaskCreate(vQuamGen, NULL, configMINIMAL_STACK_SIZE+500, NULL, 2, NULL);			//Wird f�r den Sender Gebraucht!
 	xTaskCreate(vQuamDec, NULL, configMINIMAL_STACK_SIZE+400, NULL, 2, NULL);			//Wird f�r den Empf�nger Gebraucht!
// 	xTaskCreate(vAnalyze, NULL, configMINIMAL_STACK_SIZE+400, NULL, 1, NULL);			//Wird f�r den Empf�nger Gebraucht!
//	xTaskCreate(vDisplay, NULL, configMINIMAL_STACK_SIZE+100, NULL, 3, NULL);			//Wird f�r den Empf�nger Gebraucht!

	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"FreeRTOS 10.0.1");
	vDisplayWriteStringAtPos(1,0,"EDUBoard 1.0");
	vDisplayWriteStringAtPos(2,0,"QAMDECGEN-Base");
	vDisplayWriteStringAtPos(3,0,"ResetReason: %d", reason);
	vTaskStartScheduler();
	return 0;
}
