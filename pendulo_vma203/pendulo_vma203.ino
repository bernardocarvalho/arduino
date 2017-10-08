//Sample using LiquidCrystal library
#include <LiquidCrystal.h>

const byte ledPin = 13;
const byte interruptPin = 2; //DIGITAL PIN 2 , INT0 
volatile byte state = LOW;
unsigned long timeUp, lastTimeUp, timeDown, lastTimeDown;
long  period, veloc;
int print_time     = 0, irq_cnt=0;
/*******************************************************

This program will test the LCD panel and the buttons

********************************************************/

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// read the buttons
int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor 
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
 if (adc_key_in < 50)   return btnRIGHT;  //0
 if (adc_key_in < 150)  return btnUP; // 100
 if (adc_key_in < 310)  return btnDOWN; //257
 if (adc_key_in < 460)  return btnLEFT; //411:
 if (adc_key_in < 850)  return btnSELECT;  

 // For V1.0 comment the other threshold and use the one below:
/*
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnSELECT;   
*/


 return btnNONE;  // when all others fail, return this...
}

void changePin() {
  int pinVal = digitalRead(interruptPin);
  digitalWrite(ledPin, pinVal);
  irq_cnt++;
  if (pinVal== LOW){  // Barrier closed
    if (irq_cnt > 3){
      lastTimeDown = timeDown; 
      timeDown=micros();
      period= timeDown -lastTimeDown;
      irq_cnt=0;
    }
  }
  else{
     lastTimeUp = timeUp; 
     timeUp=micros();
     veloc= timeUp - timeDown;
     if (irq_cnt ==1)  // Following last period count
       print_time++;
  }
  state = !state;
}

void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(0, changePin, CHANGE); // two external interrupts: numbers 0 (on digital pin 2) and 1 (on digital pin 3)
  Serial.begin(115200);
  Serial.println(F("Hello Physics."));
  lcd.begin(16, 2);              // start the library
  lcd.setCursor(0,0);
  lcd.print(F("Pendulo Simples")); // print a simple message
}
 
void loop()
{
 lcd.setCursor(4,1);            // move cursor to second line "1" and 9 spaces over
 lcd.print(millis()/1000);      // display seconds elapsed since power-up

   //lcd.setCursor(9,1);            // move cursor to 
   //lcd.print("P      ");   lcd.setCursor(9,1);            // move cursor to 
   //lcd.print("P      ");


 if(print_time != 0){
   print_time=0;
 /*  Serial.println(adc_key_in);
   Serial.print("TimeDown: ");
   Serial.print(timeDown, DEC);
   Serial.print("\t");
   Serial.print("TimeUp: ");
   Serial.println(timeUp, DEC);
   Serial.print("LTimeDown: ");
   Serial.print(lastTimeDown, DEC);
   Serial.print(" LTimeUp: ");
//   Serial.println(lastTimeUp);
//   Serial.print("LTimeUp: ");
   Serial.println(lastTimeUp, DEC);
*/
   Serial.print("Time: ");
   Serial.print(millis(), DEC);
   Serial.print("\tPeriod: ");
   Serial.print(period, DEC);
   Serial.print("\tVeloc:  ");
   Serial.println(veloc, DEC);
   lcd.setCursor(9,1);            // move cursor to 
   lcd.print("P      ");
   lcd.setCursor(11,1);            // move cursor
   lcd.print(period/1000);      // display period in millis

 }
 
 lcd.setCursor(0,1);            // move to the begining of the second line
 lcd_key = read_LCD_buttons();  // read the buttons

 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
   case btnRIGHT:
     {
     //lcd.print(adc_key_in);
     lcd.print("RIGHT ");
     break;
     }
   case btnLEFT:
     {
       irq_cnt=0;
       lcd.print(adc_key_in);
       lcd.print(" v");
     break;
     }
   case btnUP:
     {
     lcd.print(adc_key_in);
     lcd.print("UP    ");
     break;
     }
   case btnDOWN:
     {
     lcd.print(adc_key_in);
     lcd.print("DOWN  ");
     break;
     }
   case btnSELECT:
     {
      irq_cnt=0;  
//     lcd.print(adc_key_in);
     lcd.print(F("RESET"));
     break;
     }
     case btnNONE:
     {
     lcd.print("TIM ");
     break;
     }
 }
   delay(100);
}

