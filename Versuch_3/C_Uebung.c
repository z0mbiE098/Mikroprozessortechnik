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

#define BUFFER_SIZE 10

void init_uart1(void);	
void uart1_putc(char c);	
char uart1_getc(void);
void uart1_puts(char *s);
unsigned long read_hex32(void);
unsigned char read_hex8(void);
void print_hex8(unsigned char val);
int hexCharToInt(char c);


int main (void) {
	char cmd;
	unsigned long addr;
	unsigned char data;
	unsigned char *ptr;
	int i;
	
	
	init_uart1();
	
	for(i = 0; i < 10; i++) {
		uart1_putc('0' + i);
	}
	
	while(1) {
		cmd = uart1_getc();
		uart1_putc(cmd); //echo
		
		if (cmd == 'D') {
			
			addr = read_hex32();
			ptr = (unsigned char *)addr;
			
			uart1_puts("\r\n");
			print_hex8((addr >> 24) & 0xFF);
			print_hex8((addr >> 16) & 0xFF);
			print_hex8((addr >> 8) & 0xFF);
			print_hex8(addr & 0xFF);
			uart1_puts(": ");
			print_hex8(*ptr);
			uart1_puts("\r\n");
		}
		
		if (cmd == 'E') {
            //uart1_getc(); // Leerzeichen

            addr = read_hex32();
            //uart1_getc(); // Leerzeichen
            data = read_hex8();

            ptr = (unsigned char *)addr;
            *ptr = data;

            uart1_puts("\r\n");
        }
	}
}


void init_uart1(void) {
	PINSEL0 |= 0x50000; // jeweils 2 Bit pro Pin; T1D und R1D belegen 0.8 und 0.9 und werden durch 01 initialisiert -> 0101 0000 0000 0000 0000
	U1LCR = 0x9F; // 8 Datenbits; keine Parität; 1 Stoppbit und DLAB für Baudrate aktiviert
	U1DLL = 41; // Divisor Latch Least Significant Byte: Divisor für Baudrate von 4800 bei 12,5 MHz = 163 = 0xA3 in Hex
	U1DLM = 0x00; // Divisor Latch Most Significant Byte: leer, da Divisor kleiner als 255 und somit in die ersten 8 Bit past
	U1LCR = 0x1F; // DLAB 'Divisor Latch Access Bit' wieder deaktiviert
	U1FCR = 0x07;
}


void uart1_putc(char c) {
    while (!(U1LSR & (0x20)));
    U1THR = c;
}


char uart1_getc(void) {
	while (!(U1LSR & 1));
	return U1RBR;
}

void uart1_puts(char *s) {
	while (*s) {
		uart1_putc(*s++);
	}
}


unsigned long read_hex32(void) {
    unsigned long value = 0;
    char c;
    int i;

    for (i = 0; i < 8; i++)
    {
        c = uart1_getc();
        uart1_putc(c); // Echo
        value = (value << 4) | hexCharToInt(c);
    }

    return value;
}


unsigned char read_hex8(void) {
    char c1, c2;

    c1 = uart1_getc(); uart1_putc(c1);
    c2 = uart1_getc(); uart1_putc(c2);

    return (hexCharToInt(c1) << 4) | hexCharToInt(c2);
}


void print_hex8(unsigned char val) {
    char hex[]="0123456789ABCDEF";

    uart1_putc(hex[val >> 4]);
    uart1_putc(hex[val & 0x0F]);
}


int hexCharToInt(char c) {
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;
	if(c >= 'a' && c <= 'c') return c - 'a' + 10;
	return 0;
}
