/********************************************************************/
/*  Hochschule fuer Technik und Wirtschaft                          */
/*  Fakult?t fuer Ingenieurwissenschaften                           */
/*  Labor fuer Eingebettete Systeme                                 */
/*  Mikroprozessortechnik                                           */
/********************************************************************/
/*                                                                  */
/*  C_Uebung.C:                                                      */
/*	  Programmrumpf fuer C-Programme mit dem Keil                   */
/*    Entwicklungsprogramm uVision fuer ARM-Mikrocontroller         */
/*                                                                  */
/********************************************************************/
/*  Aufgaben-Nr.:        *  Versuch 3: UART Schnittstellen          */
/*                       *                                          */
/********************************************************************/
/*  Gruppen-Nr.: 	       *  Gruppe 1. Am Freitag 3. Stunde          */
/*                       *                                          */
/********************************************************************/
/*  Name / Matrikel-Nr.: * Hanan Ahmed Ashir, 5012967               */
/*                       * Erwin Holzhauser, 5013983                */
/*                       *                                          */
/********************************************************************/
/* 	Abgabedatum:         * 30.01.2026                               */
/*                       *                                          */
/********************************************************************/

#include <LPC21xx.H>		/* LPC21xx Definitionen                     */


#define LCRMASK_MIT_DLAB 			(1U <<7)						//Aktiviert das 7.Bit in UxLCR. Aktiviert das DLAB bit, damit man Baudrate einstelln kann
#define LCRMASK_OHNE_DLAB 		0x7F								// 0x7F=0111 1111. DLAb wird gelöscht, alles anderes bleibt unverändert
#define PINSEL_UART1 					0x50000							// P.08 und P0.9 als UART pins einstelln
#define FCR_MASK							0x07								// 0x07 = 0000 0111 --> Bit0 = FIFO Aktiv, Bit1 = Empfangs-FIFO reset, Bit2 = Sende-FIFO reset
#define P_CLOCK								12500000UL					// Bei der Berechnung kann es zu Variablenungleichheiten kommen. Deswegen mit unsigned Long einigen
#define THRE_MASK	 						0x20								// 0x20 = 0010 0000. Bedeutet U1THR ist leer, man darf schreiben
#define RDR_MASK							0x1									// 0x1 = 0000 0001. bedeutet Empfangsregister enthält Daten, man kann U1RBR lesen.

void init_uart1(unsigned long baudrate, unsigned long dataBits, unsigned long stopBits, unsigned long parityType); //Initialisierung der Schnittstelle UART1
void uart1_sendCharacter(char c);		// Sendet genau ein Zeichen über UART1
char uart1_receiveCharacter(void);	// Empfängt genau ein zeichen über UART1
void uart1_sendString(char *s);			// Sendet einen kompletten String über UART1
unsigned long read_hex32(void);			// Liest eine 32-Bit Hex-Zahl aus der seriellen Eingabe
unsigned char read_hex8(void);			// Liest eine 8-Bit Hex-Zahl aus der seriellen Eingabe
void print_hex8(unsigned char val);	// Gibt ein Byte als zwei Hex-Zeichen aus
int hexCharToInt(char c);						// Wandelt ein einzelnes Hex-Zeichen in Zahlenwert um


void init_uart1(unsigned long baudrate, unsigned long dataBits, unsigned long stopBits, unsigned long parityType){				
			PINSEL0 |= PINSEL_UART1;				// Die Pinks p0.8 und P0.9 auf UART1 Funktionen setzen. != Setzt bits, ohne die zu löschen
			U1LCR |= (dataBits -5);					// Bits 0-1 werden gesetzt
			U1LCR |= (stopBits -1) <<2;			// Auf den Bit2 Stops bits liegen. 
			U1LCR |= LCRMASK_MIT_DLAB;			// Setzt DLAB-Bit auf 1, damit DLL umd DLM als Baudratenregistern zugreifbar sind
			
			if(parityType == 0){						// if 0, then keine Parität
					U1LCR |= (0<<3);							
			} else {
					U1LCR |= (1<<3);						// ParitätBit aktivieren
					U1LCR |= (parityType -1) << 4;		//  Parity Type auswählen. 1 = 0dd, 2 = Even
			}		
			
			U1DLL = ((P_CLOCK /(16*baudrate)) % 256); // Low Byte(8-Bit wird hier gespeichert)
			U1DLM = ((P_CLOCK /(16*baudrate)) / 256); // High-Byte(obere 8-Bits werden hier gespeichert)
			U1LCR &= LCRMASK_OHNE_DLAB;								// DLAb wieder ausschalten, damit UART normal arbeitet
			U1FCR = FCR_MASK;  												// FIFO Control register einschalten und resetten 
}

void uart1_sendCharacter(char c){
	while(!(U1LSR & (THRE_MASK)));						// Solange bit5=0(THR) noch nicht leer --> warten		
	U1THR = c;																// Wenn Bit5=1, dann Zeichen senden
}

char uart1_receiveCharacter(void){
	while (!(U1LSR & RDR_MASK)); 							// Bit0(receiver Data ready) prüfen und warten bis Daten da sind.  
	return U1RBR;															// Das empfangene Zeichen kommt zurück
}

