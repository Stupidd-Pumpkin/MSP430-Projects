This file is to give a tour of the .c files available in the same folder as this one.
All these files are used to code msp430fr2xxx family of microcontrollers released by Texas Instruments. 
All thse codes have been tested on msp430fr2476.
This set of codes were written with intentions of developing nMPPT algorithm for ultrasonic wireless communication during "2019" by Sameer Mohammed Shaik.
All the test codes are available to catch up with the functionality of each module in the microcontroller and each funciton in the succeding codes.

Given below are the file names and their corresponding functions in the order of their complexity, starting from simplest:

(1) RGB_LED.c			:	GPIO Test code. RGB LEDs on the evaluation board of msp430fr2xxx will blink in rotation
(2) eCOMP.c 			:	Code to test only the enhanced comparator (eCOMP) module
(3) ADC.c 				:	Code to test only the Analog to Digital Converter (ADC) module
(4) ADC2.c 				:	Code to debug Clock speeds and continous ADC sampling.
(5)	Timer_PWM.c			:	Code to test timer modules and generate PWM outputs
(6) Timer_ADC.c 		:	Code to test Analog to Digital Converter (ADC) module periodically sampling based on timer rollover
(7) Timer_ADC_Pulse.c 	:	Equivalent to 'Timer_ADC.c' function but now produces a high frequency pulse if input signal > Vcc/2
(8) RTC_eCOMP_Pulse.c 	:	Similar to 'Timer_ADC_Pulse.c', but uses RTC instead of timer and eCOMP instead of ADC to drastically increase efficiency
(9) Freq_Sweep.c 		:	Code if you want to perform Frequency Sweep and test it seperately
(10) Power_FS.c 		:	Preliminary code for power monitoring and frequency sweep (no hysteresis and no averaged adc sampling)
(11) Power_FS_Exint.c	:	Works similar to 'Power_FS.c' but needs an external interrupt to wakeup. Useful if external power monitoring system is used.
*(11) Power_FS2.c 		:	Final code for power monitoring and frequency sweep
*(12) Power_Rx.c 		:	Code to test power monitoring and data receiving
(13) Power_Rx2.c 		:	Supplimentary code to test power monitoring and data receiving
(14) Tx.c 				:	Code to test one time data transmission
(15) Tx2.c 				:	Similar to 'Tx.c', except now, data is transmitted repeatedly with RTC interrupt

The files with * are the final files.

Please refer the 'User guide for msp430fr2xxx family of micro controllers' from TI and 'msp430fr2476.h' to understand registers of the microcontrollers and their functions.