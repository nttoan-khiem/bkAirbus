/*
  date: 20/03/2023
  team: BK-AIRBUS
  subject: Embedded system designs
*/

/*****************************
   version: beta 0.1
   F-CPU: 8MHz
   microcontroller: ATmega328p
 *****************************/

#define F_CPU 8000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//Define time remain state of system 
#define TIME_REMAIN_STATE_1 3
#define TIME_REMAIN_STATE_2 15
#define TIME_REMAIN_STATE_3 5
#define TIME_REMAIN_STATE_4 30
#define TIME_REMAIN_STATE_5	15
//end define time remain state of system
//Globle Variable 
unsigned char g_operationCode = 0;
unsigned char g_stateCodeLcd = 0;
bool changeStateLcd = 1;
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
bool checkCode();
void printLcdU();
void peintLcdL();
void operationSystem();
void operationLcd();
void overString(char input1[17], char input2[17]);
//End list function sorfware
//Data of LCD display
char stringLcdU[17];
char stringLcdL[17];
//end define
//Interupt function lock or unlock door
ISR(INT0_vect){ 	//Unlock function / allow to access cockpit
	g_operationCode = 3; 	//unlock mode
	g_timeSec = TIME_REMAIN_STATE_3;
}

ISR(INT1_vect){	//lock function / denied to access cockpit
	g_operationCode = 2; 	//lock mode
	g_timeSec = TIME_REMAIN_STATE_2;
}
//End define function interupt
//Interupt function timer of system
ISR(TIMER0_OVF_vect){	//timer 0 use for timer of system with 10ms for interrupt
	TCNT0=0xB2;
	if(g_timeMili > 0 || g_timeSec > 0){
		if(g_timeMili == 0 && g_timeSec > 0){
			g_timeSec --;
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
			writeCommandLCD(0x08);
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
***************************/ 

int main(void){
	initializationSystem();
	while(1){
		scanKey();
		operationSystem();
		operationLcd();
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
	TCNT0=0xB2;
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
		_delay_ms(1);
		while((PINC&(1<<3))==0);
		soundPushButton();
		return 0x31;
	}else if((PINC&(1<<4))==0){
		_delay_ms(1);
		while((PINC&(1<<4))==0);
		soundPushButton();
		return 0x32;
	}else if((PINC&(1<<5))==0){
		_delay_ms(1);
		while((PINC&(1<<5))==0);
		soundPushButton();
		return 0x33;
	}
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xfd);
	_delay_us(1);
	if((PINC&(1<<3))==0){
		_delay_ms(1);
		while((PINC&(1<<3))==0);
		soundPushButton();
		return 0x34;
	}else if((PINC&(1<<4))==0){
		_delay_ms(1);
		while((PINC&(1<<4))==0);
		soundPushButton();
		return 0x35;
	}else if((PINC&(1<<5))==0){
		_delay_ms(1);
		while((PINC&(1<<5))==0);
		soundPushButton();
		return 0x36;
	}
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xfb);
	_delay_us(1);
	if((PINC&(1<<3))==0){
		_delay_ms(1);
		while((PINC&(1<<3))==0);
		soundPushButton();
		return 0x37;
	}else if((PINC&(1<<4))==0){
		_delay_ms(1);
		while((PINC&(1<<4))==0);
		soundPushButton();
		return 0x38;
	}else if((PINC&(1<<5))==0){
		_delay_ms(1);
		while((PINC&(1<<5))==0);
		soundPushButton();
		return 0x39;
	}
	g_vPort |= 0x0f;
	setVPort(g_vPort & 0xf7);
	_delay_us(1);
	if((PINC&(1<<3))==0){
		_delay_ms(1);
		while((PINC&(1<<3))==0);
		soundPushButton();
		return 0x2a;
	}else if((PINC&(1<<4))==0){
		_delay_ms(1);
		while((PINC&(1<<4))==0);
		soundPushButton();
		return 0x30;
	}else if((PINC&(1<<5))==0){
		_delay_ms(1);
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
			_delay_ms(1);
			while((PINC&(1<<0))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 1;
			g_poiterOutside ++;
		}else if((PINC&(1<<1))==0){
			_delay_ms(1);
			while((PINC&(1<<1))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 2;
			g_poiterOutside ++;
		}else if((PINC&(1<<2))==0){
			_delay_ms(1);
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
			_delay_ms(1);
			while((PINC&(1<<0))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 4;
			g_poiterOutside ++;
		}else if((PINC&(1<<1))==0){
			_delay_ms(1);
			while((PINC&(1<<1))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 5;
			g_poiterOutside ++;
		}else if((PINC&(1<<2))==0){
			_delay_ms(1);
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
			_delay_ms(1);
			while((PINC&(1<<0))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 7;
			g_poiterOutside ++;
		}else if((PINC&(1<<1))==0){
			_delay_ms(1);
			while((PINC&(1<<1))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 8;
			g_poiterOutside ++;
		}else if((PINC&(1<<2))==0){
			_delay_ms(1);
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
			_delay_ms(1);
			while((PINC&(1<<0))==0);
			soundPushButton();
			g_poiterOutside = 0;
		}else if((PINC&(1<<1))==0){
			_delay_ms(1);
			while((PINC&(1<<1))==0);
			soundPushButton();
			g_codeEnter[g_poiterOutside] = 0;
			g_poiterOutside ++;
		}else if((PINC&(1<<2))==0){
			_delay_ms(1);
			while((PINC&(1<<2))==0);
			writeDataLCD('s');
			if(g_stateCodeLcd != 4){
				if(g_poiterOutside == 4){
					g_poiterOutside = 0;
					writeDataLCD('4');
					if(checkCode()){
						g_operationCode = 4;
						g_timeSec = TIME_REMAIN_STATE_4;
					}
				}else if(g_operationCode != 2){
					g_poiterOutside = 0;
					g_operationCode = 1;
					g_timeSec = TIME_REMAIN_STATE_1;
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
			if((g_timeSec==0)&&(g_timeMili==0)){
				buzzer(1);
				_delay_ms(1000);
				buzzer(0);
				g_operationCode = 5;
				g_timeSec = TIME_REMAIN_STATE_5;
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
	if(changeStateLcd){
		key = 0xfe;
		changeStateLcd = 0;
	}
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
				changeStateLcd = 1;
				key = 0xff;
				overString(stringLcdL, "Enter code:     ");
			}else if(key == 0x32){
				writeCommandLCD(0x08);
			}
		}else if(g_stateCodeLcd == 1){
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
				changeStateLcd = 1;
				key = 0xff;
			}
		}else if(g_stateCodeLcd == 2){
			overString(stringLcdU, "      Menu      ");
			overString(stringLcdL, "1.Edit  2.Strict");
			printLcdU();
			printLcdL();
			if(key == 0x31){
				g_stateCodeLcd = 1;
				changeStateLcd = 1;
				key = 0xff;
				overString(stringLcdL, "Enter code:     ");
			}else if(key == 0x32){
				g_stateCodeLcd = 3;
				changeStateLcd = 1;
				key = 0xff;
			}
		}else if(g_stateCodeLcd == 3){
			overString(stringLcdU, "  Strict mode   ");
			overString(stringLcdL, "Confirm press # ");
			printLcdU();
			printLcdL();
			if(key == 0x23){
				g_stateCodeLcd = 4;
				changeStateLcd = 1;
				key = 0xff;
			}else if(key == 0x2a){
				g_stateCodeLcd = 2;
				changeStateLcd = 1;
				key = 0xff;
			}
		}else if(g_stateCodeLcd == 4){
			overString(stringLcdU, "  STRICT  MODE  ");
			overString(stringLcdL, "----------------");
			printLcdU();
			printLcdL();
			if(key == 0x2a){
				g_stateCodeLcd = 2;
				changeStateLcd = 1;
				key = 0xff;
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
	_delay_ms(500);
	initialExternalInteruprt();
	overString(stringLcdL, "InterruptSuccess");
	printLcdL();
	_delay_ms(500);
	initialTimer0();
	overString(stringLcdL, "InitTimerSuccess");
	printLcdL();
	buzzer(1);
	_delay_ms(50);
	buzzer(0);
	_delay_ms(200);
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