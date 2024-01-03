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
#include "Defines.h"

/*Array Init*/
unsigned char byteArray[4];

/*Semaphore Init*/
SemaphoreHandle_t xMutex = NULL;

/*Var Init*/
uint8_t receivebuffer[50];
uint8_t k = 0; // Nicht Best Practise Provisorium!!
uint8_t checksumGL = 0; // Initialisierung der Checksumme
uint8_t calculatedChecksum = 0; // Variable f�r die berechnete Checksumme
uint8_t debug = 0;
float reconstructedFloat = 0; // Nicht Best Practise Provisorium!!


uint8_t quarterjump(uint8_t lastnumber, uint8_t k){
	uint8_t newnumber = 0;
	receivebuffer[k] = 0;
	return newnumber;
}

uint8_t halfjump(uint8_t lastnumber, uint8_t k){
	uint8_t newnumber = 0;
	switch(lastnumber){
		case 0:
		receivebuffer[k] = 2;
		newnumber = 2;
		return newnumber;
		break;
		case 1:
		receivebuffer[k] = 3;
		newnumber = 3;
		return newnumber;
		break;
		case 2:
		receivebuffer[k] = 0;
		newnumber = 0;
		return newnumber;
		break;
		case 3:
		receivebuffer[k] = 1;
		newnumber = 1;
		return newnumber;
		break;
		default:
		return 4;
		break;
	}
	
}

uint8_t threequartersjump(uint8_t lastnumber, uint8_t k){
	uint8_t newnumber = 0;
	switch(lastnumber){
		case 0:
		receivebuffer[k] = 3;
		newnumber = 3;
		return newnumber;
		break;
		case 1:
		receivebuffer[k] = 0;
		newnumber = 0;
		return newnumber;
		break;
		case 2:
		receivebuffer[k] = 1;
		newnumber = 1;
		return newnumber;
		break;
		case 3:
		receivebuffer[k] = 2;
		newnumber = 2;
		return newnumber;
		break;
		default:
		return 4;
		break;
	}
	
}

uint8_t fulljump(uint8_t lastnumber, uint8_t k){
	uint8_t newnumber = 0;
	switch(lastnumber){
		case 0:
		receivebuffer[k] = 0;
		newnumber = 0;
		return newnumber;
		break;
		case 1:
		receivebuffer[k] = 1;
		newnumber = 1;
		return newnumber;
		break;
		case 2:
		receivebuffer[k] = 2;
		newnumber = 2;
		return newnumber;
		break;
		case 3:
		receivebuffer[k] = 3;
		newnumber = 3;
		return newnumber;
		break;
		default:
		return 4;
		break;
	}
}

uint8_t onequarterjump(uint8_t lastnumber, uint8_t k){
	uint8_t newnumber = 0;
	
	switch(lastnumber){
		case 0:
		receivebuffer[k] = 1;
		newnumber = 1;
		return newnumber;
		break;
		case 1:
		receivebuffer[k] = 2;
		newnumber = 2;
		return newnumber;
		break;
		case 2:
		receivebuffer[k] = 3;
		newnumber = 3;
		return newnumber;
		break;
		case 3:
		receivebuffer[k] = 0;
		newnumber = 0;
		return newnumber;
		break;
		default:
		return 4;
		break;
	}
}

uint8_t onehalfjump(uint8_t lastnumber, uint8_t k){
	uint8_t newnumber = 0;
	switch(lastnumber){
		case 0:
		receivebuffer[k] = 2;
		newnumber = 2;
		return newnumber;
		break;
		case 1:
		receivebuffer[k] = 3;
		newnumber = 3;
		return newnumber;
		break;
		case 2:
		receivebuffer[k] = 0;
		newnumber = 0;
		return newnumber;
		break;
		case 3:
		receivebuffer[k] = 1;
		newnumber = 1;
		return newnumber;
		break;
		default:
		return 4;
		break;
	}
}

uint8_t onethreequartersjump(uint8_t lastnumber, uint8_t k){
	uint8_t newnumber = 0;
	switch(lastnumber){
		case 0:
		receivebuffer[k] = 3;
		newnumber = 3;
		return newnumber;
		break;
		case 1:
		receivebuffer[k] = 0;
		newnumber = 0;
		return newnumber;
		break;
		case 2:
		receivebuffer[k] = 1;
		newnumber = 1;
		return newnumber;
		break;
		case 3:
		receivebuffer[k] = 2;
		newnumber = 2;
		return newnumber;
		break;
		default:
		return 4;
		break;
	}
}

