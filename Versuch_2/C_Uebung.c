/********************************************************************/
/*  Hochschule fuer Technik und Wirtschaft                          */
/*  Fakult?t fuer Ingenieurwissenschaften                           */
/*  Labor fuer Eingebettete Systeme                                 */
/*  Mikroprozessortechnik                                           */
/********************************************************************/

#include <LPC21xx.H>		/* LPC21xx Definitionen                     */

// Masken und Konstanten
#define SEGMENT_MASK 0x01FC0000					// 7 Segment Display P0.18-P0.24
#define LED_MASK 0xFF0000								// LEDS an P1.16 - P1.23
#define SWITCH_MASK 0x30000							// Schalter an P0.16 und P0.17 (S1,S2)
#define INPUT_MASK 0x3C00								// Eingabemaske f?r BCD
#define BCD_MAX 9												// Maximalwert der BCD-Anzeige
#define LED_MAX 0xFF										// Maximales LED-Muster
#define PERI_TAKT 12500000								// Takt der Peripherie Komponente

#define S3_MASK (1U << 25)							// S3 an P1.25

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

static const unsigned int time_periods[10] = {
	2, 5, 10, 25, 50, 100, 250, 500, 750, 1000
};
	

// Funktionen
void initLED(void);											// LED Initialisierung
void initBCD(void);											// BCD-Anzeige Initialisierung
void updateBCD(unsigned int value);			// BCD-Anzeige Aktualsiieren
void updateLED(unsigned int pattern);		// LEDs aktualisieren
unsigned int readBCDInput(void);				// Eingabe einlesen
unsigned int readSwitchState(void);			// Schalter Status einlesen (S1,S2,S3)
void initTimer(void);										// Timer initialisieren
void T0isr(void) __irq;									// Timer Interrupt Service Routine

// ---- Neu für Aufgabe 3: Tick-Flag ----
static volatile unsigned int g_tick_1s = 0;


// Implementierungen
void initLED(void){
	IODIR1 = LED_MASK;										// P1.16-P1.23 als Ausgang definieren
	IOCLR1 = LED_MASK;										// alle LEDs aus
}

void initBCD(void){
	IODIR0 = SEGMENT_MASK;								// P0.18 - P0.24 als Ausgang definieren
	IOCLR0 = SEGMENT_MASK;
}

unsigned int readBCDInput(void){
	return (IOPIN0 >> 10) & 0xF;
}

void updateLED(unsigned int pattern){
	IOCLR1 = LED_MASK;									// alle LEDs aus
	IOSET1 = (pattern << 16);						// pattern Bit0 -> LED an P1.16
}

void updateBCD(unsigned int value){
	if(value > BCD_MAX) value = BCD_MAX;
	IOCLR0 = SEGMENT_MASK;
	IOSET0 = BCD_Zahlen[value];
}


void setTimerPeriod(unsigned int ms){
	if(ms == 0){
		T0TCR = 0x02;
		T0MR0 = ms;
		T0TCR = 0x01;
	}
}



// Rückgabe-Bits: bit0=S1, bit1=S2, bit2=S3
// ACTIVE-LOW: Schalter "1" => Pin = 0
unsigned int readSwitchState(void){
    unsigned int s = 0;

    // S1 an P0.16
    if ((IOPIN0 & (1U << 16)) == 0U) s |= 1U;

    // S2 an P0.17
    if ((IOPIN0 & (1U << 17)) == 0U) s |= 2U;

    // S3 an P1.25
    if ((IOPIN1 & (1U << 25)) == 0U) s |= 4U;

    return s;
}


void initTimer(void){
	T0TCR = 0x02;													// reset/stop
	T0PR  = (PERI_TAKT / 1000) - 1;				// 1 ms Tick
	T0MR0 = 1000;													// 1000 ms = 1 s
	T0MCR = 0x03;													// interrupt + reset on MR0

	VICVectAddr4 = (unsigned long)T0isr;
	VICVectCntl4 = 0x20 | 4;								// enable slot + Timer0 channel
	VICIntEnable = (1 << 4);								// enable Timer0 interrupt

	T0TCR = 0x01;													// start
}

// ---- Geändert für Aufgabe 3: ISR setzt nur Tick-Flag ----
void T0isr(void) __irq{
	g_tick_1s = 1;												// 1s Tick melden

	T0IR = 0x01;													// interrupt quittieren
	VICVectAddr = 0x00;										// VIC quittieren
}

int main (void)
{
	unsigned int sw;
	unsigned int led_pos = 0;								// 0..7
	unsigned int bcd_digit = 0;			// 0..9
	unsigned int bcd = readBCDInput();
	
	PINSEL1 &= ~((3U << 0) | (3U << 2));

	initLED();
	initBCD();
	initTimer();

	// Startzustand: alles aus
	updateLED(0);
	IOCLR0 = SEGMENT_MASK;

 	while (1)
	{
		
		// --- NEU EINGEFÜGT: Zeitbasis anpassen ---
        unsigned int bcd_in = readBCDInput();
        
        // Sicherheitsbegrenzung auf 9, falls der Schalter auf A-F steht
        if (bcd_in > 9) bcd_in = 9; 

        // Prüfen, ob der Timer-Wert angepasst werden muss
        if (T0MR0 != time_periods[bcd_in]) 
        {
            T0MR0 = time_periods[bcd_in]; // Neuen Endwert setzen
            
            // WICHTIG: Falls der aktuelle Zähler (T0TC) schon größer ist als 
            // der neue Zielwert (T0MR0), würde der Timer bis zum Überlauf 
            // weiterlaufen (ca. 7 Minuten!). Deshalb: Zähler resetten.
            if (T0TC >= T0MR0) 
            {
                T0TC = 0;
            }
        }
		
		
		if (g_tick_1s)											// alle 1 Sekunde
		{
			g_tick_1s = 0;
			sw = readSwitchState();

			// S1 = 0 => Standby
			if ((sw & 1U) != 0U)
			{
				updateLED(0);
				IOCLR0 = SEGMENT_MASK;
				led_pos = 0;
				bcd_digit = 0;
				continue;
			}

			// S1 = 1 => Lauflicht je nach S2
			if ((sw & 2U) != 0U)								// S2=0 vorwärts
			{
				updateLED(1U << led_pos);
				led_pos = (led_pos + 1U) % 8U;
			}
			else																	// S2=1 rückwärts
			{
				updateLED(1U << led_pos);
				if (led_pos == 0U) led_pos = 7U;
				else led_pos--;
			}

			// 7-Segment: nur wenn S3 = 0 zählen, sonst aus
			if ((sw & 4U) != 0U)								// S3=0 zählen
			{
				updateBCD(bcd_digit);
				bcd_digit++;
				if (bcd_digit > 9U) bcd_digit = 0U;
			}
			else																	// S3=1 aus
			{
				updateBCD(bcd_digit);
				if (bcd_digit == 0U) bcd_digit = 9U;
				else bcd_digit--;							// beim nächsten Aktivieren wieder ab 0
			}
			
		}
	}

}