EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:optocoupler
LIBS:eelib
LIBS:Conn-raspberry
LIBS:Traco
LIBS:PiCan-cache
EELAYER 27 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "CAN-BUS Interface for Raspberry Pi"
Date "9 sep 2014"
Rev "1"
Comp "Juerg5524"
Comment1 "Galvanisch getrennte CAN-BUS-Anbindung an Raspberry Pi"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L 6N137 OC2
U 1 1 53762BC4
P 6000 4300
F 0 "OC2" H 5625 4625 50  0000 L BNN
F 1 "6N137" H 5625 4000 50  0000 L BNN
F 2 "optocoupler-DIL08" H 6000 4450 50  0001 C CNN
F 3 "" H 6000 4300 60  0000 C CNN
	1    6000 4300
	1    0    0    -1  
$EndComp
$Comp
L 6N137 OC1
U 1 1 53762BD3
P 5900 3100
F 0 "OC1" H 5525 3425 50  0000 L BNN
F 1 "6N137" H 5525 2800 50  0000 L BNN
F 2 "optocoupler-DIL08" H 5900 3250 50  0001 C CNN
F 3 "" H 5900 3100 60  0000 C CNN
	1    5900 3100
	-1   0    0    -1  
$EndComp
$Comp
L MCP2515 U1
U 1 1 537758FE
P 3950 3650
F 0 "U1" H 3550 4450 60  0000 C CNN
F 1 "MCP2515" H 3700 2900 60  0000 C CNN
F 2 "" H 3950 3650 60  0000 C CNN
F 3 "" H 3950 3650 60  0000 C CNN
	1    3950 3650
	1    0    0    -1  
$EndComp
$Comp
L MCP2551 U2
U 1 1 53775912
P 7750 3700
F 0 "U2" H 7500 4100 60  0000 C CNN
F 1 "MCP2551" H 7650 3300 60  0000 C CNN
F 2 "" H 7750 3700 60  0000 C CNN
F 3 "" H 7750 3700 60  0000 C CNN
	1    7750 3700
	1    0    0    -1  
$EndComp
$Comp
L R R8
U 1 1 53775BCF
P 7150 4350
F 0 "R8" V 7230 4350 40  0000 C CNN
F 1 "10k" V 7157 4351 40  0000 C CNN
F 2 "~" V 7080 4350 30  0000 C CNN
F 3 "~" H 7150 4350 30  0000 C CNN
	1    7150 4350
	1    0    0    -1  
$EndComp
$Comp
L R R6
U 1 1 53775CFD
P 6650 2900
F 0 "R6" V 6730 2900 40  0000 C CNN
F 1 "680" V 6657 2901 40  0000 C CNN
F 2 "~" V 6580 2900 30  0000 C CNN
F 3 "~" H 6650 2900 30  0000 C CNN
	1    6650 2900
	0    1    1    0   
$EndComp
Wire Wire Line
	6400 4000 6400 4200
$Comp
L R R7
U 1 1 53775F84
P 7150 3150
F 0 "R7" V 7230 3150 40  0000 C CNN
F 1 "4k7" V 7157 3151 40  0000 C CNN
F 2 "~" V 7080 3150 30  0000 C CNN
F 3 "~" H 7150 3150 30  0000 C CNN
	1    7150 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	6400 3500 6400 3200
Connection ~ 7150 2900
$Comp
L C C6
U 1 1 53776081
P 6900 4350
F 0 "C6" H 6900 4450 40  0000 L CNN
F 1 "100n" H 6906 4265 40  0000 L CNN
F 2 "~" H 6938 4200 30  0000 C CNN
F 3 "~" H 6900 4350 60  0000 C CNN
	1    6900 4350
	1    0    0    -1  
$EndComp
Wire Wire Line
	6900 4000 6400 4000
Connection ~ 6400 4100
Wire Wire Line
	6600 3400 6600 4300
Wire Wire Line
	6600 4300 6400 4300
Connection ~ 7150 3400
Wire Wire Line
	5500 2300 5500 3000
Connection ~ 5500 2900
$Comp
L +3.3V #PWR01
U 1 1 53786402
P 3300 2500
F 0 "#PWR01" H 3300 2460 30  0001 C CNN
F 1 "+3.3V" H 3300 2610 30  0000 C CNN
F 2 "" H 3300 2500 60  0000 C CNN
F 3 "" H 3300 2500 60  0000 C CNN
	1    3300 2500
	1    0    0    -1  
