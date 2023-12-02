/*
* qamgen.c
*
* Created: 05.05.2020 16:24:59
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

#include "qaminit.h"
#include "qamgen.h"
uint8_t debug_gen = 0;
												
const int16_t Impuls1[NR_OF_SAMPLES] = {0x18F,0x30F,0x471,0x5A7, 0x5A7, 0x471, 0x30F, 0x18F, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
										0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,};
	
const int16_t Impuls2[NR_OF_SAMPLES] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x18F,0x30F,0x471,0x5A7, 0x5A7, 0x471, 0x30F, 0x18F, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,};
	
const int16_t Impuls3[NR_OF_SAMPLES] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
										 0x18F,0x30F,0x471,0x5A7, 0x5A7, 0x471, 0x30F, 0x18F, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
										 
const int16_t Impuls4[NR_OF_SAMPLES] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
										0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x18F,0x30F,0x471,0x5A7, 0x5A7, 0x471, 0x30F, 0x18F,};


#define SENDBUFFER_SIZE 31 //Working 29

uint8_t sendbuffer[100] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
uint8_t sendID = 0;
void createSendData() { /* Eine Beispiel funktion für die Erstellung der Sendedaten. */
	sendID++;
	
	char senddata[10] = "HALO";
	uint8_t datalen = strlen(senddata); 
	//MetaInfos werden übermittelt
	sendbuffer[0] = 0;
	sendbuffer[1] = 3;
	sendbuffer[2] = 0;
	sendbuffer[3] = 3;
	sendbuffer[4] = (sendID >> 0) & 0x03;
	sendbuffer[5] = (sendID >> 2) & 0x03;
	sendbuffer[6] = (sendID >> 4) & 0x03;
	sendbuffer[7] = (sendID >> 6) & 0x03;
	sendbuffer[8] = (datalen >> 0) & 0x03;
	sendbuffer[9] = (datalen >> 2) & 0x03;
	sendbuffer[10] = (datalen >> 4) & 0x03;
	sendbuffer[11] = (datalen >> 6) & 0x03;
	for(int i = 0; i < datalen;i++) { //Die Daten werden in zweier Paare nacheinander in das Sendbuffer eingeschrieben
		sendbuffer[12 + i*4 + 0] = (senddata[i] >> 0) & 0x03;
		sendbuffer[12 + i*4 + 1] = (senddata[i] >> 2) & 0x03;
		sendbuffer[12 + i*4 + 2] = (senddata[i] >> 4) & 0x03;
		sendbuffer[12 + i*4 + 3] = (senddata[i] >> 6) & 0x03;
	}
	uint8_t checksum = 0;
	for(int i = 0; i < 12 + (datalen * 4); i++) {
		checksum += sendbuffer[i]; // Alle Elemente von Sendbuffer werden zusammengerrechnet.
	}
	sendbuffer[12 + (datalen * 4) + 0] = 0;  //Die Checksume wird auf 2bit Paare aufgeteilt
	sendbuffer[12 + (datalen * 4) + 1] = 1;
	sendbuffer[12 + (datalen * 4) + 2] = 2;
	sendbuffer[12 + (datalen * 4) + 3] = 3;
// 	sendbuffer[10 + (datalen * 4) + 4] = 2;
// 	sendbuffer[10 + (datalen * 4) + 5] = 1;
	
}

void vQuamGen(void *pvParameters) { /*Task selber. Nur Delays Es wird alles über Interrupts gelöst. HIer könnte man die Createsenddata function einbringen */
	while(evDMAState == NULL) {
		vTaskDelay(3/portTICK_RATE_MS);
	}
	xEventGroupWaitBits(evDMAState, DMAGENREADY, false, true, portMAX_DELAY);
	for(;;) {
		switch (debug_gen){
			case 0:
				createSendData();
				debug_gen = 1;
				break;
			/*Simulation for a random bit stream Deleted for Final*/
			case 3:
				
				for(int i = 0; i < 29; i++){
				
					sendbuffer[i] = rand()%3;
				
				}
				
				
				
				break;
		}
		vTaskDelay(1/portTICK_RATE_MS);
	}
}

void fillBuffer(uint16_t buffer[NR_OF_SAMPLES]) { // HIer werden die Daten für den DAC Befült. In unserem Fall müssten wir schauen wie lange wir warten müssen bis wir den Impuls schicken können.
	switch (sendbuffer[0])
	{
		case 4:
		return; //für Erstellung Idel Senddata :)
		break;
	}
	
	static int pSendbuffer = 0;
	
	for(int i = 0; i < NR_OF_SAMPLES;i++) {
		
		switch(sendbuffer[pSendbuffer]){
			case 0:
				buffer[i] = 0x800 + (Impuls1[i]);
				break;
			case 1:
				buffer[i] = 0x800 + (Impuls2[i]);
				break;
			case 2:
				buffer[i] = 0x800 + (Impuls3[i]);
				break;
			case 3:
				buffer[i] = 0x800 + (Impuls4[i]);
				break;
		}
		
	}
	if(pSendbuffer <= SENDBUFFER_SIZE-1) {
		pSendbuffer++;
	} else {
		debug_gen = 0;
		pSendbuffer = 0;
	}
}

ISR(DMA_CH0_vect)
{
	//static signed BaseType_t test;
	
	DMA.CH0.CTRLB|=0x10;
	fillBuffer(&dacBuffer0[0]);
}

ISR(DMA_CH1_vect)
{
	DMA.CH1.CTRLB|=0x10;
	fillBuffer(&dacBuffer1[0]);
}