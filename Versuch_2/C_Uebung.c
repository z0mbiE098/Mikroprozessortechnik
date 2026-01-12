/********************************************************************/
/*  Hochschule fuer Technik und Wirtschaft                          */
/*  Fakultät fuer Ingenieurwissenschaften                           */
/*  Labor fuer Eingebettete Systeme                                 */
/*  Mikroprozessortechnik                                           */
/********************************************************************/
/*                                                                  */
/*  C_Übung.C:                                                      */
/*	  Programmrumpf fuer C-Programme mit dem Keil                   */
/*    Entwicklungsprogramm uVision fuer ARM-Mikrocontroller         */
/*                                                                  */
/********************************************************************/
/*  Aufgaben-Nr.:        *                                          */
/*                       *                                          */
/********************************************************************/
/*  Gruppen-Nr.: 	       *                                          */
/*                       *                                          */
/********************************************************************/
/*  Name / Matrikel-Nr.: *                                          */
/*                       *                                          */
/*                       *                                          */
/********************************************************************/
/* 	Abgabedatum:         *                                          */
/*                       *                                          */
/********************************************************************/

#include <LPC21xx.H>		/* LPC21xx Definitionen                     */


// Masken und Konstanten
#define SEGMENT_MASK 0x01FC0000					// 7 Segment Display P0.18-P0.24
#define LED_MASK 0xFF0000								// LEDS an P1.16 - P1.23
#define SWITCH_MASK 0x30000							// Schalter an P0.16 und P0.17
#define INPUT_MASK 0x3C00								// Eingabemaske für BCD
#define BCD_MAX 9												// Maximalwert der BCD-Anzeige
#define LED_MAX 0xFF										// Maximales LED-Muster
#define PERI_TAKT 12500000									// Takt der Peripherie Komponente

volatile unsigned int g_step = 0;

static const unsigned long BCD_Zahlen[10] = {
	0xFC0000, 							// Zahl 0
	0x180000, 							// Zahl 1
	0x16C0000, 							// Zahl 2
	0x13C0000, 							// Zahl 3
	0x1980000,							// Zahl 4
	0x1B40000, 							// Zahl 5
	0x1F40000, 							// Zahl 6
	0x1C0000, 							// Zahl 7
	0x1FC0000, 							// Zahl 8
	0x1BC0000								// Zahl 9
};


static const unsigned long delay_steps[10] = {
   2ul, 5ul, 10ul, 25ul, 50ul, 100ul, 250ul, 500ul, 750ul, 1000ul
};

//Funktionen

void initLED(void);											// LED Initialisierung
void initBCD(void);											// BCD-Anzeige Initialisierung
void updateBCD(unsigned int value);			// BCD-Anzeige Aktualsiieren
void updateLED(unsigned int pattern);		// LEDs aktualisieren
unsigned int readBCDInput(void);				// Eingabe einlesen
unsigned int readSwitchState(void);			// Schalter Status einlesen
void initTimer(volatile unsigned long loops);										// Timer initialisieren
void T0isr(void) __irq;									// Timer Interuppt Service Routine



void initLED(void){
	IODIR1 = LED_MASK;										// IODIR = IO Direction Register: Legt fest ob ein Pin Ein oder Ausgangs-Pin ist. Am Anfang P1.16-P1.23 als Ausgang definieren
	IOCLR1 = LED_MASK;										// IOCLR = IO Clear Registern: Setzt initial die LED auf 0
}

void initBCD(void){
	IODIR0 = SEGMENT_MASK;								// P0.18 - P0.24 als Ausgang definieren
	IOCLR0 = SEGMENT_MASK;								// BCD Anzeige initial löschen
}

unsigned int readBCDInput(void){
	return (IOPIN0 >> 10) & 0xF;					// IOPIN0 enthält logischen Zustand aller Pins auf Port 0. Dieser Zustand wird dann um 10 Stellen nach rechts geshiftet,
																				// Damit die Bits 10-13 an der Position "0-3" liegen. 
																				// 0xF brauchen wir, um sicherzustellen, dass nur die BDC Bits gesetzt sind und keine andere!
}

void updateLED(unsigned int pattern){   
		IOCLR1 = LED_MASK;									// Alle LEDs ausschalten
	IOSET1 = (pattern << 16);							// Eingegebene LED wird um 16 Stellen nach links geschoben, um die LEds die binäre Darstellung des BCDs Wert zu geben
}

void updateBCD(unsigned int value){
	if(value > BCD_MAX) value = BCD_MAX;	// Falls eingegebene Value >9 dann bleibt es bei 9
	
	IOCLR0 = SEGMENT_MASK;								// Löscht alle Segmente von 7-Segment Anzeige
	IOSET0 = BCD_Zahlen[value];						// BCD wird zu eingegebene Zahl eingesetzt
}


void initTimer(volatile unsigned long loops) {
	
	
	while(loops--) {
		volatile unsigned long x = 3000ul;
		while (x--) {}
	}
	

}


int main (void)  
{
	unsigned int bcd_value;
	int i;
	g_step = readBCDInput();
	
	if (g_step > 9) {
		g_step = 9;
	}
	initLED();
	initBCD();
	
	
 	/* while (1)  
	{
		bcd_value = readBCDInput();
		updateLED(bcd_value);
		updateBCD(bcd_value);
	} */

	
	while (1) {
		for (i = 0 ; i < 10; i++) {
			updateLED(i);
			initTimer(delay_steps[g_step]);
			updateBCD(g_step);
		
		}
	}
}
