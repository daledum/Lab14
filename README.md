# Lab14
Measurement of distance, using a slide potentiometer, written in C. 

This is lab 14 from the Edx hosted MOOC UTAustinX: UT.6.01x Embedded Systems - Shape the World.

This lab has these major objectives: 
1) an introduction to sampling analog signals using the ADC interface; 
2) the development of an ADC device driver; 
3) learning data conversion and calibration techniques; 
4) the use of fixed-point numbers, which are integers used to represent non-integer values; 
5) the development of an interrupt-driven real-time sampling device driver; 
6) the development of a software system involving multiple files; 
and 7) learn how to debug one module at a time.

Use SysTick interrupts to periodically initiate a software-triggered ADC conversion, convert the sample to a fixed-point decimal distance, and store the result in a mailbox.
The foreground thread takes the result from the mailbox, converts the result to a string, and prints it to the Nokia5110 LCD. The display is optional.

Hardware used:
TI Tiva C Series TM4C123GXL

SW Tools:
Keil uVision (MDK-Lite Version 4.73)
IDE-Version:
µVision V4.73.0.0
Copyright (C) 2013 ARM Ltd and ARM Germany GmbH. All rights reserved.
Tool Version Numbers:
Toolchain:        MDK-Lite  Version: 4.73.0.0
Toolchain Path:    C:\Keil\ARM\ARMCC\bin\
C Compiler:         Armcc.Exe       V5.03.0.76 [Evaluation]
Assembler:          Armasm.Exe       V5.03.0.76 [Evaluation]
Linker/Locator:     ArmLink.Exe       V5.03.0.76 [Evaluation]
Librarian:          ArmAr.Exe       V5.03.0.76 [Evaluation]
Hex Converter:      FromElf.Exe       V5.03.0.76 [Evaluation]
CPU DLL:            SARMCM3.DLL       V4.73.0.0
Dialog DLL:         DCM.DLL       V1.10.0.0
Target DLL:         lmidk-agdi.dll
Dialog DLL:         TCM.DLL       V1.14.1.0
