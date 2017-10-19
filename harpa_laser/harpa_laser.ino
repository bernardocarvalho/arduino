/* Ultrasonic Sensor HC-SR04 and Arduino Tutorial
  *
  * Crated by Dejan Nedelkovski,
  * www.HowToMechatronics.com
  *
  */
#include <LiquidCrystal.h>

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// defines pins numbers
const int trigPin = 2;
const int echoPin = 3;
const int analogPin = 1;     // Photodiode Emiter
// defines variables

int adcval = 0;           // variable to store the value read
int distance;

char get_note(int dist){
  char c = 'A';
  if (dist > 40)
    c = 'C';
  else if (dist > 30)
    c = 'D';
  else if (dist > 20)
    c = 'E';
  else if (dist > 10)
    c = 'F';
  else
      c = 'G';
  return c;
}

// Returns true if Laser ADC <  LEVEL, or false if 
// timeout expired.  
bool wait_for_laser_off_or_timeout(int LEVEL, unsigned long ms)
{
  unsigned long start = millis();
  unsigned long cur;
  do
  {
      cur = millis();
  }
  while ((analogRead(analogPin) > LEVEL) && (unsigned)(cur - start) < ms);

  bool ret = (unsigned)(cur - start) < ms;
  return ret;
}

// Returns time when Laser ADC > LEVEL, or  if 
// time expired. 
unsigned wait_for_laser_time_or_timeout(int LEVEL, unsigned long ms)
{
  unsigned long start = millis();
  unsigned long cur;
  do
  {
      cur = millis();
  }
  while ((analogRead(analogPin) < LEVEL) && (unsigned)(cur - start) < ms);

  unsigned time = (unsigned)(cur - start);
  
  //bool ret = (unsigned)(cur - start) < ms;
  return time;
}

// Returns sound distance.
int sound_distance(){ 
      // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    long duration = pulseIn(echoPin, HIGH);
    // Calculating the distance
    int dist = duration*0.034/2;
    return dist;
}    
void setup() {
     lcd.begin(16, 2);              // start the library
     lcd.setCursor(0,0);
     lcd.print("Velleman VMA203"); // print a simple message

    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(echoPin, INPUT); // Sets the echoPin as an Input
    Serial.begin(115200); // Starts the serial communication
  }
 void loop() {
    unsigned time_note = 0;
    if(wait_for_laser_off_or_timeout(256, 200)){
      time_note = wait_for_laser_time_or_timeout(700, 400);    
    }
    distance= sound_distance();
    // Prints the distance on the Serial Monitor
    Serial.print("Dist ");
    Serial.print(distance);
    Serial.print(" Note ");
    char nt = get_note(distance);
    Serial.write(nt);
    Serial.print(" Time ");
    Serial.println(time_note);
    
    adcval = analogRead(analogPin);
    lcd.setCursor(0,0);            // move to the begining of the second line
    lcd.print("LAS ");
    lcd.print(adcval);
    lcd.print("     ");
    lcd.setCursor(0,1);            // move to the begining of the second line
    lcd.print("DIST ");
    lcd.print(distance);
    lcd.print("   ");
    lcd.setCursor(10,1);  
    lcd.write(nt);
    //delay(400);
}
