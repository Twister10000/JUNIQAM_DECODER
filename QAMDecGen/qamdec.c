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
/*Defines*/
#define quarterjump1 7
#define quarterjump2 8
#define quarterjump3 9

#define halfjump1 15
#define halfjump2 16
#define halfjump3 17

#define threequartersjump1 23
#define threequartersjump2 24
#define threequartersjump3 25

#define fulljump1 31
#define fulljump2 32
#define fulljump3 33

#define onequarterjump1 39
#define onequarterjump2 40
#define onequarterjump3 41

#define onehalfjump1 47
#define onehalfjump2 48
#define onehalfjump3 49

#define onethreequartersjump1 55
#define onethreequartersjump2 56
#define onethreequartersjump3 57


/*Queue Init*/
QueueHandle_t decoderQueue;

/*Array Init*/
uint16_t ringbuffer[256];
uint8_t receivebuffer[100];
uint8_t maxpos[2];

/*Var Init*/
uint16_t max = 0;
uint8_t Mode = 0;
uint8_t Offset = 0;
uint8_t Ringbuffer_Pos = 0;
int j = 0;

/*Pointer Init*/
uint16_t * p_Writing = &ringbuffer[0];
uint16_t * p_Reading = &ringbuffer[0];
uint8_t * p_MAXPOS1r = &maxpos[0];
uint8_t * p_MAXPOS2r = &maxpos[1];

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
				switch(Mode)
				{
					case 0:
						if (((p_Writing - p_Reading)%64) == 0)
						{
							for (int i = 0; i < 32; i++)
							{
								
								if ((*p_Reading > *(p_Reading-1)) && *p_Reading > 1300) //Werte müssen angepasst werden mit den Werten von Merlin
								{
									max = *p_Reading;
									*p_MAXPOS1r = j;
								}
								p_Reading++;
								j++;
							}
							for (int i = 0; i < 32; i++)
							{
								if ((*p_Reading > *(p_Reading-1)) && *p_Reading > 1300) //Werte müssen angepasst werden mit den Werten von Merlin
								{
									max = *p_Reading;
									*p_MAXPOS2r = j;
								}
								p_Reading++;
								j++;
							}
						}
						Offset = *p_MAXPOS2r - *p_MAXPOS1r;
						*p_MAXPOS1r = *p_MAXPOS2r;
						Mode = 1;
						break;
						
					default:
					break;
				}
			
			if (((p_Writing - p_Reading)%32) == 0)
			{							
				for (int i = 0; i < 32; i++)
				{
					if ((*p_Reading > *(p_Reading-1)) && *p_Reading > 1300) //Werte müssen angepasst werden mit den Werten von Merlin
					{
						max = *p_Reading;
						*p_MAXPOS2r = j;
					}
					p_Reading++;
					j++;
				}
				Offset = *p_MAXPOS2r - *p_MAXPOS1r;
				*p_MAXPOS1r = *p_MAXPOS2r;
				p_Reading = p_Writing;
			}
				
			
			if (Ringbuffer_Pos%255 == 0)
			{
				Ringbuffer_Pos = 0;
				p_Writing = &ringbuffer[0];
			}
			
			}
		}
		//Decode Buffer
		//Search for Peak Position in Array
		//Switch Statement for decode Array Pos to bin		
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