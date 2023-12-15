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

#define BitMask 0x00FF

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

uint8_t receivebuffer[50];

uint8_t lastnumber = 0;
uint8_t k = 0;
uint8_t Offset = 0;

uint8_t checksumGL = 0; // Initialisierung der Checksumme
uint8_t calculatedChecksum = 0; // Variable f�r die berechnete Checksumme
float reconstructedFloat;




void quarterjump(void){
	switch(lastnumber){
		case 0:
		receivebuffer[k] = 1;
		lastnumber = 1;
		break;
		case 1:
		receivebuffer[k] = 2;
		lastnumber = 2;
		break;
		case 2:
		receivebuffer[k] = 3;
		lastnumber = 3;
		break;
		case 3:
		receivebuffer[k] = 0;
		lastnumber = 0;
		break;
	}
}

void halfjump(void){
	switch(lastnumber){
		case 0:
		receivebuffer[k] = 2;
		lastnumber = 2;
		break;
		case 1:
		receivebuffer[k] = 3;
		lastnumber = 3;
		break;
		case 2:
		receivebuffer[k] = 0;
		lastnumber = 0;
		break;
		case 3:
		receivebuffer[k] = 1;
		lastnumber = 1;
		break;
	}
	
}

void threequartersjump(void){
	switch(lastnumber){
		case 0:
		receivebuffer[k] = 3;
		lastnumber = 3;
		break;
		case 1:
		receivebuffer[k] = 0;
		lastnumber = 0;
		break;
		case 2:
		receivebuffer[k] = 1;
		lastnumber = 1;
		break;
		case 3:
		receivebuffer[k] = 2;
		lastnumber = 2;
		break;
	}
	
}

void fulljump(void){
	switch(lastnumber){
		case 0:
		receivebuffer[k] = 0;
		lastnumber = 0;
		break;
		case 1:
		receivebuffer[k] = 1;
		lastnumber = 1;
		break;
		case 2:
		receivebuffer[k] = 2;
		lastnumber = 2;
		break;
		case 3:
		receivebuffer[k] = 3;
		lastnumber = 3;
		break;
	}
}

void onequarterjump(void){
	switch(lastnumber){
		case 0:
		receivebuffer[k] = 1;
		lastnumber = 1;
		break;
		case 1:
		receivebuffer[k] = 2;
		lastnumber = 2;
		break;
		case 2:
		receivebuffer[k] = 3;
		lastnumber = 3;
		break;
		case 3:
		receivebuffer[k] = 0;
		lastnumber = 0;
		break;
	}
}

void onehalfjump(void){
	switch(lastnumber){
		case 0:
		receivebuffer[k] = 2;
		lastnumber = 2;
		break;
		case 1:
		receivebuffer[k] = 3;
		lastnumber = 3;
		break;
		case 2:
		receivebuffer[k] = 0;
		lastnumber = 0;
		break;
		case 3:
		receivebuffer[k] = 1;
		lastnumber = 1;
		break;
	}
}

void onethreequartersjump(void){
	switch(lastnumber){
		case 0:
		receivebuffer[k] = 3;
		lastnumber = 3;
		break;
		case 1:
		receivebuffer[k] = 0;
		lastnumber = 0;
		break;
		case 2:
		receivebuffer[k] = 1;
		lastnumber = 1;
		break;
		case 3:
		receivebuffer[k] = 2;
		lastnumber = 2;
		break;
	}
}

void analyzediff();

void vTest(void *pvParameters){
	
	(void) pvParameters;
	
	for (;;)
	{ /*Data ist eine Biilig Counting Semaphore weill ich noch keines erstellen konnte 05.12.2023*/
		
		
		vTaskDelay(2/portTICK_RATE_MS);
	}
}

void analyzediff(){

	uint8_t symbol = 0;
	switch(Offset){ // Startwert ist 3
		case quarterjump1: //Cases zusammenf�hren f�r weniger zeilen code!! case1:case2:case3: Code break;
		quarterjump(); //Wenn man zu oft hier landet kann man beim Offset noch +1 dazurechnen
		break;
		case quarterjump2:
		quarterjump();
		break;
		case quarterjump3:
		quarterjump();
		break;
		case quarterjump4:
		quarterjump();
		break;
		case quarterjump5:
		quarterjump();
		break;
		case quarterjump6:
		quarterjump();
		break;
		case halfjump1:
		halfjump();
		break;
		case halfjump2:
		halfjump();
		break;
		case halfjump3:
		halfjump();
		break;
		case halfjump4:
		halfjump();
		break;
		case halfjump5:
		halfjump();
		break;
		case halfjump6:
		halfjump();
		break;
		case threequartersjump1:
		threequartersjump();
		break;
		case threequartersjump2:
		threequartersjump();
		break;
		case threequartersjump3:
		threequartersjump();
		break;
		case threequartersjump4:
		threequartersjump();
		break;
		case threequartersjump5:
		threequartersjump();
		break;
		case threequartersjump6:
		threequartersjump();
		break;
		case fulljump1:
		fulljump();
		break;
		case fulljump2:
		fulljump();
		break;
		case fulljump3:
		fulljump();
		break;
		case onequarterjump1:
		onequarterjump();
		break;
		case onequarterjump2:
		onequarterjump();
		break;
		case onequarterjump3:
		onequarterjump();
		break;
		case onehalfjump1:
		onehalfjump();
		break;
		case onehalfjump2:
		onehalfjump();
		break;
		case onehalfjump3:
		onehalfjump();
		break;
		case onethreequartersjump1:
		onethreequartersjump();
		break;
		case onethreequartersjump2:
		onethreequartersjump();
		break;
		case onethreequartersjump3:
		onethreequartersjump();
		break;
		default:
		//Code f�r Resett einbauen
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

	}
}