$EndComp
$Comp
L R R5
U 1 1 53786476
P 5050 2750
F 0 "R5" V 5130 2750 40  0000 C CNN
F 1 "4k7" V 5057 2751 40  0000 C CNN
F 2 "~" V 4980 2750 30  0000 C CNN
F 3 "~" H 5050 2750 30  0000 C CNN
	1    5050 2750
	1    0    0    -1  
$EndComp
$Comp
L R R4
U 1 1 5378658A
P 4800 2750
F 0 "R4" V 4880 2750 40  0000 C CNN
F 1 "470" V 4807 2751 40  0000 C CNN
F 2 "~" V 4730 2750 30  0000 C CNN
F 3 "~" H 4800 2750 30  0000 C CNN
	1    4800 2750
	1    0    0    -1  
$EndComp
$Comp
L CRYSTAL X1
U 1 1 53786867
P 3950 4750
F 0 "X1" H 3950 4900 60  0000 C CNN
F 1 "16 MHz" H 3950 4600 60  0000 C CNN
F 2 "~" H 3950 4750 60  0000 C CNN
F 3 "~" H 3950 4750 60  0000 C CNN
	1    3950 4750
	1    0    0    -1  
$EndComp
$Comp
L C C4
U 1 1 53786880
P 4250 4950
F 0 "C4" H 4250 5050 40  0000 L CNN
F 1 "22p" H 4256 4865 40  0000 L CNN
F 2 "~" H 4288 4800 30  0000 C CNN
F 3 "~" H 4250 4950 60  0000 C CNN
	1    4250 4950
	1    0    0    -1  
$EndComp
$Comp
L C C3
U 1 1 53786886
P 3650 4950
F 0 "C3" H 3650 5050 40  0000 L CNN
F 1 "22p" H 3656 4865 40  0000 L CNN
F 2 "~" H 3688 4800 30  0000 C CNN
F 3 "~" H 3650 4950 60  0000 C CNN
	1    3650 4950
	1    0    0    -1  
$EndComp
$Comp
L C C2
U 1 1 5378695D
P 2600 2700
F 0 "C2" H 2600 2800 40  0000 L CNN
F 1 "10n" H 2606 2615 40  0000 L CNN
F 2 "~" H 2638 2550 30  0000 C CNN
F 3 "~" H 2600 2700 60  0000 C CNN
	1    2600 2700
	1    0    0    -1  
$EndComp
$Comp
L PBD-26 J1
U 1 1 53786A00
P 1800 3650
F 0 "J1" H 1800 4350 60  0000 C CNN
F 1 "GIOP Raspberry Pi" V 1800 3650 50  0000 C CNN
F 2 "" H 1800 3650 60  0000 C CNN
F 3 "" H 1800 3650 60  0000 C CNN
	1    1800 3650
	1    0    0    -1  
$EndComp
$Comp
L R R3
U 1 1 53786B62
P 3050 2750
F 0 "R3" V 3130 2750 40  0000 C CNN
F 1 "100k" V 3057 2751 40  0000 C CNN
F 2 "~" V 2980 2750 30  0000 C CNN
F 3 "~" H 3050 2750 30  0000 C CNN
	1    3050 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	3050 3000 3050 3150
$Comp
L R R2
U 1 1 53786B9F
P 2900 2750
F 0 "R2" V 2980 2750 40  0000 C CNN
F 1 "100k" V 2907 2751 40  0000 C CNN
F 2 "~" V 2830 2750 30  0000 C CNN
F 3 "~" H 2900 2750 30  0000 C CNN
	1    2900 2750
	1    0    0    -1  
$EndComp
Wire Wire Line
	2900 3000 2900 3300
Wire Wire Line
	2600 2900 2600 5300
Wire Wire Line
	2600 3250 2350 3250
Wire Wire Line
	1250 2500 5050 2500
Connection ~ 3050 2500
Connection ~ 2900 2500
Connection ~ 2600 2500
Wire Wire Line
	2750 3150 2750 2400
Connection ~ 3050 3150
Wire Wire Line
	2500 3850 3300 3850
Wire Wire Line
	2500 3850 2500 4050
Wire Wire Line
	2500 4050 2350 4050
Wire Wire Line
	2750 3300 2750 4150
Wire Wire Line
	2750 4150 2350 4150
