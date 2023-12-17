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

#define BitMask 0x000000FF
#define Idel0 0
#define Idel1 1
#define type 2
#define sync 3
#define Data 4
#define checksum 5


#define quarterjump1 7
#define quarterjump2 8 //Perfekt f�r Sync Weil diese Spr�nge nur in einem Fall auftreten k�nnen 3 -> 0
#define quarterjump3 9
#define quarterjump4 10
#define quarterjump5 11
#define quarterjump6 12

#define halfjump1 15
#define halfjump2 16
#define halfjump3 17
#define halfjump4 18
#define halfjump5 19
#define halfjump6 20

#define threequartersjump1 23
#define threequartersjump2 24
#define threequartersjump3 25
#define threequartersjump4 26
#define threequartersjump5 27
#define threequartersjump6 28

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
#define onethreequartersjump2 56 //Perfekt f�r Sync Weil diese Spr�nge nur in einem Fall auftreten k�nnen 0 -> 3
#define onethreequartersjump3 57

#define HI 2150
#define LO 1200


SemaphoreHandle_t xMutex = NULL;
uint8_t receivebuffer[50];
uint8_t k = 0; // Nicht Best Practise Provisorium!!
uint8_t checksumGL = 0; // Initialisierung der Checksumme
uint8_t calculatedChecksum = 0; // Variable f�r die berechnete Checksumme
float reconstructedFloat; // Nicht Best Practise Provisorium!!


	uint16_t ringbuffer[256] = {
		LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
		LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
		LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
		LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,HI,LO,LO,LO,
		LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
		LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,HI,LO,LO,LO,
		LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
		LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,HI,LO,LO,LO};



