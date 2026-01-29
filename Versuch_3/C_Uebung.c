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

#define PINSEL_UART1 					0x50000
#define LCRMASK_MIT_DLAB 			0x9F
#define LCRMASK_OHNE_DLAB 		(0<<7)
#define FCR_MASK							0x07
#define P_CLOCK								12500000UL
#define THRE_MASK	 						0x20
#define RDR_MASK							0x1

void init_uart1(unsigned long baudrate, unsigned long dataBits, unsigned long stopBits, unsigned long parityType);							//Initialisierung der Schnittstelle UART1
void uart1_sendCharacter(char c);		// Sendet genau ein Zeichen über UART1
char uart1_receiveCharacter(void);	// Empfängt genau ein zeichen über UART1
void uart1_sendString(char *s);			// Sendet einen kompletten String über UART1
unsigned long read_hex32(void);			// Liest eine 32-Bit Hex-Zahl aus der seriellen Eingabe
unsigned char read_hex8(void);			// Liest eine 8-Bit Hex-Zahl aus der seriellen Eingabe
void print_hex8(unsigned char val);	// Gibt ein Byte als zwei Hex-Zeichen aus
int hexCharToInt(char c);						// Wandelt ein einzelnes Hex-Zeichen in Zahlenwert um


void init_uart1(unsigned long baudrate, unsigned long dataBits, unsigned long stopBits, unsigned long parityType){
			unsigned baudRate_LOW = ((P_CLOCK /(16*baudrate)) % 256);
			unsigned baudRate_HIGH = ((P_CLOCK /(16*baudrate)) / 256);
	
			PINSEL0 |= PINSEL_UART1;
			U1LCR &= LCRMASK_MIT_DLAB;
			U1LCR |= (dataBits -5);
			U1LCR |= (stopBits -1) <<2;
			if(parityType == 0){
					U1LCR |= (0<<3);
			} else {
					U1LCR |= (1<<3);
					U1LCR |= (parityType -1) << 4;
			}			
			U1DLL = baudRate_LOW;
			U1DLM = baudRate_HIGH;
			U1LCR &= LCRMASK_OHNE_DLAB;
			U1FCR = FCR_MASK;
}

void uart1_sendCharacter(char c){
	while(!(U1LSR & (THRE_MASK)));
	U1THR = c;
}

char uart1_receiveCharacter(void){
	while (!(U1LSR & RDR_MASK)); // !!!
	return U1RBR;
}

void uart1_sendString(char *s) {
	while (*s) {
		uart1_sendCharacter(*s++);
	}
}

unsigned long read_hex32(void) {
    unsigned long value = 0;
    char c;
    int i;

    for (i = 0; i < 8; i++) {
        c = uart1_receiveCharacter();
        uart1_sendCharacter(c); // Echo
				if (c == 0x0D) {
					return value;
				}
        value = (value << 4) | hexCharToInt(c);
    }
    return value;
}

unsigned char read_hex8(void) {
    char c1, c2;

    c1 = uart1_receiveCharacter(); uart1_sendCharacter(c1);
    c2 = uart1_receiveCharacter(); uart1_sendCharacter(c2);

    return (hexCharToInt(c1) << 4) | hexCharToInt(c2);
}

void print_hex8(unsigned char val) {
    char hex[]="0123456789ABCDEF";

    uart1_sendCharacter(hex[val >> 4]);
    uart1_sendCharacter(hex[val & 0x0F]);
}

int hexCharToInt(char c) {
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	return 0;
}


int main (void) {
	char command;
	unsigned long address;
	unsigned char data;
	unsigned char *ptr;
	int i;
	
	init_uart1(19200, 8, 2, 2);
	for(i = 0; i <10; i++){
		uart1_sendCharacter('0' + i);
	}
	uart1_sendString("\r\n");
	
	while(1) {
			command = uart1_receiveCharacter();
			while(command != 'D' && command != 'E' && command != 'd' && command != 'e'){
				uart1_sendString("Bitte nur D/d oder E/e eingeben!\r\n");
				command = uart1_receiveCharacter();
				uart1_sendCharacter(command);
			};
			
			uart1_receiveCharacter();
			uart1_sendCharacter(0x20);
			
			if(command == 'D' || command == 'd'){
				address = read_hex32();
				ptr =(unsigned char *)address;
				uart1_sendString("\r\n");
				print_hex8((address >> 24) & 0xFF);
				print_hex8((address >> 16) & 0xFF);
				print_hex8((address >> 8) & 0xFF);
				print_hex8(address & 0xFF);
				uart1_sendString(": ");
				print_hex8(*ptr);
				uart1_sendString("\r\n");
			}
			
			if(command == 'E' || command == 'e'){
				address = read_hex32();
				uart1_receiveCharacter();
				uart1_sendCharacter(0x20);
				data = read_hex8();
				
				ptr = (unsigned char *)address;
				*ptr = data;
				uart1_sendString("\r\n");
			}		
	}
}
