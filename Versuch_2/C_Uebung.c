/********************************************************************/
/*  Hochschule fuer Technik und Wirtschaft                          */
/*  Fakult?t fuer Ingenieurwissenschaften                           */
/*  Labor fuer Eingebettete Systeme                                 */
/*  Mikroprozessortechnik                                           */
/********************************************************************/
/*                                                                  */
/*  C_Uebung.C:                                                     */
/*	  Programmrumpf fuer C-Programme mit dem Keil                   */
/*    Entwicklungsprogramm uVision fuer ARM-Mikrocontroller         */
/*                                                                  */
/********************************************************************/
/*  Aufgaben-Nr.:        *   Versuch_2:                             */
/*                       *                                          */
/********************************************************************/
/*  Gruppen-Nr.: 	       *   Gruppe 1, Freitag 3.Stunde             */
/*                       *                                          */
/********************************************************************/
/*  Name / Matrikel-Nr.: *   Hanan Ahmed Ashir, 5012967             */
/*                       *   Erwin Holzhauser, 5013983              */
/*                       *                                          */
/********************************************************************/
/* 	Abgabedatum:         *   16.01.2026                             */
/*                       *                                          */
/********************************************************************/

#include <LPC21xx.H>        /* LPC21xx Definitionen                     */

// Masken und Konstanten
#define SEGMENT7_MASK 0x01FC0000        // 7 Segment Display P0.18-P0.24
#define LED_MASK 0xFF0000              // LEDS an P1.16 - P1.23
#define SWITCH_MASK 0x30000            // Schalter an P0.16 und P0.17 (S1,S2)
#define BCD_MAX 9                      // Maximalwert der BCD-Anzeige

//static volatile unsigned int led_pos = 0;      // Aktuelle Position Lauflicht (0..7)
//static volatile unsigned int segment7_digit = 0;    // Aktuelle Zahl 7-Segment (0..9)

// Lookup-Tabellen
static const unsigned long SEGMENT_Zahlen[10] = {
    0xFC0000,     // 0
    0x180000,     // 1
    0x16C0000,    // 2
    0x13C0000,    // 3
    0x1980000,    // 4
    0x1B40000,    // 5
    0x1F40000,    // 6
    0x1C0000,     // 7
    0x1FC0000,    // 8
    0x1BC0000     // 9
};

static const unsigned int time_periods[10] = {			//�ber BCD-Schalter gesteuerte Zeitintervalle
    2, 5, 10, 25, 50, 100, 250, 500, 750, 1000
};



void initLED(void);
void initSegment7(void);
void updateSegment7(unsigned int value);
void updateLED(unsigned int pattern);
unsigned int readBCDInput(void);
unsigned int readSwitchState(void);
void initTimer(void);
void T0isr(void) __irq;


void initLED(void){										 // Initialisiert die LED-PINS als Ausgang
    IODIR1 |= LED_MASK;                // P1.16-P1.23 als Ausgang. |= Stellt sicher, dass nur die Bits von LEDs als Ausgang definiert sind, und nicht andere
    IOCLR1 = LED_MASK;                 // LEDs aus
}

void initSegment7(void){								// Initilialisiert das 7-Segment-Display als Ausgang
    IODIR0 |= SEGMENT7_MASK;            // P0.18-P0.24 als Ausgang. Auch hier nur die Bits von 7Segment �berschreiben, keine anderen!
    IOCLR0 = SEGMENT7_MASK;							// Segment leer am Anfang
}

unsigned int readBCDInput(void){				// Liest den Wert eines BCD-Codierschalters an P.010 - P0.13 ein.
    return (IOPIN0 >> 10) & 0xF;				// schieb die bits um 10 Stellen nach rechts, damit steht P0.10 an Bit0. Mit & 0xF bleiben die untere 4 Bits �brig.
}

void updateLED(unsigned int pattern){		// Aktualisiert das Muster an Port 1
    IOCLR1 = LED_MASK;									// Erstmal alle Bits aus
    IOSET1 = (pattern << 16);						// Dann die Bits ab Bit 16 laut pattern setzen
}

void updateSegment7(unsigned int value){		// Zeigt eine Zahl 0-9 auf das Segment
    if(value > BCD_MAX) value = BCD_MAX;		// Array �berlauf schutz, Falls value>9, dann auf 9 beschr�nken
    IOCLR0 = SEGMENT7_MASK;									// Segment leer lassen
    IOSET0 = SEGMENT_Zahlen[value];					// Bitmuster aus Tabelle an Port 0 schreiben
}

unsigned int readSwitchState(void){					// liest den Status der Schalter S1, S2(Port 0) und S3(Port 1)
    unsigned int s = 0;
    if ((IOPIN0 & (1U << 16)) == 0U) s |= 1U; // IoPin0& eine Zahl, wobei nur das 16.Bit 1 ist. Falls Taste gedr�ckt wird, steht an 16. Stelle eine 0 und das Ergebnis ist dann 0.
																							// s |= 1U sagt an der 0te Stelle von s eine 1 schreiben. Damit S1 und S2 gleichzeitig funktionieren k�nnen, bracuhen wir |=
    if ((IOPIN0 & (1U << 17)) == 0U) s |= 2U;
    if ((IOPIN1 & (1U << 25)) == 0U) s |= 4U;
    return s;
}

