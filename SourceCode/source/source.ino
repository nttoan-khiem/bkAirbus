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
#define TIME_REMAIN_STATE_5  15
//end define time remain state of system
//Globle Variable 
unsigned char g_operationCode = 0;
unsigned char g_vPort = 0;
unsigned char g_timeMili = 0;
unsigned char g_timeSec = 0;
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
bool checkCode();
void operationSystem();
//End list function sorfware
//Interupt function lock or unlock door
ISR(INT0_vect){   //Unlock function / allow to access cockpit
  g_operationCode = 3;  //unlock mode
  g_timeSec = TIME_REMAIN_STATE_3;
  writeDataLCD('0');
}

ISR(INT1_vect){ //lock function / denied to access cockpit
  g_operationCode = 2;  //lock mode
  g_timeSec = TIME_REMAIN_STATE_2;
  writeDataLCD('1');
}
//End define function interupt
//Interupt function timer of system
ISR(TIMER0_OVF_vect){ //timer 0 use for timer of system with 10ms for interrupt
  TCNT0=0xB2;
  if(g_timeMili > 0 || g_timeSec > 0){
    if(g_timeMili == 0 && g_timeSec > 0){
      g_timeSec --;
      g_timeMili = 99;
    }else if(g_timeMili > 0){
      g_timeMili --;
    }
  }
}
//end define function timer of system
//define function initial hardware 
void initialTimer0();
void initialExternalInteruprt();
void initialPinConfig();  //config pin
void initializationSystem();
//End define function initial hardware
/**************************
* 0: normal operation
* 1: request access normal 
* 2: lock mode
* 3: unlock mode
* 4: emergency request access mode
* 5: door fail/ open emergency success
* 6: test mode
***************************/ 

int main(void){
  initializationSystem();
  writeCommandLCD(0x0c);
  writeDataLCD('y');
  while(1){
    scanKey();
    setLCDPort(g_poiterOutside);
    operationSystem();
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
  TCCR0B=(0<<WGM02) | (1<<CS02) | (0<<CS01) | (1<<CS00);    //source clock / 1024
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
void scanKey(){
  g_vPort |= 0x0f;
  setVPort(g_vPort & 0xfe);
  _delay_us(1);
  if((PINC&(1<<3))==0){

  }else if((PINC&(1<<4))==0){

  }else if((PINC&(1<<5))==0){

  }
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
  
  setVPort(g_vPort & 0xfd);
  _delay_us(1);
  if((PINC&(1<<3))==0){

  }else if((PINC&(1<<4))==0){

  }else if((PINC&(1<<5))==0){

  }
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
  
  setVPort(g_vPort & 0xfb);
  _delay_us(1);
  if((PINC&(1<<3))==0){

  }else if((PINC&(1<<4))==0){

  }else if((PINC&(1<<5))==0){

  }
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
  setVPort(g_vPort & 0xf7);
  _delay_us(1);
  if((PINC&(1<<3))==0){

  }else if((PINC&(1<<4))==0){

  }else if((PINC&(1<<5))==0){

  }
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
      if(g_poiterOutside == 4){
        g_poiterOutside = 0;
        writeDataLCD('4');
        if(checkCode()){
          g_operationCode = 4;
          g_timeSec = TIME_REMAIN_STATE_4;
        }
      }else{
        g_poiterOutside = 0;
        g_operationCode = 1;
        g_timeSec = TIME_REMAIN_STATE_1;
      }
    }
    if(g_poiterOutside == 5){
      g_poiterOutside = 0;
    }
  }
}
void soundPushButton(){
  setBitPD(7,1);
  _delay_ms(100);
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
    case 0:   //normal state
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
void initializationSystem(){
  SREG |= 0x80;  //Global interrupt enable
  initialPinConfig();
  initialExternalInteruprt();
  initialTimer0();
}