Connection ~ 2900 3300
Wire Wire Line
	2850 4400 2850 3600
Wire Wire Line
	2850 3600 3300 3600
Wire Wire Line
	2950 3400 2950 4500
Wire Wire Line
	3050 3500 3300 3500
Wire Wire Line
	3050 3500 3050 4600
$Comp
L +5V #PWR02
U 1 1 537874B1
P 2400 2300
F 0 "#PWR02" H 2400 2390 20  0001 C CNN
F 1 "+5V" H 2400 2390 30  0000 C CNN
F 2 "" H 2400 2300 60  0000 C CNN
F 3 "" H 2400 2300 60  0000 C CNN
	1    2400 2300
	1    0    0    -1  
$EndComp
$Comp
L LED D4
U 1 1 53787A6A
P 8450 4450
F 0 "D4" H 8450 4550 50  0000 C CNN
F 1 "LED" H 8450 4350 50  0000 C CNN
F 2 "~" H 8450 4450 60  0000 C CNN
F 3 "~" H 8450 4450 60  0000 C CNN
	1    8450 4450
	0    1    1    0   
$EndComp
$Comp
L R R9
U 1 1 53787A92
P 8450 4000
F 0 "R9" V 8530 4000 40  0000 C CNN
F 1 "2k2" V 8457 4001 40  0000 C CNN
F 2 "~" V 8380 4000 30  0000 C CNN
F 3 "~" H 8450 4000 30  0000 C CNN
	1    8450 4000
	1    0    0    -1  
$EndComp
$Comp
L 7805 U3
U 1 1 53787F29
P 9300 4300
F 0 "U3" H 9450 4104 60  0000 C CNN
F 1 "L7805" H 9300 4500 60  0000 C CNN
F 2 "~" H 9300 4300 60  0000 C CNN
F 3 "~" H 9300 4300 60  0000 C CNN
	1    9300 4300
	-1   0    0    -1  
$EndComp
$Comp
L C C8
U 1 1 53787F98
P 8900 4450
F 0 "C8" H 8900 4550 40  0000 L CNN
F 1 "100n" H 8906 4365 40  0000 L CNN
F 2 "~" H 8938 4300 30  0000 C CNN
F 3 "~" H 8900 4450 60  0000 C CNN
	1    8900 4450
	-1   0    0    -1  
$EndComp
Wire Wire Line
	9300 4650 9300 4550
Connection ~ 9300 4650
Wire Wire Line
	6400 4650 9700 4650
$Comp
L DIODE D3
U 1 1 53788ADC
P 9900 4000
F 0 "D3" H 9900 4100 40  0000 C CNN
F 1 "DIODE" H 9900 3900 40  0000 C CNN
F 2 "~" H 9900 4000 60  0000 C CNN
F 3 "~" H 9900 4000 60  0000 C CNN
	1    9900 4000
	-1   0    0    -1  
$EndComp
$Comp
L DB9 J2
U 1 1 53788AEB
P 10550 3700
F 0 "J2" H 10550 4250 70  0000 C CNN
F 1 "DB9" H 10550 3150 70  0000 C CNN
F 2 "" H 10550 3700 60  0000 C CNN
F 3 "" H 10550 3700 60  0000 C CNN
	1    10550 3700
	1    0    0    -1  
$EndComp
$Comp
L CP1 C7
U 1 1 53789022
P 9700 4450
F 0 "C7" H 9750 4550 50  0000 L CNN
F 1 "1u" H 9750 4350 50  0000 L CNN
F 2 "~" H 9700 4450 60  0000 C CNN
F 3 "~" H 9700 4450 60  0000 C CNN
	1    9700 4450
	-1   0    0    -1  
$EndComp
Wire Wire Line
	6900 2900 6900 4150
Wire Wire Line
	2350 3050 2400 3050
Wire Wire Line
	2400 3050 2400 2300
Connection ~ 2600 3250
Wire Wire Line
	6400 4650 6400 4400
Connection ~ 6900 4000
Wire Wire Line
	6900 4550 6900 4650
Connection ~ 6900 4650
Wire Wire Line
	8250 4650 8250 4000
Connection ~ 8250 4650
Wire Wire Line
	2750 3150 3300 3150
Wire Wire Line
	2750 3300 3300 3300
Wire Wire Line
	2950 3400 3300 3400