void uart1_sendString(char *s) {
	while (*s) {															// Solange aktuelle Zeichen nicht Nullterminator ist
		uart1_sendCharacter(*s++);							// Das aktuelle Zeichen senden und Pointer erhöhen
	}
}

unsigned long read_hex32(void) {						
    unsigned long value = 0;								// Sammelt die 32-Bit gelesene Zahl
    char c;																	// aktuelle Zeichen
    int i;																	// zählt bis 8 Hex zeichen, 32bit

    for (i = 0; i < 8; i++) {								//maximal 8 Zeichen lese
        c = uart1_receiveCharacter();					// Liest das Zeichen vom Hyperterminal
        uart1_sendCharacter(c); 							// Zeigt das Zeichen, damit man sieht was eingegeben wurde
			if (c == 0x0D) {											// Wenn Enter(Ascii: 0x20) gedrückt wurde
					return value;											// Dann aktuelle zahl zurückgeben
				}
        value = (value << 4) | hexCharToInt(c);	// Das aktuelle zahl um 4 bits nach links schicken und unwandeln. 
    }
    return value;															// Nach 8 bits ganze zahl zurückgeben
}

unsigned char read_hex8(void) {								// Liest 2 Hexadezimalzahlen (bei E)
    char c1, c2;

    c1 = uart1_receiveCharacter(); 
		uart1_sendCharacter(c1);			// Erstes Hex Zeichen lesen und zeigen
    c2 = uart1_receiveCharacter(); 
		uart1_sendCharacter(c2);			// Zweites Hex Zeichen lesen und zeigen
    return (hexCharToInt(c1) << 4) | hexCharToInt(c2);	// Erstes Zeichen an Position 1 und zweite an Position 0
}

void print_hex8(unsigned char val) {
    char hex[]="0123456789ABCDEF";		// Lookup Tabelle_ 
    uart1_sendCharacter(hex[val >> 4]);		// sendet erstes zeichen 
    uart1_sendCharacter(hex[val & 0x0F]);		// sendet zweites Hex zeichen
}

int hexCharToInt(char c) {
	if(c >= '0' && c <= '9') return c - '0';				// Wandelt ASCII Zeichen in Zahlen. 0-9 bleibt 0-9
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;		// A-F werden 10-15 dargestellt
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;		// dasselbe
	return 0;																				// falls keine hexadezimal, dann einfach 0 Ausgeben
}


int main (void) {
	char command;																		// Command D/E
	unsigned long address;													// 32-Bit Adresse
	unsigned char data;															// 1 Byte Daten
	unsigned char *ptr;															// Pointer auf die Adresse
	int i;																					// Schleife für Testausgabe
	
	init_uart1(19200, 8, 2, 2);
	
	for(i = 0; i <10; i++){
		uart1_sendCharacter('0' + i);
	}
	uart1_sendString("\r\n");												// Neue Zeile
	
	while(1) {
			command = uart1_receiveCharacter();					// Liest Kommando und 
			uart1_sendCharacter(command);									// zeigt an dem Terminal
			
		while(command != 'D' && command != 'E' && command != 'd' && command != 'e'){ //Solange nicht d/e --> ungültige Eingabe
				uart1_sendString("Bitte nur D/d oder E/e eingeben!\r\n");
				command = uart1_receiveCharacter();					// Neues kommando einlesen
				uart1_sendCharacter(command);								// An dem Terminal zeigen
		};
			
			uart1_receiveCharacter();											// Erwartet eine Leerzeichen nach dem Kommando und konsumiert es
			uart1_sendCharacter(0x20);										// Eine Leerzeichen am Terminal zeigen
			
			if(command == 'D' || command == 'd'){
				address = read_hex32();											// 32-Bit Adresse lesen. Adresse zb 0x12 34 56 78. Jedes Byte wird einzeln rausgeholt und mit print_hex8 ausgegebeb
				ptr =(unsigned char *)address;							// mach aus der Zahl eine Adresse, pointer auf byte
				uart1_sendString("\r\n");										// Neue Zeile
				print_hex8((address >> 24) & 0xFF);					// Die oberem 8-bit nach ganz unten schicken. &Ff alles außer die unteren 8 bits wird gelöscht. (0x12)
				print_hex8((address >> 16) & 0xFF);					// Zweites Byte von oben. gibt jetzt 0x34
				print_hex8((address >> 8) & 0xFF);					// 3tes Byte. Gibt 0x56
				print_hex8(address & 0xFF);									// letztes byte. gibt 0x78. Das heisst es wird: 0x12, 0x34, 0x56, 0x78b geprintet
				uart1_sendString(": ");											// Trenner
				print_hex8(*ptr);														// Liest das Byte an address und gibt es als 2 Hex-zeichen aus.
				uart1_sendString("\r\n");
			}
			
			if(command == 'E' || command == 'e'){
				address = read_hex32();								
				uart1_receiveCharacter();										// Leerzeichen konsumieren und echo
				uart1_sendCharacter(0x20);									
				data = read_hex8();													// Ein Byte Daten lesen(2 Hexa Zeichen)
				
				ptr = (unsigned char *)address;							// Pointer setzen 
				*ptr = data;																// Byte in den Speicher schreiben
				uart1_sendString("\r\n");										// Neue Zeile
			}		
	}
}
