/*
  date: 20/03/2023
  team: BK-AIRBUS
  subject: Embedded system designs
*/

/*****************************
   version: beta 0.1
   F-CPU: 16MHz
   microcontroller: ATmega328p
 *****************************/

#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//Define time remain state of system 
#define TIME_REMAIN_STATE_1 3  		//s
#define TIME_REMAIN_STATE_2 15  	//s
#define TIME_REMAIN_STATE_3 5 		//s
#define TIME_REMAIN_STATE_4 30		//s
#define TIME_REMAIN_STATE_5	15   //s
#define TIME_DEBOUNCE 20   //ms
//end define time remain state of system
//Globle Variable 
char stateMenuLcd = 0;
unsigned char g_checkUnlock = 0;
bool testting = 0;
unsigned char g_checkLock = 0;
unsigned char g_remainTestKeyPad = 0;
unsigned char g_errorSystem = 0;
unsigned char g_errorCode[4] = {0,0,0,0};
unsigned char g_operationCode = 0;
unsigned char g_stateCodeLcd = 0;
bool g_changeStateLcd = 1;
bool g_changeSec = 0;
bool g_firstInit = 1;
bool g_priorityState = 0;
unsigned char g_remainTimeLcd = 0;
unsigned char g_remainTimeLcdMili = 0;
unsigned char g_vPort = 0;
unsigned char g_timeMili = 0;
unsigned char g_timeSec = 0;
unsigned char g_timeSecOld = 0;
unsigned char g_codeSet[4] = {0,0,0,0};
unsigned char g_codeEnter[4] = {1,1,1,1};
unsigned char g_codeTemp[4] = {0,0,0,0};
unsigned char g_poiterOutside = 0;
unsigned char g_poiterInside = 0;
//End define global variable
//List function hardwave interface
void setVPort(unsigned char input); //function to write to virtual port 
void setLCDPort(unsigned char input); //function to write data to LCD port
void writeDataLCD(unsigned char data); //function write data to LCD
void writeCommandLCD(unsigned char command); //function write command to LCD
void setBitPB(unsigned char bit, bool logic); //function to write bit to port B
void setBitPD(unsigned char bit, bool logic); //function to write bit to port D
void ledOpen(bool input);
void ledFail(bool input);
void ledAllow(bool input);
void ledDenied(bool input);
void ledLock(bool input);
void ledUnlock(bool input);
void buzzer(bool input);
void soundPushButton();
//End list function hardwave interface
//List function sorfware
void scanKey();
unsigned char scanKeyInside();
unsigned char scanKeyOutside();
bool checkError();
bool checkCode();
void printLcdU();
void peintLcdL();
void operationSystem();
void operationLcd();
void overString(char input1[17], char input2[17]);
void stateTest();
void testLight();
unsigned char testDisplay();
unsigned char decemalConvertChuc(unsigned char input);
unsigned char decemalConvertDonVi(unsigned char input);
unsigned char testKeyPad();
//End list function sorfware
//Data of LCD display
char stringLcdU[17];
char stringLcdL[17];
//end define
//Interupt function lock or unlock door
ISR(INT0_vect){ 	//Unlock function / allow to access cockpit
	if(testting){
		g_checkUnlock = 1;
	}else{
		g_operationCode = 3; 	//unlock mode
		g_timeSec = TIME_REMAIN_STATE_3;
		g_stateCodeLcd = 2;
		g_changeStateLcd = 1;
	}
}

ISR(INT1_vect){	//lock function / denied to access cockpit
	if(testting){
		g_checkLock = 1;
	}else{
		g_operationCode = 2; 	//lock mode
		g_timeSec = TIME_REMAIN_STATE_2;
		g_stateCodeLcd = 2;
		g_changeStateLcd = 1;
	}
}
//End define function interupt
//Interupt function timer of system
ISR(TIMER0_OVF_vect){	//timer 0 use for timer of system with 10ms for interrupt
	TCNT0=0x63;
	if(g_timeMili > 0 || g_timeSec > 0){
		if(g_timeMili == 0 && g_timeSec > 0){
			g_timeSec --;
			g_changeSec = 1;
			g_timeMili = 99;
		}else if(g_timeMili > 0){
			g_timeMili --;
		}
	}
	if(g_remainTimeLcdMili > 0 || g_remainTimeLcd > 0){
		if(g_remainTimeLcdMili == 0 && g_remainTimeLcd > 0){
			g_remainTimeLcd --;
			g_remainTimeLcdMili = 99;
		}else if(g_remainTimeLcdMili > 0){
			g_remainTimeLcdMili --;
		}
		if(g_remainTimeLcd == 0 && g_remainTimeLcdMili == 0){
			if(testting == 0){
				writeCommandLCD(0x08);
			}
		}
	}
}
//end define function timer of system
//define function initial hardware 
void initialTimer0();
void initialLcd();
void initialExternalInteruprt();
void initialPinConfig();	//config pin
void initializationSystem();
//End define function initial hardware
/**************************
*	0: normal operation
*	1: request access normal 
*	2: lock mode
*	3: unlock mode
*	4: emergency request access mode
*	5: door fail/ open emergency success
*	6: test mode
* 7: initialation not complete
***************************/ 

