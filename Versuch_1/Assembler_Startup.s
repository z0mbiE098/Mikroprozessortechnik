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
					
RAM_Size      	EQU      	0x400     		; 1024 Byte Stack-Größe

Top_Stack       EQU      	Datenanfang+RAM_Size
Datenende       EQU      	Top_Stack

;********************************************************************
;* Programm-Bereich bzw. Programm-Speicher							*
;********************************************************************
				
				AREA		Programm, CODE, READONLY
				ARM
Reset_Handler	MSR			CPSR_c, #0x10	; User Mode
				LDR SP, =Top_Stack

;********************************************************************
;* Hier das eigene (Haupt-)Programm einfuegen   					*
;********************************************************************
				
				LDR R0, =String
				BL AtoI
				
				LDR R0, =X
				LDR R0, [R0]
				BL Formel
				
				LDR R0, =Number
				LDR R0, [R0]
				BL uItoBCD
;********************************************************************
;* Ende des eigenen (Haupt-)Programms                               *
;********************************************************************

endlos			B			endlos

;********************************************************************
;* ab hier Unterprogramme                                           *
;********************************************************************


;------------------------Aufgabe 1-----------------------------------
AtoI
				PUSH {R1, R2, R3, R4, R5, LR}
				MOV R2, #0
				MOV R3, #0
				LDRB R1, [R0]
				
				CMP R1, #0x2B
				BEQ Skip_first_positive
				
				CMP R1, #0x2D
				BEQ Set_Minus
				B AtoI_Loop

Skip_first_positive
				ADD R0, R0, #1
				B AtoI_Loop
				
Set_Minus
				MOV R3, #1
				ADD R0, R0, #1

AtoI_Loop		
				LDRB R1, [R0]
				CMP R1, #0
				BEQ AtoI_EndDigits
				
				SUB R1, R1, #CHAR_0
				MOV R4, R2, LSL #3
				ADD R4, R4, R2, LSL#1
				ADD R2, R4, R1
				
				ADD R0, R0, #1
				B AtoI_Loop
AtoI_EndDigits
				CMP R3, #0
				BEQ AtoI_Positive
				RSB R2, R2, #0
				
AtoI_Positive
				MOV R0, R2
				POP {R1, R2, R3, R4, R5, LR}
				BX LR

;------------------------Aufgabe 2-----------------------------------

Formel
				PUSH {R1, R2, R3, R4, LR}
				MOV R1, R0
				MUL R2, R1, R1
				MOV R3, #0
				
Division_Loop
				CMP R2, #9
				BLT Division_Done
				
				SUB R2, R2, #9
				ADD R3, R3, #1
				B Division_Loop
				
Division_Done
				MOV R0, R3, LSL #2
				POP{R1, R2, R3, R4, LR}
				BX LR
	
;------------------------Aufgabe 3-----------------------------------

uItoBCD
				PUSH {R1, R2, R3, R4, R5, LR}
				MOV R1, #0
				MOV R2, #0
				
uItoBCD_Loop
				CMP R0, #0
				BEQ uItoBCD_Done
				MOV R3, R0
				MOV R4, #0

uItoBCD_Division_Loop
				CMP R3, #10
				BLT uItoBCD_END
				SUB R3, R3, #10
				ADD R4, R4, #1
				B uItoBCD_Division_Loop
				
uItoBCD_END
				MOV R5, R3, LSL R2
				ORR R1, R1, R5
				ADD R2, R2, #4
				MOV R0, R3
				B uItoBCD_Loop
				
uItoBCD_Done
				MOV R0, R1
				POP{R1, R2, R3, R4, R5, LR}
				BX LR
				
;********************************************************************
;* Konstanten im CODE-Bereich                                       *
;********************************************************************

String DCB "65535",0
CHAR_0 EQU 0x30
		ALIGN
X DCD 30
Number DCD 3035 

;********************************************************************
;* Ende der Programm-Quelle                                         *
;********************************************************************
				END
