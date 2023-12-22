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
#include "stdio.h"
#include "string.h"

#include "math.h"
#include "LSM9DS1Defines.h"
#include "LSM9DS1Driver.h"
#include "twiMaster.h"

#define IDEL 101
#define DATA 100

TickType_t old_time, new_time;

uint8_t Chaos_data = 0; //Nur F�r Testzwecke ChaosData! Kann sp�ter Gel�scht werden
uint8_t sendbuffer[50] = {4,4,4,4,4,4,4,4};
uint8_t Modus = IDEL;
uint8_t sendID = 0;
uint8_t debug_gen = 0;
float temparatur = 0;

const int16_t Impuls1[NR_OF_SAMPLES] = {0x148, 0x355, 0x5C1, 0x7FF, 0x7FF, 0x5C1, 0x355, 0x148, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,};

const int16_t Impuls2[NR_OF_SAMPLES] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x148, 0x355, 0x5C1, 0x7FF, 0x7FF, 0x5C1, 0x355, 0x148, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,};

const int16_t Impuls3[NR_OF_SAMPLES] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x148, 0x355, 0x5C1, 0x7FF, 0x7FF, 0x5C1, 0x355, 0x148, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

const int16_t Impuls4[NR_OF_SAMPLES] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x148, 0x355, 0x5C1, 0x7FF, 0x7FF, 0x5C1, 0x355, 0x148,};

#define SENDBUFFER_SIZE 31

unsigned char byteArray[4];		// Float-Daten als binary

void printBinary(unsigned char byte) {
	for (int i = 7; i >= 0; --i) {
		printf("%u", (byte >> i) & 1);  // Hier verwenden wir %u f�r unsigned
	}
}

int createBinary() {
	
	// Verwendung eines Zeigers und Typumwandlung, um float in 4-Byte-Array zu konvertieren
	unsigned char *ptr = (unsigned char*)&temparatur;

	// Kopieren der Bytes von float in das Array
	for (int i = 0; i < sizeof(float); ++i) {
		byteArray[i] = *(ptr + i);
	}

	for (int i = 0; i < sizeof(float); ++i) {
		printBinary(byteArray[i]);
	}
	return 0;
}

void createSendData() { //0 -> 3 & 3-> 0 sind Idel Task (createideldata)
	sendID++;
	char senddata[4];
	for (int i = 0; i < 4; i++) {
		senddata[i] = (char)byteArray[i];
	}
	uint8_t datalen = 4;
	/*Header Start*/
	sendbuffer[0] = 0;
	sendbuffer[1] = 2;
	sendbuffer[2] = 0;
	sendbuffer[3] = 2;
	sendbuffer[4] = (sendID >> 0) & 0x03;
	sendbuffer[5] = (sendID >> 2) & 0x03;
	sendbuffer[6] = (sendID >> 4) & 0x03;
	sendbuffer[7] = (sendID >> 6) & 0x03;
	sendbuffer[8] = (datalen >> 0) & 0x03;
	sendbuffer[9] = (datalen >> 2) & 0x03;
	sendbuffer[10] = (datalen >> 4) & 0x03;
	sendbuffer[11] = (datalen >> 6) & 0x03;
	/*Header END*/
	for(int i = 0; i < datalen;i++) {
		sendbuffer[12 + i*4 + 0] = (senddata[i] >> 0) & 0x03; //12 steht f�r die Gr�sse vom Header
		sendbuffer[12 + i*4 + 1] = (senddata[i] >> 2) & 0x03;	// Reihenfolge +n vom sendbuffer ge�ndert wegen LIFO
		sendbuffer[12 + i*4 + 2] = (senddata[i] >> 4) & 0x03;
		sendbuffer[12 + i*4 + 3] = (senddata[i] >> 6) & 0x03;
	} //K nur bis hier mitrechnen
	uint8_t checksum = 0;
	for(int i = 0; i < 12 + (datalen * 4); i++) {
		checksum += sendbuffer[i];
	}
	sendbuffer[12 + (datalen * 4) + 0] = 0;  //Die Checksume wird auf 2bit Paare aufgeteilt
	sendbuffer[12 + (datalen * 4) + 1] = 1;
	sendbuffer[12 + (datalen * 4) + 2] = 2;
	sendbuffer[12 + (datalen * 4) + 3] = 3;
}

void vQuamGen(void *pvParameters) {
	while(evDMAState == NULL) {
		vTaskDelay(3/portTICK_RATE_MS);
	}
	xEventGroupWaitBits(evDMAState, DMAGENREADY, false, true, portMAX_DELAY);
	for(;;) {

		switch(debug_gen)
		{case 3: // Nur F�r Testzwecke ChaosData! Kann sp�ter von 3 zu 0 getauscht werden
				createSendData();
				Modus = IDEL;
				debug_gen = 1;
				break;
			/************************************************************************/
			/*        Simulation for a random bit stream Deleted for Final          */
			/************************************************************************/
			case 0:
				sendbuffer[0] = 3;
					for (int i = 0; i < 32; i = ++i)
					{
						sendbuffer[i] = 0;
						sendbuffer[++i] = 3;
					}
				debug_gen = 1;
				break;
			/************************************************************************/
			/*        END OF SIMULATION                                             */
			/************************************************************************/
		}
		if (new_time - old_time >= 1000)
		{
			 				readTempData();
			 				temparatur =  getTemperatureData();
			 				printBinary(byteArray[4]);
			 				createBinary();
							old_time = new_time;
							Modus = DATA;
		}else{
			new_time = xTaskGetTickCount();
		}
		vTaskDelay(1/portTICK_RATE_MS);
	}
}

void fillBuffer(uint16_t buffer[NR_OF_SAMPLES]) {
	static int pSendbuffer = 0;
	
	switch (sendbuffer[0])
	{
		case 4:
		return; //f�r Erstellung Idel Senddata :)
		break;
	}
	
	for(int i = 0; i < NR_OF_SAMPLES;i++) {
		switch(sendbuffer[pSendbuffer]) {
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
		/************************************************************************/
		/*        Simulation for a random bit stream Deleted for Final          */
		/************************************************************************/
		switch(Modus){
			case DATA:
			debug_gen = 3;
			break;
			case IDEL:
			debug_gen = 0;
			break;
			/************************************************************************/
			/*        END OF SIMULATION                                             */
			/************************************************************************/
		}
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