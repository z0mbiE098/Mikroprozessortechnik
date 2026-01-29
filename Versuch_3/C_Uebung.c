/********************************************************************/
/*  Hochschule fuer Technik und Wirtschaft                          */
/*  Fakultät fuer Ingenieurwissenschaften                           */
/*  Labor fuer Eingebettete Systeme                                 */
/*  Mikroprozessortechnik                                           */
/********************************************************************/
/*                                                                  */
/*  C_Übung.C:                                                      */
/*    Programmrumpf fuer C-Programme mit dem Keil                   */
/*    Entwicklungsprogramm uVision fuer ARM-Mikrocontroller         */
/*                                                                  */
/********************************************************************/
/*  Aufgaben-Nr.:        *                                          */
/*                       *                                          */
/********************************************************************/
/*  Gruppen-Nr.:         *                                          */
/*                       *                                          */
/********************************************************************/
/*  Name / Matrikel-Nr.: *                                          */
/*                       *                                          */
/*                       *                                          */
/********************************************************************/
/*  Abgabedatum:         *                                          */
/*                       *                                          */
/********************************************************************/

#include <LPC21xx.H>     /* LPC21xx Definitionen                     */

#define LINE_BUF_SIZE 64

void init_uart1(void);
void uart1_putc(char c);
char uart1_getc(void);
void uart1_puts(const char *s);

void print_hex8(unsigned char val);

/* ---------- Helfer: line-based input + hex parsing ---------- */

static int hex_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static int parse_hex_n(const char *s, int n, unsigned long *out) {
    unsigned long v = 0;
    int i, h;

    for (i = 0; i < n; i++) {
        h = hex_nibble(s[i]);
        if (h < 0) return 0;
        v = (v << 4) | (unsigned long)h;
    }
    *out = v;
    return 1;
}

/* Liest eine komplette Zeile bis ENTER.
   - Echo jedes Zeichens
   - ENTER -> sendet automatisch "\r\n"
   - Backspace wird unterstützt
   Rückgabe: Länge der Zeile (ohne Terminator) */
static int read_line(char *buf, int maxLen) {
    int len = 0;
    char c;

    while (1) {
        c = uart1_getc();

        /* ENTER (je nach Terminal \r oder \n) */
        if (c == '\r' || c == '\n') {
            uart1_puts("\r\n");      /* ENTER -> Zeilenumbruch */
            buf[len] = '\0';
            return len;
        }

        /* Backspace (optional, aber praktisch) */
        if (c == '\b' || c == 0x7F) {
            if (len > 0) {
                len--;
                uart1_puts("\b \b");
            }
            continue;
        }

        /* Normale Zeichen */
        if (len < maxLen - 1) {
            buf[len++] = c;
            uart1_putc(c);           /* Echo */
        }
    }
}

/* ---------------------------- MAIN ---------------------------- */

int main(void) {
    char line[LINE_BUF_SIZE];
    unsigned long addr, tmp;
    unsigned char data;
    volatile unsigned char *ptr;
    int i;

    init_uart1();

    /* 1–9 zeilenweise ausgeben */
    for (i = 1; i <= 9; i++) {
        uart1_putc('0' + i);
        uart1_puts("\r\n");
    }

    uart1_puts("CMD: D <8hex>  |  E <8hex> <2hex>\r\n");

    while (1) {
        int n = read_line(line, LINE_BUF_SIZE);

        if (n == 0) {
            continue; /* leere Zeile ignorieren */
        }

        /* --------- D: Dump (Adresse lesen, Wert ausgeben) --------- */
        if (line[0] == 'D' || line[0] == 'd') {

            /* Pflicht: Leerzeichen nach D */
            if (n < 2 || line[1] != ' ') {
                uart1_puts("ERR (use: D <8hex>)\r\n");
                continue;
            }

            /* Mindestens 8 Hex-Zeichen nach dem Leerzeichen */
            if (n < 2 + 8) {
                uart1_puts("ERR (addr missing)\r\n");
                continue;
            }

            if (!parse_hex_n(&line[2], 8, &addr)) {
                uart1_puts("ERR (addr not hex)\r\n");
                continue;
            }

            ptr = (volatile unsigned char *)addr;

            /* Ausgabe: AABBCCDD: XX */
            print_hex8((addr >> 24) & 0xFF);
            print_hex8((addr >> 16) & 0xFF);
            print_hex8((addr >> 8)  & 0xFF);
            print_hex8(addr & 0xFF);
            uart1_puts(": ");
            print_hex8(*ptr);
            uart1_puts("\r\n");
        }

        /* --------- E: Edit (Adresse + Datenbyte schreiben) --------- */
        else if (line[0] == 'E' || line[0] == 'e') {

            /* Pflicht: Leerzeichen nach E */
            if (n < 2 || line[1] != ' ') {
                uart1_puts("ERR (use: E <8hex> <2hex>)\r\n");
                continue;
            }

            /* Erwartet: "E " + 8hex + " " + 2hex => mind. 13 Zeichen */
            if (n < (2 + 8 + 1 + 2)) {
                uart1_puts("ERR (use: E <8hex> <2hex>)\r\n");
                continue;
            }

            if (!parse_hex_n(&line[2], 8, &addr)) {
                uart1_puts("ERR (addr not hex)\r\n");
                continue;
            }

            /* Pflicht-Leerzeichen nach Adresse (Position 10) */
            if (line[10] != ' ') {
                uart1_puts("ERR (need space between addr and data)\r\n");
                continue;
            }

            if (!parse_hex_n(&line[11], 2, &tmp)) {
                uart1_puts("ERR (data not hex)\r\n");
                continue;
            }

            data = (unsigned char)tmp;

            ptr = (volatile unsigned char *)addr;
            *ptr = data;

            uart1_puts("OK\r\n");
        }

        else {
            uart1_puts("?\r\n");
        }
    }
}

/* ------------------------- UART FUNKTIONEN ------------------------- */

void init_uart1(void) {
    /* P0.8 = TXD1, P0.9 = RXD1 -> Funktion 01 an beiden Pins */
    PINSEL0 |= 0x50000;

    /* 8 Datenbits, 1 Stopbit, keine Parität, DLAB=1 (Baudrate einstellen) */
    U1LCR = 0x83;              /* 0b1000_0011 */

    /* Baudrate: Beispielwerte (bitte passend zu eurem Setup!)
       Bei vielen LPC21xx: PCLK = 15 MHz oder 12 MHz etc.
       Divisor = PCLK / (16*Baud)
       Dein ursprünglicher Code hatte 41 (passt eher zu ~19200/?? je nach PCLK).
       -> Lass es so, wenn es bei euch stimmt. */
    U1DLL = 41;
    U1DLM = 0x00;

    /* DLAB=0 */
    U1LCR = 0x03;              /* 8N1 */

    /* FIFO enable + reset RX/TX FIFO */
    U1FCR = 0x07;
}

void uart1_putc(char c) {
    while (!(U1LSR & 0x20)) { /* THR empty */
    }
    U1THR = c;
}

char uart1_getc(void) {
    while (!(U1LSR & 0x01)) { /* RDR */
    }
    return U1RBR;
}

void uart1_puts(const char *s) {
    while (*s) {
        uart1_putc(*s++);
    }
}

void print_hex8(unsigned char val) {
    const char hex[] = "0123456789ABCDEF";
    uart1_putc(hex[val >> 4]);
    uart1_putc(hex[val & 0x0F]);
}
