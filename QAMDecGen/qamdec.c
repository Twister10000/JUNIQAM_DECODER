/*
* qamdec.c
*
* Created: 05.05.2020 16:38:25
*  Author: Chaos
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

#include "qaminit.h"
#include "qamdec.h"
#include "string.h"
#include "main.h"
/*Defines*/

/*Test Init*/


/*Queue Init*/
QueueHandle_t decoderQueue;

SemaphoreHandle_t CountingSemaphore;

/*Array Init*/
uint16_t ringbuffer[256]; //Array nicht Global erstellen


/*Var Init*/



/*Pointer Init*/
uint16_t * p_Writing = &ringbuffer[0];
uint16_t * p_Reading = &ringbuffer[0];
uint8_t data = 0;



void analyzediff();

void vQuamDec(void* pvParameters)
{
	( void ) pvParameters;
	decoderQueue = xQueueCreate( 4, NR_OF_SAMPLES * sizeof(int16_t) );
	
	while(evDMAState == NULL) {
		vTaskDelay(3/portTICK_RATE_MS);
	}
	
	uint16_t bufferelement[NR_OF_SAMPLES];
	
	xEventGroupWaitBits(evDMAState, DMADECREADY, false, true, portMAX_DELAY);
	for(;;) {
		while(uxQueueMessagesWaiting(decoderQueue) > 0) { // Nur arbeiten wenn in der Queue auch Werte drin sind
			if(xQueueReceive(decoderQueue, &bufferelement[0], portMAX_DELAY) == pdTRUE) {
				for (int i = 0; i < 32; i++) // Die Werte von der Queue werden in das Ringbuffer geschrieben
				{
					*p_Writing = bufferelement[i];
					p_Writing++;
					Ringbuffer_Pos++;
					//Differenz Wert
				}
				data++;
				/*xSemaphoreGive(TestSemaphore);*/
				if(Ringbuffer_Pos == 256) //% Operator | Bits maskieren!? Ringbuffer 0/256 == 0
					{
						Ringbuffer_Pos = 0;
						p_Writing = &ringbuffer[0]; 

					}
			} //Klammer While
		} //Klammer For
		vTaskDelay( 2 / portTICK_RATE_MS );
	}
}



void fillDecoderQueue(uint16_t buffer[NR_OF_SAMPLES])
{
	BaseType_t xTaskWokenByReceive = pdFALSE;

	xQueueSendFromISR( decoderQueue, &buffer[0], &xTaskWokenByReceive );
}

ISR(DMA_CH2_vect)
{
	DMA.CH2.CTRLB|=0x10;

	fillDecoderQueue( &adcBuffer0[0] );
}

ISR(DMA_CH3_vect)
{
	DMA.CH3.CTRLB |= 0x10;

	fillDecoderQueue( &adcBuffer1[0] );
}