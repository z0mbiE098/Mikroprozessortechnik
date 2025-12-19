;********************************************************************
;* htw saar - Fakultaet fuer Ingenieurwissenschaften				*
;* Labor fuer Eingebettete Systeme									*
;* Mikroprozessortechnik											*
;********************************************************************
;* Assembler_Startup.S: 											*
;* Programmrumpf fuer Assembler-Programme mit dem Keil				*
;* Entwicklungsprogramm uVision fuer ARM-Mikrocontroller			*
;********************************************************************
;* Aufgabe-Nr.:         	*	 Verusch 1              			*
;*              			*						    			*
;********************************************************************
;* Gruppen-Nr.: 			*	1. Gruppe am Freitag ab 3. Stunde	*
;*              			*										*
;********************************************************************
;* Name / Matrikel-Nr.: 	* Hanan Ahmed Ashir, 5012967			*
;*							* Erwin Holzhauser, 5013983				*
;*							*										*
;********************************************************************
;* Abgabedatum:         	*  19.12.2025            				*
;*							*										*
;********************************************************************

;********************************************************************
;* Daten-Bereich bzw. Daten-Speicher				            	*
;********************************************************************
				AREA		Daten, DATA, READWRITE	;Ezeugung einer Speicherbereich für Daten(RAM)  					
Datenanfang
					
RAM_Size      	EQU      	0x800     		; 4*16^2=1024 bytes

Top_Stack       EQU      	Datenanfang+RAM_Size  
Datenende       EQU      	Top_Stack

;********************************************************************
;* Programm-Bereich bzw. Programm-Speicher							*
;********************************************************************
				
				AREA		Programm, CODE, READONLY
				ARM
Reset_Handler	MSR			CPSR_c, #0x10	; MSR=Move to Status Register. Der User Mode wird in CPSR geschrieben

String DCB "10",0
	ALIGN								 	; Falls die Adresse nicht durch 4 teilbar ist, fülle sie mit Nullen
;********************************************************************
;* Hier das eigene (Haupt-)Programm einfuegen   					*
;********************************************************************
				LDR SP, =Top_Stack		;Stack-pointer startet hier
				LDR R0, =String		; Load Register: Die Adresse von vollständiges 32 Bit Wort wird geladen
				BL Berechnung   	; 
				
				;LDR R0, =String
				;BL AtoI
				
				;LDR R0, =X
				;LDR R0, [R0]
				;BL Formel
				
				;LDR R0, =Number
				;LDR R0, [R0]
				;BL uItoBCD
;********************************************************************
;* Ende des eigenen (Haupt-)Programms                               *
;********************************************************************

endlos	B					endlos	;

;********************************************************************
;* ab hier Unterprogramme                                           *
;********************************************************************


;------------------------Aufgabe 1-----------------------------------

AtoI
				STMFD	SP!, {R1-R5, LR}
                MOV     R2, #0   		; Hier wird das Ergebnis gespeichert
				MOV		R5, #0			; Hier kommt der Status Flag
				
                LDRB    R1, [R0]        ; Das erste Zeichen von der Eingabe wird geprüft
				CMP		R1, #0x2D 		; CMP macht intern eine Subtraktion: R1 = R1-0x2D und setzt die Flags im CSPR
				MOVEQ	R5, R1			; Setze Flag in R5. Move if Z-Flag=1
				
				CMPNE	R1, #0x2B		; Auch eine Subtraktion: R1 = R1-0x2B und die Flags in CSPR werden gesetzt 
				ADDEQ	R0, R0, #1		; Wenn Zeichen + ist und Flag gesetzt ist, wird ADDEQ ausgeführt und Pointer wird zu nächster Stelle inkrementiert			