uint8_t quarterjump(uint8_t lastnumber){
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

uint8_t halfjump(uint8_t lastnumber){
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

uint8_t threequartersjump(uint8_t lastnumber){
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

uint8_t fulljump(uint8_t lastnumber){
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

uint8_t onequarterjump(uint8_t lastnumber){
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

uint8_t onehalfjump(uint8_t lastnumber){
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

uint8_t onethreequartersjump(uint8_t lastnumber){
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

uint8_t analyzediff(uint8_t Pos, uint8_t nexpos, uint8_t lastnumber);

uint8_t getNextHighPos(uint32_t Pos){
	int16_t syncpos = -1;

	for (int i = 0; i < 60; ++i)
	{
		Pos = Pos + 2 ;
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

void vTest(void *pvParameters){
	uint32_t read_pos = 0;
	int16_t pos = 0;
	int16_t nextpos = 0;
	uint8_t currentnumber = 0;
	uint8_t lastnumber = 0;
	uint8_t protocolmode = 0;
	xMutex = xSemaphoreCreateMutex();

	
	
// 	uint16_t ringbuffer[256] = {LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
// 								LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
// 								LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
// 								LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,HI,LO,LO,LO,
// 								LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
// 								LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,HI,LO,LO,LO,
// 								LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
// 								LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,HI,LO,LO,LO};
								
	//uint16_t ringbuffer[256] = {LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
		//LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
		//LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,HI,LO,LO,LO,
		//LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
		//LO,LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
		//LO,LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
		//LO,LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,
	//LO,LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO,HI,LO,LO,LO,LO,LO,LO,LO,LO,LO,LO};
	
	(void) pvParameters;
	
	for (;;)
	{ /*Data ist eine Biilig Counting Semaphore weill ich noch keines erstellen konnte 05.12.2023*/
		xSemaphoreTake(xMutex, portMAX_DELAY);
		if (((write_pos) - (read_pos)) >= 70 )
		{
			pos = getNextHighPos(read_pos);
			nextpos = getNextHighPos(pos);
			currentnumber = analyzediff(pos, nextpos, lastnumber);
			lastnumber = currentnumber;
			read_pos = nextpos-4;
			
			switch(protocolmode){ //Sinnvoll hier das mit dem Receivbuffer zu machen?
				case Idel0:
					if (protocolmode == Idel0 && currentnumber == 2)
					{
						protocolmode = type;
					}if (currentnumber == 0)
					{
						protocolmode = Idel1;
					}
					break;
				case Idel1:
					if (currentnumber == 3)
					{
						protocolmode = Idel0;
					}
					break;
				case type:
					
					break;
				case sync:
					
					break;
				case Data:
					
					break;
				case checksum:
					
					break;
				
				
			}
			
// 			switch(protocolmode) {
// 				case Idele0:
// 					if(impulseValue == 0)
// 						protocolmode = Idle1:
// 				break;
// 				case Idle1:
// 					if(impulseValue == 3)
// 						protocolmode = Idle0:
// 				break;
// 				case Sync:
// 					if(protocolmode == Idle0 && impulseValue == 2):
// 						protocolmode =  type;
// 				break;
// 				case type:
// 				
// 				break;
// 				case data:
// 				
// 				break;
// 				case checksum:
// 				
// 				brek;
//				}
				
		}
	xSemaphoreGive(xMutex);
	vTaskDelay(1/portTICK_RATE_MS);
	}
}


uint8_t analyzediff(uint8_t Pos, uint8_t nexpos, uint8_t number){
	uint8_t Offset = 0;
	uint8_t symbol = 0;
	uint8_t newnumber = 0;
	
	Offset = nexpos - Pos;
	
	switch(Offset){ // Startwert ist 3
		case quarterjump1: //Cases zusammenf�hren f�r weniger zeilen code!! case1:case2:case3: Code break;
		newnumber = quarterjump(number); //Wenn man zu oft hier landet kann man beim Offset noch +1 dazurechnen
		break;
		case quarterjump2:
		newnumber = quarterjump(number);
		break;
		case quarterjump3:
		newnumber = quarterjump(number);
		break;
		case quarterjump4:
		newnumber = quarterjump(number);
		break;
		case quarterjump5:
		newnumber = quarterjump(number);
		break;
		case quarterjump6:
		newnumber = quarterjump(number);
		break;
		case halfjump1:
		newnumber = halfjump(number);
		break;
		case halfjump2:
		newnumber = halfjump(number);
		break;
		case halfjump3:
		newnumber = halfjump(number);
		break;
		case halfjump4:
		newnumber = halfjump(number);
		break;
		case halfjump5:
		newnumber = halfjump(number);
		break;
		case halfjump6:
		newnumber = halfjump(number);
		break;
		case threequartersjump1:
		newnumber = threequartersjump(number);
		break;
		case threequartersjump2:
		newnumber = threequartersjump(number);
		break;
		case threequartersjump3:
		newnumber = threequartersjump(number);
		break;
		case threequartersjump4:
		newnumber = threequartersjump(number);
		break;
		case threequartersjump5:
		newnumber = threequartersjump(number);
		break;
		case threequartersjump6:
		newnumber = threequartersjump(number);
		break;
		case fulljump1:
		newnumber = fulljump(number);
		break;
		case fulljump2:
		newnumber = fulljump(number);
		break;
		case fulljump3:
		newnumber = fulljump(number);
		break;
		case onequarterjump1:
		newnumber = onequarterjump(number);
		break;
		case onequarterjump2:
		newnumber = onequarterjump(number);
		break;
		case onequarterjump3:
		newnumber = onequarterjump(number);
		break;
		case onehalfjump1:
		newnumber = onehalfjump(number);
		break;
		case onehalfjump2:
		newnumber = onehalfjump(number);
		break;
		case onehalfjump3:
		newnumber = onehalfjump(number);
		break;
		case onethreequartersjump1:
		newnumber = onethreequartersjump(number);
		break;
		case onethreequartersjump2:
		newnumber = onethreequartersjump(number);
		break;
		case onethreequartersjump3:
		newnumber = onethreequartersjump(number);
		break;
		default:
		//Code für Resett einbauen
		break;
	}
	k++;
	switch(k){
		case 32: //Wert noch anpassen working 62
		k = 0;
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
		if (calculatedChecksum == checksumGL) {
			 //Schluss vom Analyze Teil
			//CODE Für TEMP Auslesen!	

			
		}
		
		for (int i = 0; i < 32; i++) //Nötig?
		{
			receivebuffer[i] = 0; //Mutex!
		}

		checksumGL = 0;

		break;
		case 4:
			 		if (!((receivebuffer[0] == 0) && (receivebuffer[1] == 3) && (receivebuffer[2] == 0) && (receivebuffer[3] == 3))) //Gesammte P�ckchen Anzahl muss durch 4 Sauber geteitl werden k�nnen
			 		{
			 			k = 0;
			 			for (int i = 0; i < 4; i++)
			 			{
			 				receivebuffer[i] = 0;
			 			}
			 		}
			break;

	}
	return newnumber;
}