uint8_t analyzediff(int16_t Pos, int16_t nextpos, uint8_t lastnumber, uint8_t k, int8_t * p_drift);

int16_t getNextHighPos(uint32_t Pos){ //Funktion für das Suchen der Peaks im Ringpuffer
	int16_t syncpos = -1;

	for (int i = 0; i < 60; ++i)
	{
		Pos = Pos + 2;
		if ((ringbuffer[Pos & BitMask] > 2000)) //Wert 2000 über Durchschnitt peak vom Idel Stream setzten!
		{
			syncpos = (Pos & BitMask);
			return syncpos;
		}
	}
	if (syncpos != -1)
	{
		return syncpos;
	}else{
		return -1;
	}
}

void getDataTemp(void) {
	uint8_t datalenght = 4;
	static int pReceivebuffer = 12;
	static int byteArrayIndex = 0;

	for (int i = 0; i < 16; i++) {
		switch (receivebuffer[pReceivebuffer]) {
			case 0:
			byteArray[byteArrayIndex] |= (0b000 << (i % 4) * 2);
			break;
			case 1:
			byteArray[byteArrayIndex] |= (0b001 << (i % 4) * 2);
			break;
			case 2:
			byteArray[byteArrayIndex] |= (0b010 << (i % 4) * 2);
			break;
			case 3:
			byteArray[byteArrayIndex] |= (0b11 << (i % 4) * 2); 
			break;
		}
		pReceivebuffer++;

		// Wenn viermal ins byteArray[0] geschrieben wurde, gehe zum nächsten Element
		if ((i + 1) % datalenght == 0) {
			byteArrayIndex++;
		}
	}

	// Zurücksetzen von pReceivebuffer und byteArrayIndex, wenn es am Ende des receivebuffer angelangt ist
	if (pReceivebuffer >= 28) {
		pReceivebuffer = 12;
		byteArrayIndex = 0;
	}
}

void vAnalyze(void *pvParameters){
	uint32_t read_pos = 0;
	int16_t pos = 0;
	int16_t nextpos = 0;
	uint8_t currentnumber = 4;
	uint8_t lastnumber = 0;
	uint8_t protocolmode = 0;
	uint8_t RX_Pos = 0;
	uint8_t symbol = 0;
	int8_t drift = 0;
	xMutex = xSemaphoreCreateMutex();

	(void) pvParameters;
	
	for (;;)
	{ 
		xSemaphoreTake(xMutex, portMAX_DELAY); //Mutex für die Variabel write_pos!
		if (((write_pos) - (read_pos)) >= 70 ) 
		{
			xSemaphoreGive(xMutex);
			pos = getNextHighPos(read_pos);
			nextpos = getNextHighPos(pos);
			currentnumber = analyzediff(pos, nextpos, lastnumber, RX_Pos, &drift); 
			RX_Pos++;
			lastnumber = currentnumber;
			if (nextpos == -1) // Falls wir kein neuen Peak finden konnten!
			{
				read_pos = pos-4;
			}else{
			read_pos = nextpos-4; // Damit wir beim nächsten Durchlauf den letzten Peak wieder finden können
			}
			switch(protocolmode){  // Hier wird unser Protocolhandling betrieben
				case Idel0:
					if (currentnumber == 0)
					{
						protocolmode = Idel1;
					}
					break;
				case Idel1:
					if (currentnumber == 2)
					{
						protocolmode = type;
					}
					if (currentnumber == 3)
					{
						protocolmode = Idel0;
					}
					break;
				case type:
					if (currentnumber == 0)
					{
						protocolmode = sync;
					}else
					{
						protocolmode = Idel0;						
					}
					break;
				case sync: 
					if (currentnumber == 2) // Ab diesem Zeitpunkt sind wir uns Sicher dass wir den Start von einem Datenpaket gefunden haben.
					{
						RX_Pos = 4;
						debug = 1;
						receivebuffer[0] = 0;
						receivebuffer[1] = 2;
						receivebuffer[2] = 0;
						receivebuffer[3] = 2;
						protocolmode = Data;
					}else{
						protocolmode = Idel0;
					}
					break;
				case Data:
					if(RX_Pos == 31){
						protocolmode = checksum;
					}else
					{
						break;					
					}
					break;
				case checksum:
					for (size_t i = 0; i < (NR_OF_SAMPLES-4); i++) { 
						calculatedChecksum += receivebuffer[i];
					}
					for (int z = 0; z < NR_OF_SAMPLES-4; z++){
						symbol = receivebuffer[z]; // Generiert das Signal entsprechend der Zeit seit dem letzten "Peak"-Signal
						// Decodiere das Symbol
						switch (symbol){
							case 0:
							checksumGL += receivebuffer[z];
							break;
							case 1:
							checksumGL += receivebuffer[z];
							break;
							case 2:
							checksumGL += receivebuffer[z];
							break;
							case 3:
							checksumGL += receivebuffer[z];
							break;
							default:
							// Auf Dispaly "Unknown symbol received!"
							break;
						}
					}
					if ((receivebuffer[28] == 0) && (receivebuffer[29] == 1) && (receivebuffer[30] == 2) && (receivebuffer[31] == 3))
					{
						protocolmode = FINAL;
						getDataTemp();
						memcpy(&reconstructedFloat, byteArray, sizeof(float));
						for (int i = 0; i < 4 ; i++)
						{
							byteArray[i] = 0;
						}
					}else{
						RX_Pos = 0;
						protocolmode = Idel0;
					}
					break;
				case FINAL:
					
						protocolmode = Idel0;
						RX_Pos = 0;
					break;	
			}	
	}else
	{
		xSemaphoreGive(xMutex);
	}
	
	vTaskDelay(1/portTICK_RATE_MS);
	}
}

