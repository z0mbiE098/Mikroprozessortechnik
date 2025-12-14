;********************************************************************
;* htw saar - Fakultaet fuer Ingenieurwissenschaften				*
;* Labor fuer Eingebettete Systeme									*
;* Mikroprozessortechnik											*
;********************************************************************
;* Assembler_Startup.S: 											*
;* Programmrumpf fuer Assembler-Programme mit dem Keil				*
;* Entwicklungsprogramm uVision fuer ARM-Mikrocontroller			*
;********************************************************************
;* Aufgabe-Nr.:         	*	               						*
;*              			*						    			*
;********************************************************************
;* Gruppen-Nr.: 			*										*
;*              			*										*
;********************************************************************
;* Name / Matrikel-Nr.: 	*										*
;*							*										*
;*							*										*
;********************************************************************
;* Abgabedatum:         	*              							*
;*							*										*
;********************************************************************

;********************************************************************
;* Daten-Bereich bzw. Daten-Speicher				            	*
;********************************************************************
				AREA		Daten, DATA, READWRITE
Datenanfang
;********************************************************************
;* Programm-Bereich bzw. Programm-Speicher							*
;********************************************************************
				AREA		Programm, CODE, READONLY
				ARM
Reset_Handler	MSR			CPSR_c, #0x10	; User Mode aktivieren


;********************************************************************
;* Hier das eigene (Haupt-)Programm einfuegen   					*
;********************************************************************
                ;LDR     R0, =STRING     ; R0 = &STRING (Adresse von String wird in R0 geladen)
                ;BL      AtoI            ; AtoI wird gerufen
			
				LDR 	R0, =NUMBER	;Replace String mit X for Aufgabe_2
				;LDR 	R0, [R0]		;Zeile für Aufgabe 2
				LDR 	R0, [R0]		;Zeile für Aufgabe 3
				BL uItoBCD					; Funktion Aufrufen
;********************************************************************
;* Ende des eigenen (Haupt-)Programms                               *
;********************************************************************
endlos			B			endlos

;********************************************************************
;* ab hier Unterprogramme                                           *
;********************************************************************
CHAR_0      EQU     0x30    ; '0'

;------------------------Aufgabe 1-----------------------------------
AtoI
                MOV     R2, #0          ; Hier wird Result gespeichert
                MOV     R3, #0          ; Vorzeichen Flag. 0 = positiv, 1 = negative
                LDRB    R1, [R0]        ; erstes Zeichen wird geholt
                CMP     R1, #0x2B		; Überprüfen, ob erstes Zeichen positiv ist
                BEQ     AtoI_SkipPlus

                CMP     R1, #0x2D		; Überprüfen ob erstes Zeichen negativ ist
                BEQ     AtoI_SetMinus
				
                B       AtoI_Loop  		; sonst direkt in die Schleife mit dem aktuellen Zeichen

AtoI_SkipPlus
                ADD     R0, R0, #1      ; zum nächsten Zeichen
                B       AtoI_Loop

AtoI_SetMinus
                MOV     R3, #1          ; Negatives Vorzeichen wird gemerkt
                ADD     R0, R0, #1
                B       AtoI_Loop
				
AtoI_Loop
                LDRB    R1, [R0]        ; aktuelles Zeichen
                CMP     R1, #0          ; Überprüfen,R ob wir Ende vom String erreicht haben.
                BEQ     AtoI_EndDigits  ; ja -> fertig

                ; ASCII-Zeichen in Ziffer 0..9 umwandeln: digit = char - '0'
                SUB     R1, R1, #CHAR_0

           
                ; 10 = 8 + 2 -> result*10 = (result<<3) + (result<<1)
                MOV     R4, R2, LSL #3              ; R4 = result*8
                ADD     R4, R4, R2, LSL #1          ; R4 = result*8 + result*2 = result*10
                ADD     R2, R4, R1                  ; result = result*10 + digit

                ; n?chstes Zeichen
                ADD     R0, R0, #1
                B       AtoI_Loop
				
AtoI_EndDigits
                CMP     R3, #0
                BEQ     AtoI_Positive

                ; negativ: result = -result
                RSB     R2, R2, #0      ; R2 = 0 - result

AtoI_Positive
                MOV     R0, R2          ; R?ckgabewert in R0
                BX      LR              ; zur?ck zum Aufrufer


;------------------------Aufgabe 2-----------------------------------
Formel
				MOV R1, R0
				
				MUL R2, R1, R1
				MOV R3, #0

DivideLoop
				CMP R2, #9
				BLT DivDone
				
				SUB R2, R2, #9
				ADD R3, R3, #1
				B DivideLoop
				
DivDone
				MOV R0, R3, LSL #2
				BX LR
;------------------------Aufgabe 3-----------------------------------

uItoBCD
				MOV R1, #0				; Ergebnis wird hier gespeichert 
				MOV R2, #0				; R2 = Shiftzähler

uItoBCD_Loop
				CMP R0, #0
				BEQ uItoBCD_Done
				
				MOV R3, R0
				MOV R4, #0
				
Division_10
				CMP R3, #10
				BLT Division_END
				SUB R3, R3, #10
				ADD R4, R4, #1
				B Division_10
				
Division_END				
				MOV R5, R3, LSL R2
				ORR R1, R1, R5
				ADD R2, R2, #4
				MOV R0, R4
				B uItoBCD_Loop
				

uItoBCD_Done
				MOV R0, R1
				BX LR

;------------------------Aufgabe 4-----------------------------------
                
;********************************************************************
;* Konstanten im CODE-Bereich                                       *
;********************************************************************
X 				DCD 30
STRING          DCB     "-65535",0      ; '\0'-terminierter String
NUMBER 			DCD		65535
;********************************************************************
;* Ende der Programm-Quelle                                         *
;********************************************************************
				END
