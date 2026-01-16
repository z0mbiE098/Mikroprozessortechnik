/********************************************************************/
/*  Hochschule fuer Technik und Wirtschaft                          */
/*  Fakultät fuer Ingenieurwissenschaften                           */
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
#define PERI_TAKT 12500000             // Takt der Peripherie Komponente

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

static const unsigned int time_periods[10] = {
    2, 5, 10, 25, 50, 100, 250, 500, 750, 1000
};

static volatile unsigned int led_pos = 0;      // Aktuelle Position Lauflicht (0..7)
static volatile unsigned int segment7_digit = 0;    // Aktuelle Zahl 7-Segment (0..9)

void initLED(void);
void initSegment7(void);
void updateSegment7(unsigned int value);
void updateLED(unsigned int pattern);
unsigned int readBCDInput(void);
unsigned int readSwitchState(void);
void initTimer(void);
void T0isr(void) __irq;


void initLED(void){
    IODIR1 |= LED_MASK;                // P1.16-P1.23 als Ausgang
    IOCLR1 = LED_MASK;                 // LEDs aus
}

void initSegment7(void){
    IODIR0 |= SEGMENT7_MASK;            // P0.18-P0.24 als Ausgang
    IOCLR0 = SEGMENT7_MASK;
}

unsigned int readBCDInput(void){
    return (IOPIN0 >> 10) & 0xF;
}

void updateLED(unsigned int pattern){
    IOCLR1 = LED_MASK;
    IOSET1 = (pattern << 16);
}

void updateSegment7(unsigned int value){
    if(value > BCD_MAX) value = BCD_MAX;
    IOCLR0 = SEGMENT7_MASK;
    IOSET0 = SEGMENT_Zahlen[value];
}

unsigned int readSwitchState(void){
    unsigned int s = 0;
    if ((IOPIN0 & (1U << 16)) == 0U) s |= 1U; // S1 gedrückt
    if ((IOPIN0 & (1U << 17)) == 0U) s |= 2U; // S2 gedrückt
    if ((IOPIN1 & (1U << 25)) == 0U) s |= 4U; // S3 gedrückt
    return s;
}

void initTimer(void){
	  T0TCR = 0x00;
    T0TCR = 0x02;                      // Timer Reset
    T0PR  = (PERI_TAKT / 1000) - 1;    // Prescaler für 1 ms Auflösung
    T0MR0 = 1000;                      // Default: 1000 ms = 1 s
    T0MCR = 0x03;                      // Interrupt + Reset bei Match MR0

    VICVectAddr4 = (unsigned long)T0isr; // Adresse der ISR
    VICVectCntl4 = 0x20 | 4;             // Slot enable + Kanal 4 (Timer0)
    VICIntEnable = (1 << 4);             // Interrupt enable für Kanal 4

    T0TCR = 0x01;                      // Timer Start
}


void T0isr(void) __irq{
    unsigned int sw = readSwitchState();
    unsigned int bcd_in;
	  // unsigned int led_pos = 0;      // Aktuelle Position Lauflicht (0..7)
    // unsigned int segment7_digit = 0;    // Aktuelle Zahl 7-Segment (0..9)
	


    bcd_in = readBCDInput();
    if (bcd_in > 9) bcd_in = 9;

    if (T0MR0 != time_periods[bcd_in]) {
          T0TCR = 0;                        // anhalten
					T0MR0 = time_periods[bcd_in];     // aktualsieren
			    T0MCR = 0x03;
					T0TCR = 2;                        // reset
					T0TCR = 1;                        // start
			     T0TC = 0;                        // counter reset
			   

        
    }

    

    if ((sw & 1U) != 0U)              // s1 (0.16) resetten alle leds und segmente
    {
        updateLED(0);
        IOCLR0 = SEGMENT7_MASK; 
        led_pos = 0;
        segment7_digit = 0;
    }
    else
    {

        if ((sw & 2U) != 0U)
        {
            updateLED(1U << led_pos);
            led_pos = (led_pos + 1U) % 8U; // Vorwärts 0..7
        }
        else
        {
            updateLED(1U << led_pos);
            if (led_pos == 0U) led_pos = 7U; // Rückwärts
            else led_pos--;
        }

        if ((sw & 4U) != 0U)
        {
            updateSegment7(segment7_digit);
            segment7_digit++;
            if (segment7_digit > 9U) segment7_digit = 0U;
        }
        else
        {
            updateSegment7(segment7_digit);
            if (segment7_digit == 0U) segment7_digit = 9U;
            else segment7_digit--;
        }
    }

    T0IR = 0x01;            // Lösche Interrupt-Flag im Timer
    VICVectAddr = 0x00;     // Signalisiere VIC, dass ISR fertig ist
}


int main (void)
{
	  initLED();
    initSegment7();
    updateLED(0);
    IOCLR0 = SEGMENT7_MASK;
    initTimer();

    while (1)
    {
        
    }
}