AtoI_Loop
				LDRB    R1, [R0], #1    ; Load Register Byte. Zeochen aus R0 wird geladen und Zeiger zeigt mit #1 direkt auf die nächste Stelle 
                CMP     R1, #0          ; R1=R1-0. Flags setzen
                BEQ     AtoI_End  		; Falls Stringende erreicht, dann zum Ende springen
          
                SUB     R1, R1, #CHAR_0 	; ASCII Zeichen werden in numerischer Wert konvertiert(z.B. 6(0x36) - 0(0x30) = 6)

                MOV     R4, R2, LSL #3      ; Ersten teil der Multiplikation. 
											; Die zahl die bisher in R2 steht muss auf Zehnerstelle rücken, um platz für die nächste Zahl zu machen.
                ADD     R4, R4, R2, LSL #1  ; Add nimmt das Ergebnis von R4 und addiert die R2*2 dazu.
											; R4  = Die bisherige Zahl(multipliziert mit 10)
                ADD     R2, R4, R1          ; Jetzt müssen wir die neue Zahl bilden. (Alte Zahl in R4(60) + neue Zahl in R1(5) = "65___"
                B       AtoI_Loop			; Zurück zum Anfang, wo es weitergeht
				
AtoI_End
				CMP		R5, #0			; Falls im R5 eine negative Flag steht.
				RSBNE	R2, R2, #0		; Reverse Substract Not Equals, 0 - R2, falls negative Flag gesetzt ist, um Zahl zu negieren		
				MOV 	R0, R2			; Ergebnis in R0 abspeichern
				LDMFD	SP!, {R1-R5, LR} ; Registern wieder freigeben
				BX		LR




;-----------------Aufgabe 2 mit magic numbers-----------------------

Formel
				STMFD SP! ,{R1-R4, LR}
				LDR        R1, =DIV_9		; R1 wird mit der Magic Number geladen(Siehe Skript Seite 96) 
				MUL        R2, R0, R0		; Quadrat wird berechnet und in R2 gespeichert: R2 = (R0)^2
				UMULL      R3, R4, R2, R1   ; UMULL: Unsigned Multiply long, weil X^2*DIV_9 eine Zahl>32-Bit ist. 
											; Ergebis wird eine 64-Bit Zahl, die in zwei Registern gespeichert wird. (63-32)=R4, (31-0)=R3
				MOV        R3, R4, LSR #1   ; Um die größe Zahl mit genauigkeit in einem register zu schreiben machen wir ein Right shift.(siehe "n" auf der Siete 96)
											; Also (X^2/9) wird grob geschätzt und im Register R3 gespeichert.
				MOV        R0, R3, LSL #2   ; Wir rechnen hier (X^2/9)*4. Also Link Shift um 2^2=4.
				LDMFD  SP!, {R1-R4, LR}		; Registern werden wieder vom Stack entfernt und Ergebnis steht im R0
				BX		LR					; Wieder zu Rücksprungadressen springen

;------------------------Aufgabe 3 mit magic numbers-----------------

; anhand dieses Formel (Pseudocode): while (dec)
;    {
;        result +=  (dec % 10) << shift;
;        dec = dec / 10;
;        shift += 4;
;    }
;    return result;

uItoBCD
        STMFD SP! ,{R1-R8, LR}				; Register R1 bis R8 auf den Stack laden.
        MOV  R1, #0          				; Ergebnis wird hier gespeichert
        MOV  R2, #0          				; Shift kommmt hier. (0,4,8,...)
		LDR  R5, =DIV_10					; R5 bekommt magische Nummer für Division durch 10
		MOV  R6, #10						; R6 bekommt die Konstante Zahl 10 für Modulo-Operation

uItoBCD_Loop
        CMP  R0, #0							; R0 = R0-0 und Flags Setzen. Falls Ergebnis 0, dann sind wir schon fertig
        BEQ  uItoBCD_Done					; Springe zu Ende

        
        UMULL R3, R7, R0, R5                ; Unsigned Integer, also UMULL darf verwendet werden. 
											; R0 wird mit magische Nummer multipliziert und Ergebnis wird in R3 ind R7 gespeichert   
        MOV R3, R7, LSR#3  					; Genauere Ergebis in 32 Bit, R7 muss noch 3 bits geshiftet werden(siehe "n" Seite 96)
											; Für 123 berechnen wir: q = 123/10 = 12(kommt in R3)
		
        MUL R4, R3, R6						; In R4 steht jetzt = q * 10.  Dann ist R4=12*10=120 
		SUB R4, R0, R4       				; r = R0 - (q * 10) , sprich modulo. (123 -120) = 3. Also letzte Dezimalziffer steht in R4
		
		MOV R8, R4, LSL R2   				; Nimmt den Inhalt von n und schiebt so viele Bits nach links, wie in R2 steht
											; Das Ergebnis landet dann in R8
		ADD R1, R1, R8       				; R1 speichert das BCD Ergebnis, R8 wird dort addiert.
	
		
		MOV R0, R3          				; R3 enthält die Quotient, das wird in R0 als Eingabe geschickt und die nächste Iteration beginnt
		ADD R2, R2, #4       				; bei jeder iteration wird einmla mehr um 4 geshiftet, damit die nächste Dezimalzahl richtig positioniert wird

		B    uItoBCD_Loop					; Sprint zurück zum Anfang der Schleife

uItoBCD_Done
        MOV  R0, R1          				; Fertige BCD Ergnis wird in R0 gemoved 
        LDMFD SP! ,{R1-R8, LR}				; Registern wieder freimachen
        BX   LR								; Zur Rücksprungadresse springen
				
;------------------------Aufgabe 4-----------------------------------

Berechnung
                STMFD SP!, {LR}         ; Rücksprungadresse sichern
				
                BL AtoI           		; R0 = X (signed Integer)
                BL Formel        		; R0 = Y = 4*X^2/9
                BL uItoBCD        		; R0 = Y als BCD

                LDMFD SP! ,{LR}
                BX LR
				
				
;********************************************************************
;* Konstanten im CODE-Bereich                                       *
;********************************************************************

DIV_9            EQU 0x38E38E39     ; mit n=1
DIV_10           EQU 0xCCCCCCCD    ; mit n=3
CHAR_0 EQU 0x30
	
;X DCD 100
;Number DCD 3043


;********************************************************************
;* Ende der Programm-Quelle                                         *
;********************************************************************
				END