int main(void){
	initializationSystem();
	while(1){
		scanKey();
		operationSystem();
		operationLcd();
		if((PIND&(1<<4))==0){
			testLight();
			while((PIND&(1<<4))==0){
				stateTest();
			}
			g_stateCodeLcd = 2;
			g_changeStateLcd = 1;
		}
	}
}
void setBitPB(unsigned char bit, bool logic){
 	if(logic){
 	PORTB |= 0x01 << bit;
 	}else{
  	PORTB &= ~(0x01<<bit);
 	}
}
void setBitPD(unsigned char bit, bool logic){
 	if(logic){
 		PORTD |= 0x01 << bit;
 	}else{
   	PORTD &= ~(0x01<<bit);
 	}
}
void setVPort(unsigned char input){
	g_vPort = input;
 	char i = 0;
 	setBitPB(2,0);
 	setBitPB(0,0);
 	for(i=7;i>=0;i--){
  	setBitPB(0,0);
   	if((input >> i)&0x01){
    		setBitPB(1,1);
   	}else{
     		setBitPB(1,0);
   	}
  		setBitPB(0,1);
 	}
 	setBitPB(2,1);
}
void setLCDPort(unsigned char input){
   	char i = 0;
   	setBitPB(3,0);
   	setBitPB(0,0);
		for(i=7;i>=0;i--){
	   	setBitPB(0,0);
	    if((input >> i)&0x01){
	        setBitPB(1,1);
	    }else{
	       	setBitPB(1,0);
	    }
    	setBitPB(0,1);
   	}
   	setBitPB(3,1);
}
void writeDataLCD(unsigned char data){
  	setBitPD(5,1);
  	setBitPD(6,0);
  	setLCDPort(data);
  	setBitPD(6,1);
  	_delay_ms(2);
  	setBitPD(6,0);
}
void writeCommandLCD(unsigned char command){
		setBitPD(5,0);
		setBitPD(6,0);
  	setLCDPort(command);
  	setBitPD(6,1);
  	_delay_ms(2);
  	setBitPD(6,0);
}
void initialTimer0(){
	// Timer/Counter 0 initialization
	// Clock source: System Clock
	// Clock value: 7.813 kHz
	// Mode: Normal top=0xFF
	// OC0A output: Disconnected
	// OC0B output: Disconnected
	// Timer Period: 9.984 ms
	TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
	TCCR0B=(0<<WGM02) | (1<<CS02) | (0<<CS01) | (1<<CS00);		//source clock / 1024
	TCNT0=0x63;
	OCR0A=0x00;
	OCR0B=0x00;
	// Timer/Counter 0 Interrupt(s) initialization
	TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (1<<TOIE0);	
}
void initialExternalInteruprt(){
	// External Interrupt(s) initialization
	// INT0: On
	// INT0 Mode: Falling Edge
	// INT1: On
	// INT1 Mode: Falling Edge
	// Interrupt on any change on pins PCINT0-7: Off
	// Interrupt on any change on pins PCINT8-14: Off
	// Interrupt on any change on pins PCINT16-23: Off
	EICRA=(1<<ISC11) | (0<<ISC10) | (1<<ISC01) | (0<<ISC00);
	EIMSK=(1<<INT1) | (1<<INT0);
	EIFR=(1<<INTF1) | (1<<INTF0);
	PCICR=(0<<PCIE2) | (0<<PCIE1) | (0<<PCIE0);
}
unsigned char scanKeyInside(){
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xfe);
	_delay_us(1);
	if((PINC&(1<<3))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<3))==0);
		soundPushButton();
		return 0x31;
	}else if((PINC&(1<<4))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<4))==0);
		soundPushButton();
		return 0x32;
	}else if((PINC&(1<<5))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<5))==0);
		soundPushButton();
		return 0x33;
	}
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xfd);
	_delay_us(1);
	if((PINC&(1<<3))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<3))==0);
		soundPushButton();
		return 0x34;
	}else if((PINC&(1<<4))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<4))==0);
		soundPushButton();
		return 0x35;
	}else if((PINC&(1<<5))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<5))==0);
		soundPushButton();
		return 0x36;
	}
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xfb);
	_delay_us(1);
	if((PINC&(1<<3))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<3))==0);
		soundPushButton();
		return 0x37;
	}else if((PINC&(1<<4))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<4))==0);
		soundPushButton();
		return 0x38;
	}else if((PINC&(1<<5))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<5))==0);
		soundPushButton();
		return 0x39;
	}
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xf7);
	_delay_us(1);
	if((PINC&(1<<3))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<3))==0);
		soundPushButton();
		return 0x2a;
	}else if((PINC&(1<<4))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<4))==0);
		soundPushButton();
		return 0x30;
	}else if((PINC&(1<<5))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<5))==0);
		soundPushButton();
		return 0x23;
	}
	return 0xff;
}
void scanKey(){
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xfe);
	_delay_us(1);
	if(g_operationCode == 0){
		if((PINC&(1<<0))==0){
			_delay_ms(TIME_DEBOUNCE);
			while((PINC&(1<<0))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 1;
			g_poiterOutside ++;
		}else if((PINC&(1<<1))==0){
			_delay_ms(TIME_DEBOUNCE);
			while((PINC&(1<<1))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 2;
			g_poiterOutside ++;
		}else if((PINC&(1<<2))==0){
			_delay_ms(TIME_DEBOUNCE);
			while((PINC&(1<<2))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 3;
			g_poiterOutside ++;
		}
		if(g_poiterOutside == 5){
			g_poiterOutside = 0;
		}
	}
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xfd);
	_delay_us(1);
	if(g_operationCode == 0){
		if((PINC&(1<<0))==0){
			_delay_ms(TIME_DEBOUNCE);
			while((PINC&(1<<0))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 4;
			g_poiterOutside ++;
		}else if((PINC&(1<<1))==0){
			_delay_ms(TIME_DEBOUNCE);
			while((PINC&(1<<1))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 5;
			g_poiterOutside ++;
		}else if((PINC&(1<<2))==0){
			_delay_ms(TIME_DEBOUNCE);
			while((PINC&(1<<2))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 6;
			g_poiterOutside ++;
		}
		if(g_poiterOutside == 5){
			g_poiterOutside = 0;
		}
	}
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xfb);
	_delay_us(1);
	if(g_operationCode == 0){
		if((PINC&(1<<0))==0){
			_delay_ms(TIME_DEBOUNCE);
			while((PINC&(1<<0))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 7;
			g_poiterOutside ++;
		}else if((PINC&(1<<1))==0){
			_delay_ms(TIME_DEBOUNCE);
			while((PINC&(1<<1))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 8;
			g_poiterOutside ++;
		}else if((PINC&(1<<2))==0){
			_delay_ms(TIME_DEBOUNCE);
			while((PINC&(1<<2))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 9;
			g_poiterOutside ++;
		}
		if(g_poiterOutside == 5){
			g_poiterOutside = 0;
		}
	}
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xf7);
	_delay_us(1);
	if(g_operationCode == 0){
		if((PINC&(1<<0))==0){
			_delay_ms(TIME_DEBOUNCE);
			while((PINC&(1<<0))==0);
			soundPushButton();
			g_poiterOutside = 0;
		}else if((PINC&(1<<1))==0){
			_delay_ms(TIME_DEBOUNCE);
			while((PINC&(1<<1))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 0;
			g_poiterOutside ++;
		}else if((PINC&(1<<2))==0){
			_delay_ms(TIME_DEBOUNCE);
			while((PINC&(1<<2))==0);
			if(g_stateCodeLcd != 4){
				if(g_poiterOutside == 4){
					g_poiterOutside = 0;
					if(checkCode()){
						g_operationCode = 4;
						g_timeSec = TIME_REMAIN_STATE_4;
						g_priorityState = 1;
						overString(stringLcdU," EMERGENCY:    s"); //13 and 14
						stringLcdU[13] = 0x30 + decemalConvertChuc(TIME_REMAIN_STATE_4);
						stringLcdU[14] = 0x30 + decemalConvertDonVi(TIME_REMAIN_STATE_4);
						overString(stringLcdL,"SwitchToLock now");
						g_remainTimeLcd = 5;
						writeCommandLCD(0x0c);
						printLcdU();
						printLcdL();
					}
				}else if(g_operationCode != 2){
					g_poiterOutside = 0;
					g_operationCode = 1;
					g_timeSec = TIME_REMAIN_STATE_1;
					g_priorityState = 1;
					overString(stringLcdU," NORMAL REQUEST ");
					overString(stringLcdL," Lock or unlock ");
					printLcdU();
					printLcdL();
					writeCommandLCD(0x0c);
					g_remainTimeLcd = TIME_REMAIN_STATE_1;
				}
			}
		}
		if(g_poiterOutside == 5){
			g_poiterOutside = 0;
		}
	}
}
void soundPushButton(){
	setBitPD(7,1);
	_delay_ms(50);
	setBitPD(7,0);
}
bool checkCode(){
	unsigned char i = 0;
	for(i = 0; i < 4; i++){
		if(g_codeEnter[i] != g_codeSet[i]){
			return false;
		}
	}
	return true;
}
void operationSystem(){
	switch(g_operationCode){
		case 0:		//normal state
			if(g_stateCodeLcd == 0){
				g_operationCode = 7;
			}
			buzzer(0);
			ledOpen(0);
			ledFail(0);
			ledAllow(0);
			ledDenied(0);
			ledLock(1);
			ledUnlock(0);
			break;
		case 1:
			if((g_timeSec==0)&&(g_timeMili==0)){
				g_operationCode = 0;
			}else{
				if(g_timeMili < 50){
					buzzer(1);
				}else{
					buzzer(0);
				}
				ledOpen(0);
				ledFail(0);
				ledAllow(0);
				ledDenied(0);
				ledLock(1);
				ledUnlock(0);
			}
			break;
		case 2:
			if((g_timeSec==0)&&(g_timeMili==0)){
				g_operationCode = 0;
			}else{
				buzzer(0);
				ledOpen(0);
				ledFail(0);
				ledAllow(0);
				ledDenied(1);
				ledLock(1);
				ledUnlock(0);
			}
			break;
		case 3:
			if((g_timeSec==0)&&(g_timeMili==0)){
				g_operationCode = 0;
			}else{
				buzzer(0);
				ledOpen(1);
				ledFail(0);
				ledAllow(1);
				ledDenied(0);
				ledLock(0);
				ledUnlock(1);
			}
			break;
		case 4:
			g_remainTimeLcd = 5;
			if((g_timeSec==0)&&(g_timeMili==0)){
				buzzer(1);
				_delay_ms(1000);
				buzzer(0);
				g_operationCode = 5;
				g_timeSec = TIME_REMAIN_STATE_5;
				g_remainTimeLcd = TIME_REMAIN_STATE_5;
				overString(stringLcdU, "   DOOR FAIL !  ");
				overString(stringLcdL, "Security failed ");
				printLcdU();
				printLcdL();
				writeCommandLCD(0x0c);
			}else{
				if((g_timeMili/10)%2){
					buzzer(1);
					ledOpen(1);
					ledAllow(1);
				}else{
					buzzer(0);
					ledOpen(0);
					ledAllow(0);
				}
				if(g_changeSec){
					writeCommandLCD(0x80 + 13);
					writeDataLCD(0x30 + decemalConvertChuc(g_timeSec));
					writeDataLCD(0x30 + decemalConvertDonVi(g_timeSec));
				}
				if(g_priorityState){
					if(g_timeMili < 50){
						writeCommandLCD(0x08);
					}else{
						writeCommandLCD(0x0c);
					}
				}
				ledFail(0);
				ledDenied(0);
				ledLock(1);
				ledUnlock(0);
			}
			break;
		case 5:
			if((g_timeSec==0)&&(g_timeMili==0)){
				g_operationCode = 0;
			}else{
				g_remainTimeLcd = 3;
				if(g_priorityState){
					if(g_timeMili < 50){
						writeCommandLCD(0x0c);
					}else{
						writeCommandLCD(0x08);
					}
				}
				buzzer(0);
				ledOpen(1);
				ledFail(1);
				ledAllow(1);
				ledDenied(0);
				ledLock(0);
				ledUnlock(1);
			}
			break;
		case 6:
			buzzer(1);
			ledOpen(1);
			ledFail(1);
			ledAllow(1);
			ledDenied(1);
			ledLock(1);
			ledUnlock(1);
			break;
		case 7:
			buzzer(0);
			ledOpen(1);
			ledFail(0);
			ledAllow(0);
			ledDenied(0);
			ledLock(0);
			ledUnlock(1);
			if(g_stateCodeLcd != 0){
				g_operationCode = 0;
			}
			break;
	}
}
void operationLcd(){
	unsigned char key = scanKeyInside();
	if(g_changeStateLcd){
		key = 0xfe;
		g_changeStateLcd = 0;
	}
	if(key != 0xff){
		g_priorityState = 0;
	}
	if(!g_priorityState){
		if(key != 0xff){
			g_remainTimeLcd = 10;
			writeCommandLCD(0x0c);
			if(g_stateCodeLcd == 0){
				overString(stringLcdU, " Initialization ");
				overString(stringLcdL, "1.Init     2.Off");
				printLcdU();
				printLcdL();
				if(key == 0x31){
					g_stateCodeLcd = 1;
					g_changeStateLcd = 1;
					key = 0xff;
					overString(stringLcdL, "Enter code:     ");
				}else if(key == 0x32){
					writeCommandLCD(0x08);
				}
			}else if(g_stateCodeLcd == 1){
				g_firstInit = 0;
				writeCommandLCD(0x0f);
				unsigned char i = 0;
				overString(stringLcdU, "Active code:    ");
				if(key >= 0x30 && key <= 0x39){
					if(g_poiterInside != 4){
						stringLcdL[11+g_poiterInside] = key;
					}
					g_codeTemp[g_poiterInside] = key - 0x30;
					g_poiterInside ++;
					if(g_poiterInside == 5){
						g_poiterInside = 0;
					}
				}else if(key == 0x23){
					if(g_poiterInside == 4){
						overString(stringLcdL, "Enter code:     ");
						for(i=0; i<4; i++){
							g_codeSet[i] = g_codeTemp[i];
						}
					}
				}
				for(i=0; i<4; i++){
					stringLcdU[i+12] = g_codeSet[i]+0x30;
				}
				printLcdU();
				printLcdL();
				if(key == 0x2a){
					g_stateCodeLcd = 2;
					g_changeStateLcd = 1;
					key = 0xff;
				}
			}else if(g_stateCodeLcd == 2){
				if(g_firstInit){
					g_stateCodeLcd = 0;
					g_changeStateLcd = 1;
				}
				overString(stringLcdU, "      Menu      ");
				if(checkError()){
					stringLcdU[13] = 0x3c;
					stringLcdU[14] = 0x21;
					stringLcdU[15] = 0x3e;
				}
				if(stateMenuLcd == 0){
					overString(stringLcdL, "1.Edit  2.Strict");
				}else if(stateMenuLcd == 1){
					overString(stringLcdL, "  3.Test keypad ");
				}else if(stateMenuLcd == 2){
					overString(stringLcdL, " 4.Test display ");
				}else if(stateMenuLcd == 3){
					overString(stringLcdL, " 5. Information ");
				}
				printLcdU();
				printLcdL();
				if(key == 0x31){
					g_stateCodeLcd = 1;
					g_changeStateLcd = 1;
					key = 0xff;
					overString(stringLcdL, "Enter code:     ");
				}else if(key == 0x32){
					g_stateCodeLcd = 3;
					g_changeStateLcd = 1;
					key = 0xff;
				}else if(key == 0x2a){
					stateMenuLcd --;
					if(stateMenuLcd < 0){
						stateMenuLcd = 3;
					}
					g_changeStateLcd = 1;
				}else if(key == 0x23){
					stateMenuLcd ++;
					if(stateMenuLcd > 3){
						stateMenuLcd = 0;
					}
					g_changeStateLcd = 1;
				}else if(key == 0x33){
					g_errorCode[0] = 0;
					g_errorCode[1] = 0;
					g_errorCode[2] = 0;
					g_errorSystem += testKeyPad();
				}else if(key == 0x34){
					g_errorCode[3] = 0;
					g_errorSystem += testDisplay();
				}else if(key == 0x35){
					g_stateCodeLcd = 5;
					stateMenuLcd = 0;
					g_changeStateLcd = 1;
					key = 0xff;
				}
			}else if(g_stateCodeLcd == 3){
				overString(stringLcdU, "  Strict mode   ");
				overString(stringLcdL, "Confirm press # ");
				printLcdU();
				printLcdL();
				if(key == 0x23){
					g_stateCodeLcd = 4;
					g_changeStateLcd = 1;
					key = 0xff;
				}else if(key == 0x2a){
					g_stateCodeLcd = 2;
					g_changeStateLcd = 1;
					key = 0xff;
				}
			}else if(g_stateCodeLcd == 4){
				overString(stringLcdU, "  STRICT  MODE  ");
				overString(stringLcdL, "----------------");
				printLcdU();
				printLcdL();
				if(key == 0x2a){
					g_stateCodeLcd = 2;
					g_changeStateLcd = 1;
					key = 0xff;
				}
			}else if(g_stateCodeLcd == 5){
				if(key != 0x23 && key != 0x2a){
					if(stateMenuLcd == 0){
						if(checkError()){
							overString(stringLcdU, "System in ERROR ");
							overString(stringLcdL, "Press # to view ");
							printLcdU();
							printLcdL();
							buzzer(1);
							_delay_ms(500);
							buzzer(0);
						}else{
							overString(stringLcdU, "System in NORMAL");
							overString(stringLcdL, "Press # to view ");
							printLcdU();
							printLcdL();
						}
					}else if(stateMenuLcd == 1){
						overString(stringLcdU, "1. keypad inside");
						if(g_errorCode[0] == 0){
							overString(stringLcdL, "Erything is okay");
							printLcdU();
							printLcdL();
						}else{
							overString(stringLcdL, " Run with ERROR ");
							printLcdU();
							printLcdL();
							buzzer(1);
							_delay_ms(500);
							buzzer(0);
						}
					}else if(stateMenuLcd == 2){
						overString(stringLcdU, "2.keypad outside");
						if(g_errorCode[1] == 0){
							overString(stringLcdL, "Erything is okay");
							printLcdU();
							printLcdL();
						}else{
							overString(stringLcdL, " Run with ERROR ");
							printLcdU();
							printLcdL();
							buzzer(1);
							_delay_ms(500);
							buzzer(0);
						}
					}else if(stateMenuLcd == 3){
						overString(stringLcdU, "3.Switch control");
						if(g_errorCode[2] == 0){
							overString(stringLcdL, "Erything is okay");
							printLcdU();
							printLcdL();
						}else{
							overString(stringLcdL, " Run with ERROR ");
							printLcdU();
							printLcdL();
							buzzer(1);
							_delay_ms(500);
							buzzer(0);
						}
					}else if(stateMenuLcd == 4){
						overString(stringLcdU, "4.System display");
						if(g_errorCode[3] == 0){
							overString(stringLcdL, "Erything is okay");
							printLcdU();
							printLcdL();
						}else{
							overString(stringLcdL, " Run with ERROR ");
							printLcdU();
							printLcdL();
							buzzer(1);
							_delay_ms(500);
							buzzer(0);
						}
					}
				}else{
					if(key == 0x23){
						g_changeStateLcd = 1;
						stateMenuLcd ++;
						if(stateMenuLcd > 4){
							stateMenuLcd = 0;
						}
					}else if(key == 0x2a){
						g_changeStateLcd = 1;
						stateMenuLcd = 0;
						g_stateCodeLcd = 2;
					}
				}
			}
		}
	}
}
void ledOpen(bool input){
	if(input){
		setVPort(g_vPort|(1<<6));
	}else{
		setVPort(g_vPort&(~(1<<6)));
	}
}
void ledFail(bool input){
	if(input){
		setVPort(g_vPort|(1<<7));
	}else{
		setVPort(g_vPort&(~(1<<7)));
	}
}
void ledAllow(bool input){
	if(input){
		setVPort(g_vPort|(1<<4));
	}else{
		setVPort(g_vPort&(~(1<<4)));
	}
}
void ledDenied(bool input){
	if(input){
		setVPort(g_vPort|(1<<5));
	}else{
		setVPort(g_vPort&(~(1<<5)));
	}
}
void ledLock(bool input){
	if(input){
		PORTB = PORTB|(1<<4);
	}else{
		PORTB = PORTB&(~(1<<4));
	}
}
void ledUnlock(bool input){
	if(input){
		PORTB = PORTB|(1<<5);
	}else{
		PORTB = PORTB&(~(1<<5));
	}
}
void buzzer(bool input){
	if(input){
		setBitPD(7,1);
	}else{
		setBitPD(7,0);
	}
}
void initialPinConfig(){
	DDRB |= 0x3f; //bit 0 -> 5 is output
	DDRC &= 0xc0; //bit 0 -> 5 is input
	PORTC = 0xFF; //all 1 to pull up
	DDRD |= 0xe0; //bit 7 -> 5 is output else is input 
	PORTD |= 0x11;
}
void initialLcd(){
	g_remainTimeLcd = 20;
	writeCommandLCD(0x0c);
	writeCommandLCD(0x38);
	_delay_ms(2000);
	overString(stringLcdU, " Initialization ");
	overString(stringLcdL, "Init LCD success");
	printLcdU();
	printLcdL();
	_delay_ms(1000);
}
void initializationSystem(){
	SREG |= 0x80;  //Global interrupt enable
	initialPinConfig();
	initialLcd();
	overString(stringLcdL, "PinConfigSuccess");
	printLcdL();
	_delay_ms(200);
	initialExternalInteruprt();
	overString(stringLcdL, "InterruptSuccess");
	printLcdL();
	_delay_ms(300);
	initialTimer0();
	overString(stringLcdL, "InitTimerSuccess");
	printLcdL();
	_delay_ms(100);
	overString(stringLcdU, "Vision: Beta 0.5");
	overString(stringLcdL, " Please press # ");
	printLcdU();
	printLcdL();
	buzzer(1);
	_delay_ms(50);
	buzzer(0);
	while(scanKeyInside() != 0x23);
}
void printLcdU(){
	unsigned char i = 0;
	writeCommandLCD(0x80);
	for(i = 0; i<16 ; i++){
		writeDataLCD(stringLcdU[i]);
	}
}
void printLcdL(){
	unsigned char i = 0;
	writeCommandLCD(0xc0);
	for(i = 0; i<16 ; i++){
		writeDataLCD(stringLcdL[i]);
	}
}
void overString(char input1[17], char input2[17]){
	unsigned char i = 0;
	for(i=0; i<17; i++){
		input1[i] = input2[i];
	}
}
void stateTest(){
	buzzer(1);
	_delay_ms(25);
	buzzer(0);
	_delay_ms(175);
	g_remainTimeLcd = 2;
}
void testLight(){
	_delay_ms(200);
	writeCommandLCD(0x0c);
	g_remainTimeLcd = 10;
	overString(stringLcdU," TEST OPERATION ");
	overString(stringLcdL," only for test  ");
	printLcdU();
	printLcdL();
	ledOpen(1);
	ledFail(1);
	ledAllow(1);
	ledDenied(1);
	ledLock(1);
	_delay_ms(500);
	ledUnlock(1);
	_delay_ms(500);
	ledUnlock(0);
	_delay_ms(1000);
}
unsigned char decemalConvertChuc(unsigned char input){
	return input/10;
}
unsigned char decemalConvertDonVi(unsigned char input){
	return input%10;
}
unsigned char testKeyPad(){
	testting = 1;
	_delay_ms(500);
	overString(stringLcdU, "TEST KEYP INSIDE");
	unsigned char i = 0;
	unsigned char parameter = 0;
	unsigned char result = 0;
	for(i = 0; i < 12; i++){
		g_timeSec = 4;
		overString(stringLcdL, "Press  .... sec ");
		if(i == 10){
			parameter = 0x2a;
			stringLcdL[6] = 0x2a;
		}else if(i == 11){
			stringLcdL[6] = 0x23;
			parameter = 0x23;
		}else{
			stringLcdL[6] = i + 0x30;
			parameter = i + 0x30;
		}
		printLcdU();
		printLcdL();
		while(scanKeyInside() != parameter){
			if(g_changeSec == 1){
				g_changeSec == 0;
				writeCommandLCD(0xcb);
				writeDataLCD(0x30 + g_timeSec);
			}
			if(g_timeSec == 0 && g_timeMili == 0){
				result ++;
				g_errorCode[0] = 1;
				soundPushButton();
				soundPushButton();
				break;
			}
		}
	}
	overString(stringLcdU, "TEST KEY OUTSIDE");
	for(i = 0; i < 12; i++){
		g_timeSec = 4;
		overString(stringLcdL, "Press  .... sec ");
		if(i == 10){
			parameter = 0x2a;
			stringLcdL[6] = 0x2a;
		}else if(i == 11){
			stringLcdL[6] = 0x23;
			parameter = 0x23;
		}else{
			stringLcdL[6] = i + 0x30;
			parameter = i + 0x30;
		}
		printLcdU();
		printLcdL();
		while(scanKeyOutside() != parameter){
			if(g_changeSec == 1){
				g_changeSec == 0;
				writeCommandLCD(0xcb);
				writeDataLCD(0x30 + g_timeSec);
			}
			if(g_timeSec == 0 && g_timeMili == 0){
				result ++;
				g_errorCode[1] = 1;
				soundPushButton();
				soundPushButton();
				break;
			}
		}
	}
	g_checkLock = 0;
	g_checkUnlock = 0;
	overString(stringLcdU, "  TEST SWITCH   ");
	overString(stringLcdL, "Unlock test  sec");
	printLcdU();
	printLcdL();
	g_timeSec = 5;
	while(g_checkUnlock == 0){
		if(g_changeSec == 1){
			g_changeSec = 0;
			writeCommandLCD(0xcb);
			writeDataLCD(0x30 + g_timeSec);
		}
		if(g_timeSec == 0 && g_timeMili == 0){
			result ++;
			g_errorCode[2] = 1;
			soundPushButton();
			soundPushButton();
			break;
		}
	}
	overString(stringLcdU, "  TEST SWITCH   ");
	overString(stringLcdL, "Lock test    sec");
	printLcdU();
	printLcdL();
	g_timeSec = 5;
	while(g_checkLock == 0){
		if(g_changeSec == 1){
			g_changeSec = 0;
			writeCommandLCD(0xcb);
			writeDataLCD(0x30 + g_timeSec);
		}
		if(g_timeSec == 0 && g_timeMili == 0){
			result ++;
			g_errorCode[2] = 1;
			soundPushButton();
			soundPushButton();
			break;
		}
	}
	testting = 0;
	g_changeStateLcd = 1;
	return result;
}

unsigned char scanKeyOutside(){
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xfe);
	_delay_us(1);
	if((PINC&(1<<0))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<0))==0);
		soundPushButton();
		return 0x31;
	}else if((PINC&(1<<1))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<1))==0);
		soundPushButton();
		return 0x32;
	}else if((PINC&(1<<2))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<2))==0);
		soundPushButton();
		return 0x33;
	}
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xfd);
	_delay_us(1);
	if((PINC&(1<<0))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<0))==0);
		soundPushButton();
		return 0x34;
	}else if((PINC&(1<<1))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<1))==0);
		soundPushButton();
		return 0x35;
	}else if((PINC&(1<<2))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<2))==0);
		soundPushButton();
		return 0x36;
	}
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xfb);
	_delay_us(1);
	if((PINC&(1<<0))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<0))==0);
		soundPushButton();
		return 0x37;
	}else if((PINC&(1<<1))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<1))==0);
		soundPushButton();
		return 0x38;
	}else if((PINC&(1<<2))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<2))==0);
		soundPushButton();
		return 0x39;
	}
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xf7);
	_delay_us(1);
	if((PINC&(1<<0))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<0))==0);
		soundPushButton();
		return 0x2a;
	}else if((PINC&(1<<1))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<1))==0);
		soundPushButton();
		return 0x30;
	}else if((PINC&(1<<2))==0){
		_delay_ms(TIME_DEBOUNCE);
		while((PINC&(1<<2))==0);
		soundPushButton();
		return 0x23;
	}
	return 0xff;
}
unsigned char testDisplay(){
	testting = 1;
	unsigned char result = 0;
	overString(stringLcdU, "  TEST DISPLAY  ");
	overString(stringLcdL, "Start testing   ");
	printLcdU();
	printLcdL();
	_delay_ms(100);
	writeCommandLCD(0xcd);
	writeDataLCD('.');
	_delay_ms(20);
	writeDataLCD('.');
	_delay_ms(50);
	writeDataLCD('.');
	_delay_ms(300);
	overString(stringLcdU,"                ");
	overString(stringLcdL,"                ");
	printLcdU();
	printLcdL();
	unsigned char i = 0;
	writeCommandLCD(0x80);
	for(i = 0; i < 16; i++){
		writeDataLCD(0xff);
		_delay_ms(50);
	}
	writeCommandLCD(0xc0);
	for(i = 0; i < 16; i++){
		writeDataLCD(0xff);
		_delay_ms(50);
	}
	_delay_ms(500);
	overString(stringLcdU, "Press * if error");
	overString(stringLcdL, "Else press #    ");
	soundPushButton();
	printLcdL();
	printLcdU();
	g_timeSec = 5;
	bool run = 1;
	unsigned char keyTemp = 0xff;
	do{
		keyTemp = scanKeyInside();
		if(g_timeSec == 0 && g_timeMili == 0){
			result ++;
			g_errorCode[0] = 1;
			g_errorCode[3] = 1;
			soundPushButton();
			soundPushButton();
			g_changeStateLcd = 1;
			run = 0; 
		}
		if(keyTemp == 0x2a){
			result ++;
			g_errorCode[3] = 1;
			g_changeStateLcd = 1;
			run = 0;
		}
		if(keyTemp == 0x23){
			g_errorCode[3] = 0;
			g_changeStateLcd = 1;
			run = 0;
		}
	}while(run);
	testting = 0;
	return result;
}
bool checkError(){
	unsigned char i = 0;
	unsigned error = 0;
	for(i = 0; i < 4; i++){
		error += g_errorCode[i];
	}
	if(error){
		return 1;
	}else{
		return 0;
	}
}