uint8_t analyzediff(int16_t Pos, int16_t nextpos, uint8_t number, uint8_t rxpos, int8_t * p_drift){ //Funktion für die Analyse der Differenz von zwei Peaks 
	uint8_t Offset = 0;
	
	uint8_t newnumber = 4;
	if (nextpos == -1)
	{
		Offset = 8;
	}else{
	Offset = nextpos - Pos;
	Offset = Offset + *p_drift;
	}
	switch(Offset){ 
		case quarterjump1: 
		newnumber = quarterjump(number, rxpos); //Wenn man zu oft hier landet kann man beim Offset noch +1 dazurechnen
		*p_drift = 1;
		break;
		case quarterjump2:
		newnumber = quarterjump(number, rxpos);
		*p_drift = 0;
		break;
		case quarterjump3:
		newnumber = quarterjump(number, rxpos);
		*p_drift = -1;
		break;
		case quarterjump4:
		newnumber = quarterjump(number, rxpos);
		*p_drift = -1;
		break;
		case quarterjump5:
		newnumber = quarterjump(number, rxpos);
		*p_drift = -1;
		break;
		case quarterjump6:
		newnumber = quarterjump(number, rxpos);
		*p_drift = -1;
		break;
		case halfjump0:
		newnumber = halfjump(number, rxpos);
		*p_drift = 1;
		break;
		case halfjump1:
		newnumber = halfjump(number, rxpos);
		*p_drift = 1;
		break;
		case halfjump2:
		newnumber = halfjump(number, rxpos);
		*p_drift = 0;
		break;
		case halfjump3:
		newnumber = halfjump(number, rxpos);
		*p_drift = -1;
		break;
		case halfjump4:
		newnumber = halfjump(number, rxpos);
		*p_drift = -1;
		break;
		case halfjump5:
		newnumber = halfjump(number, rxpos);
		*p_drift = -1;
		break;
		case halfjump6:
		newnumber = halfjump(number, rxpos);
		*p_drift = -1;
		break;
		case halfjump7:
		newnumber = halfjump(number, rxpos);
		*p_drift = -1;
		break;
		case threequartersjump0:
		newnumber = threequartersjump(number, rxpos);
		*p_drift = 1;
		break;
		case threequartersjump1:
		newnumber = threequartersjump(number, rxpos);
		*p_drift = 1;
		break;
		case threequartersjump2:
		newnumber = threequartersjump(number, rxpos);
		*p_drift = 0;
		break;
		case threequartersjump3:
		newnumber = threequartersjump(number, rxpos);
		*p_drift = -1;
		break;
		case threequartersjump4:
		newnumber = threequartersjump(number, rxpos);
		*p_drift = -1;
		break;
		case threequartersjump5:
		newnumber = threequartersjump(number, rxpos);
		*p_drift = -1;
		break;
		case threequartersjump6:
		newnumber = threequartersjump(number, rxpos);
		*p_drift = -1;
		break;
		case fulljump0:
		newnumber = fulljump(number, rxpos);
		*p_drift = 1;
		break;
		case fulljump1:
		newnumber = fulljump(number, rxpos);
		*p_drift = 1;
		break;
		case fulljump2:
		newnumber = fulljump(number, rxpos);
		*p_drift = 0;
		break;
		case fulljump3:
		newnumber = fulljump(number, rxpos);
		*p_drift = -1;
		break;
		case fulljump4:
		newnumber = fulljump(number, rxpos);
		*p_drift = -1;
		break;
		case fulljump5:
		newnumber = fulljump(number, rxpos);
		*p_drift = -1;
		break;
		case fulljump6:
		newnumber = fulljump(number, rxpos);
		*p_drift = -1;
		break;
		case onequarterjump0:
		newnumber = onequarterjump(number, rxpos);
		*p_drift = 1;
		break;
		case onequarterjump1:
		newnumber = onequarterjump(number, rxpos);
		*p_drift = 1;
		break;
		case onequarterjump2:
		newnumber = onequarterjump(number, rxpos);
		*p_drift = 0;
		break;
		case onequarterjump3:
		newnumber = onequarterjump(number, rxpos);
		*p_drift = -1;
		break;
		case onequarterjump4:
		newnumber = onequarterjump(number, rxpos);
		*p_drift = -1;
		break;
		case onequarterjump5:
		newnumber = onequarterjump(number, rxpos);
		*p_drift = -1;
		break;
		case onequarterjump6:
		newnumber = onequarterjump(number, rxpos);
		*p_drift = -1;
		break;
		case onequarterjump7:
		newnumber = onequarterjump(number, rxpos);
		*p_drift = -1;
		break;
		case onehalfjump0:
		newnumber = onehalfjump(number, rxpos);
		*p_drift = 1;
		break;
		case onehalfjump1:
		newnumber = onehalfjump(number, rxpos);
		*p_drift = 1;
		break;
		case onehalfjump2:
		newnumber = onehalfjump(number, rxpos);
		*p_drift = 0;
		break;
		case onehalfjump3:
		newnumber = onehalfjump(number, rxpos);
		*p_drift = -1;
		break;
		case onehalfjump4:
		newnumber = onehalfjump(number, rxpos);
		*p_drift = -1;
		break;
		case onehalfjump5:
		newnumber = onehalfjump(number, rxpos);
		*p_drift = -1;
		break;
		case onehalfjump6:
		newnumber = onehalfjump(number, rxpos);
		*p_drift = -1;
		break;
		case onehalfjump7:
		newnumber = onehalfjump(number, rxpos);
		*p_drift = -1;
		break;
		case onethreequartersjump0:
		newnumber = onethreequartersjump(number, rxpos);
		*p_drift = 1;
		break;
		case onethreequartersjump1:
		newnumber = onethreequartersjump(number, rxpos);
		*p_drift = 1;
		break;
		case onethreequartersjump2:
		newnumber = onethreequartersjump(number, rxpos);
		*p_drift = 0;
		break;
		case onethreequartersjump3:
		newnumber = onethreequartersjump(number, rxpos);
		*p_drift = -1;
		break;
		case onethreequartersjump4:
		newnumber = onethreequartersjump(number, rxpos);
		*p_drift = -1;
		break;
		case onethreequartersjump5:
		newnumber = onethreequartersjump(number, rxpos);
		*p_drift = -1;
		break;
		case onethreequartersjump6:
		newnumber = onethreequartersjump(number, rxpos);
		*p_drift = -1;
		break;
		default:
		//Code für Resett einbauen
		break;
	}
	//k++;
	switch(rxpos){
		case 32: 
		if (debug == 1)
		{
		rxpos = 0;
		if (calculatedChecksum == checksumGL) {	
		}
		debug = 0;
		checksumGL = 0;
		}else{
			break;
		}
		break;
	}
	return newnumber;
}
void vDisplay(void* pvParameters){
	
	for (;;) {
		
		vDisplayClear();
		vDisplayWriteStringAtPos(0,0, "%f 'C", reconstructedFloat);
		vTaskDelay(1000/portTICK_RATE_MS);
	}
}