Connection ~ 3300 2500
Wire Wire Line
	1250 2500 1250 3050
Wire Wire Line
	2750 2400 1100 2400
Wire Wire Line
	1100 2400 1100 3550
Wire Wire Line
	1100 3550 1250 3550
Wire Wire Line
	2850 4400 1100 4400
Wire Wire Line
	1100 4400 1100 4150
Wire Wire Line
	1100 4150 1250 4150
Wire Wire Line
	2950 4500 1000 4500
Wire Wire Line
	1000 4500 1000 4050
Wire Wire Line
	1000 4050 1250 4050
Wire Wire Line
	3050 4600 900  4600
Wire Wire Line
	900  4600 900  3950
Wire Wire Line
	900  3950 1250 3950
Wire Wire Line
	2400 2300 5500 2300
Wire Wire Line
	3650 4750 3650 4550
Wire Wire Line
	3650 4550 3800 4550
Wire Wire Line
	4100 4550 4250 4550
Wire Wire Line
	4250 4550 4250 4750
Wire Wire Line
	4250 5300 4250 5150
Wire Wire Line
	3650 5150 3650 5300
Wire Wire Line
	4600 5300 4600 4250
NoConn ~ 2350 4250
NoConn ~ 2350 3850
NoConn ~ 2350 3750
NoConn ~ 2350 3650
NoConn ~ 2350 3550
NoConn ~ 2350 3450
NoConn ~ 2350 3350
NoConn ~ 1250 3650
NoConn ~ 1250 3750
NoConn ~ 1250 3850
NoConn ~ 1250 3350
NoConn ~ 1250 3250
NoConn ~ 1250 3150
NoConn ~ 3300 4050
NoConn ~ 3300 4200
NoConn ~ 4600 3400
Wire Wire Line
	3300 2500 3300 3000
Wire Wire Line
	4800 3000 4800 4100
Wire Wire Line
	4800 4100 5500 4100
Wire Wire Line
	4600 3000 4700 3000
Wire Wire Line
	4700 3000 4700 4400
Wire Wire Line
	4700 4400 5500 4400
Wire Wire Line
	5300 5300 5300 3000
Wire Wire Line
	5300 3200 5500 3200
Wire Wire Line
	4600 3100 5500 3100
Wire Wire Line
	5050 3000 5050 3100
Connection ~ 5050 3100
$Comp
L C C5
U 1 1 53C0FBAE
P 5300 2800
F 0 "C5" H 5300 2900 40  0000 L CNN
F 1 "100n" H 5306 2715 40  0000 L CNN
F 2 "~" H 5338 2650 30  0000 C CNN
F 3 "~" H 5300 2800 60  0000 C CNN
	1    5300 2800
	1    0    0    -1  
$EndComp
Wire Wire Line
	5300 2600 5300 2300
Connection ~ 5300 2300
Connection ~ 5300 3200
Connection ~ 9700 4250
Connection ~ 4800 2500
Connection ~ 3650 4750
Connection ~ 4250 4750
Connection ~ 6900 2900
Connection ~ 2400 2300
Wire Wire Line
	8250 3500 10100 3500
Wire Wire Line
	8250 3400 9600 3400
Wire Wire Line
	9600 3400 9600 3600
Wire Wire Line
	9600 3600 10100 3600
Wire Wire Line
	6400 3500 7250 3500
Wire Wire Line
	6600 3400 7250 3400
Wire Wire Line
	7250 3750 7150 3750
Wire Wire Line
	7150 4600 7150 4650
Connection ~ 7150 4650
Wire Wire Line
	9700 4000 9700 4250
Connection ~ 8900 4650
Wire Wire Line
	10100 3700 8700 3700
Wire Wire Line
	8700 3700 8700 4650
Connection ~ 8700 4650
Wire Wire Line
	6900 2900 8450 2900
Wire Wire Line
	8450 2900 8450 3750
Wire Wire Line
	8250 3750 8600 3750
Wire Wire Line
	8600 3750 8600 4250
Wire Wire Line
	8600 4250 8900 4250
Connection ~ 8450 3750
Wire Wire Line
	2600 5300 5300 5300
Connection ~ 4600 5300
Connection ~ 4250 5300
Connection ~ 3650 5300
Connection ~ 8450 4650
Connection ~ 8900 4250
Wire Wire Line
	7150 3750 7150 4100
$EndSCHEMATC