void initTimer(void){										// Timer0 konfigurieren f�r periodische Interrupts
	  T0TCR = 0x00;												// Timer Stoppen
    T0TCR = 0x02;                       // Timer Reset
    T0PR  = 12499;    									// Prescaler, teilt Takt, damit Timer alle 1ms tickt. Prescaler = (P-Clock / 1000Hz) - 1 = (12.500.000/1000) - 1 = 12.499
    T0MR0 = 1000;                      	// Default: 1000 ms = 1s. Nach jedem Sekunde wird der Controller den (aktuellen Z�hlerstand) T0TC mit T0MR0 vergleichen.
    T0MCR = 0x03;                      	// Bei einem Match --> Interrupt ausl�sen und Reset

    VICVectAddr4 = (unsigned long)T0isr; // Adresse der Funktion f�r Slot 4 festlegen
    VICVectCntl4 = 0x20 | 4;             // Slot enable + Timer0 Kanal 4 zuweisen
    VICIntEnable = (1 << 4);             // Interrupt enable global f�r Timer0 einschalten

    T0TCR = 0x01;                      // Timer Start
}


void T0isr(void) __irq{
    unsigned int sw = readSwitchState();
    unsigned int bcd_in = readBCDInput();
	  static unsigned int led_pos = 0;      // Aktuelle Position Lauflicht (0..7)
    static unsigned int segment7_digit = 0;    // Aktuelle Zahl 7-Segment (0..9)
	


    bcd_in = readBCDInput();
    if (bcd_in > 9) bcd_in = 9;

    if (T0MR0 != time_periods[bcd_in]) {			// Gucken ob die eingestellte Zeit in T0MR0 immer noch dieselbe ist, falls nicht, dann kurz aktualisiern
          T0TCR = 0x00;                        // Anhalten, damit keine neue Signale auftauchen k�nnen
					T0MR0 = time_periods[bcd_in];     		// Timer aktualisieren.
			    T0MCR = 0x03;													// Wenn Ziel erreicht, dann Interrupt ausl�sen und sich auf NULL Setzen.
					T0TCR = 0x02;                        // reset.
																							// Notwendig wenn wir zb von 500ms auf 25ms kommen. Dann braucht TC bis zum maximalen Wert kommen. Das dauert halt so lange deswegen resetten
					T0TCR = 0x01;                        // start mit neuem Speed
			    T0TC = 0x00;                        // counter reset
   }

	 if ((sw & 1U) != 0U){              // Logik F�r Taste S1: Pr�fe ob Bit 0 in sw gestezt ist. Ist das unterste Bit eine 1? falls ja setzt alles auf NULL
        updateLED(0);									// Alle LEDs sind aus
        IOCLR0 = SEGMENT7_MASK; 			// 7-Segment auch komplett aus
        led_pos = 0;									// Z�hler f�r Lauflicht auf 0
        segment7_digit = 0;						// Z�hler f�r Segment auf 0
    } else {

        if ((sw & 2U) != 0U) {				// S2 gedr�ckt?
            updateLED(1U << led_pos);	//Schalte Bit an der aktuelle Position an
            led_pos = (led_pos + 1U) % 8U; // gehe eine Position weiter und modulo 8 damit alles in 0-7 Bereich bleibt
        } else {											//S2 nicht gedr�ckt
            updateLED(1U << led_pos);
            if (led_pos == 0U) led_pos = 7U; // R�ckw�rts. Falls 0 erreicht ist, die letzte Stelle gehen,
            else led_pos--;										// ansonsten immer dekrementieren
        }
					if ((sw & 4U) != 0U) {				// S3 gedr�ckt?
            updateSegment7(segment7_digit);	// Aktuelle zahl anzeigen
            segment7_digit++;								// Hochz�hlen
            if (segment7_digit > 9U) segment7_digit = 0U;	// Nach 9 wieder auf 0 setzten
        } else {
            updateSegment7(segment7_digit);	//Falls S3 nicht gedr�ckt,
            if (segment7_digit == 0U) segment7_digit = 9U;	// Von Null Laufen bis 9
            else segment7_digit--;													// R�ckw�rts
        }
    }

    T0IR = 0x01;            // L�sche Interrupt-Flag im Timer
    VICVectAddr = 0x00;     // Signalisiere VIC, dass ISR fertig ist
}


int main (void) {
	  initLED();
    initSegment7();
    updateLED(0);
    IOCLR0 = SEGMENT7_MASK;
    initTimer();		//Timers und Interrupts starten

    while (1)
    {
        //
    }
}
