/* Reaction Time Tester
 *  
 * by mikalhart - mhcom <at> sundial <dot> com
 * http://www.arduino.cc/playground/ComponentLib/ReactionTimeTester
 * 
 * Adapted by Bernardo Carvalho to LCD display LCD & Keypad Shield for Arduino Boards
 * https://www.vellemanstore.com/en/velleman-vma203-lcd-keypad-shield-for-arduino
 *
 * A game that measures a person's reaction time
 *
 * You press the big red button to start play. 
 * When the green light comes on, you release the 
 * button as quickly as possible, and the RTT displays 
 * the number of milliseconds that it took you to react. 
 * Repeat 10 times, and your "score" for the "game" is the 
 * sum of the 10 individual response times. 
 *
 * Your goal, of course, is to try to minimize your 
 * cumulative score. At the end of the game, the top 10 
 * all-time scores (stored in EEPROM) are displayed on the 
 * LCD display panel. If you are lucky enough to get 
 * the low score, LEDs flash admiringly. 
 *
 */

#include <LiquidCrystal.h>
#include <EEPROM.h>

const byte ledPin = 13;
//const byte interruptPin = 2; //DIGITAL PIN 2 , INT0 
volatile byte state = LOW;
//unsigned long timeUp, lastTimeUp, timeDown, lastTimeDown;
//long  period, veloc;
//int print_time     = 0, irq_cnt=0;

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

// Change to "true" to enable serial debugging
#define DEBUG true

// This trick makes all the embedded serial stuff go away magically
#if !DEBUG
#define Serial NullSerial
class NullSerialClass
{
  public:
  void begin(int baud) {}
  void println(int x, int type=DEC) {}
  void println(const char *p=NULL) {}
  void print(int x, int type=DEC) {}
  void print(const char *p) {}
  int available() {return 0;}
  int read() {return -1;}
  void flush() {}
} NullSerial;
#endif

// Global variables
bool bCheat = false, bRecord = false;

// read the shield keypad buttons
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

 return btnNONE;  // when all others fail, return this...
}

// Returns true if pin attained LEVEL, or false if 
// time expired.  During the wait, the cheat, ready and go
// LEDs are flash appropriately
bool wait_for_pin_or_timer(int LEVEL, unsigned long ms)
{
  unsigned long start = millis();
  unsigned long cur;
  do
  {
    if (bCheat) {
      //flash_led(CHEAT_LED, 400, 200);
        //lcd.setCursor(0,0); // move to the begining of the second line
        //lcd.blink();
        //lcd.print("CHEAT!  "); 
 
      }
    if (bRecord) 
    {
      //flash_led(READY_LED, 400, 200);
      //flash_led(GO_LED, 400, 200);
        lcd.setCursor(0,0); // move to the begining of the first line
        lcd.blink();
        lcd.print("RECORD! "); 
    }
    cur = millis();
  }
  while ((read_LCD_buttons() != LEVEL) && (unsigned)(cur - start) < ms);

  bool ret = (unsigned)(cur - start) < ms;
 /* if (ret)
  {
    Serial.print("WFPT: "); Serial.print(ret);   Serial.print(" "); Serial.print(LEVEL);  Serial.print(" "); Serial.println(ms);
    Serial.print("Button pressed!  cur: ");
    Serial.print(cur);
    Serial.print(", start: ");
    Serial.println(start);
  }
  */
  return ret;
}


// Flash a "hello!"
void flash_all_leds(int interval, int count)
{
  for (int i=0; i<count; ++i)
  {
    digitalWrite(ledPin, HIGH);   
    delay(interval);
    digitalWrite(ledPin, LOW);
    delay(interval);
  }
}

// Set the LED state depending on period and duty
void flash_led(int pin, int period, int duty)
{
  int cycle = millis() % period;
  digitalWrite(pin, cycle <= duty ? HIGH : LOW);
}

void flash_lcd(int interval, int count){
  for (int i=0; i<count; ++i){
    delay(interval);
     // Turn off the display:
    lcd.noDisplay();
    delay(interval);
    lcd.display();
  }
}

void setup()
{
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  Serial.println(F("Hello MEFT-LIFE."));
  lcd.begin(16, 2);              // start the library
  lcd.clear();
  lcd.print(F("Tempo Reacao")); // print a simple message
  flash_lcd(200,5);
  
  lcd_key = read_LCD_buttons(); // read the buttons
    // If user has his finger inserted at startup, then this is a special cue to erase the EEPROM
  if (lcd_key == btnUP)
  {
    for (int i=0; i<512; ++i)
      EEPROM.write(i, 0xFF);
    Serial.println("EEPROM erased!");
    lcd.setCursor(0,1); // move to the begining of the second line
    lcd.blink();
    lcd.print("EEPROM erased!"); 
   // lcd.blink();
    flash_lcd(100,5);
    //flash_all_leds(100, 4);
    //disp.scroll_text("ERASE   ", 1000);
    delay(1000);  
    // require that he lift his finger before beginning (to get a good seed)
    while (read_LCD_buttons() == btnUP);
  }

}
 
void loop()
{

  unsigned long start, delta;
  unsigned short top_nine_scores[9];
  unsigned long total_score;
  
  // Load the top nine high (low!) scores for display
  for (int i=0; i<9; ++i)
    top_nine_scores[i] = (byte)EEPROM.read(2*i) + 256 * (byte)EEPROM.read(2*i + 1);
    
  // During the "inactive" state scroll through high scores until button is pushed
  lcd.setCursor(0,0); 
  lcd.print("RANKING         "); 
  lcd.setCursor(0,1); 
  lcd.print("             "); 
  bool lets_play = false;
  while (!lets_play)
  {
    for (int i=0; i<9; ++i)
    {
      if (top_nine_scores[i] == 0 || top_nine_scores[i] == 0xFFFF)
        break;
      lcd.setCursor(0,1); // move to the begining of the second line
      lcd.blink();
      lcd.print(i+1); 
  
      //disp.display_number(i + 1, 0);
      if (lets_play = wait_for_pin_or_timer(btnSELECT, 1000))
        break;
      lcd.setCursor(3,1); 
      lcd.print(top_nine_scores[i]); 

      //disp.display_number(top_nine_scores[i], 0); 
      if (lets_play = wait_for_pin_or_timer(btnSELECT, 3000))
        break;

      //lcd.setCursor(0,0);
      //lcd.scrollDisplayLeft();
    }
    if (!lets_play)
      lets_play = wait_for_pin_or_timer(btnSELECT, 1000);
  }
  lcd.clear();
   
  // Begin game with "PLAY" message on display
  //disp.scroll_text("PLAY", 2000);
  lcd.print("Solta para jogar"); 

  bCheat = bRecord = false;
  
  // Wait for user to lift "start" button
  while (read_LCD_buttons() == btnSELECT);
  
  lcd.setCursor(0,0); 
  lcd.print("                "); 
    
  // Randomize
  randomSeed(millis());
  
  total_score = 0;
  // Repeat the test 10 times
  for (int count = 0; count < 10; ++count)
  {
    // Turn off go LED
    //digitalWrite(GO_LED, LOW);
    lcd.setCursor(0,0); 
    lcd.print("PRIMA "); 
    if (count < 9)
      lcd.setCursor(15,0);
    else 
      lcd.setCursor(14,0);
    lcd.print(count + 1);
    // Wait for button press and debounce
    while (read_LCD_buttons() != btnSELECT)
      delay(10);
 
    lcd.setCursor(0,0); 
    lcd.print("ESPERA"); 
      // debounce button for 200ms
    //for (start = millis(); (unsigned long)(millis() - start) < 200;)
//     // flash_led(READY_LED, 100, 90);
    delay(200);
    
    // Start a counter for between 2 and 5 seconds
    delta = random(2000, 5000);

    // Wait for the timer to expire
    for (start = millis(); (unsigned long)(millis() - start) < delta; )
    {
      // Flash the "record" lamp
      //flash_led(READY_LED, 100, 90);

       // Did he release the button before the timer expired?
      if (read_LCD_buttons() != btnSELECT)
      {
        Serial.println("Cheat!");
        lcd.setCursor(0,0); 
        lcd.print("BATOTA"); 
        flash_lcd(200, 5);
        bCheat = true;
        return;
      }
    }
    // At the end of timer, light the "go" light
    lcd.setCursor(0,0); 
    lcd.print("SOLTA!"); 
 
    // Wait for player to remove finger: watch for "light touch" inverse debounce
    int score;
    bool bButtonLifted = false;
    while (!bButtonLifted)
    {
      //wait_for_pin(BUTTON_PIN, HIGH);     
      while (read_LCD_buttons() == btnSELECT);
      score = (unsigned long)(millis() - start - delta);
      bButtonLifted = true;
      
      // Make sure button is up at least 200ms
      for (long i=millis(); bButtonLifted && (unsigned long)(millis() - i) < 200; )
        if (read_LCD_buttons() == btnSELECT)      
          bButtonLifted = false;
    }
    
    
    // Calculate score
    Serial.print("Score is "); Serial.println(score);
    total_score += score;
    lcd.setCursor(0,1); // 
    lcd.print("ATRASO "); 
    //lcd.setCursor(7,1); // 
    lcd.print(score > 9999 ? 9999 : score); 
    //disp.display_number(score > 9999 ? 9999 : score, 1000);
    delay(1000);
  }
  // display score
  if (total_score > 9999) total_score = 9999;
  Serial.print("Cumulative score is "); Serial.println(total_score);
  lcd.setCursor(0,1); // 
  lcd.print("TOTAL "); 
  lcd.print(total_score); 
  //disp.display_number(total_score, 2000);
  delay(2000);
  
  // Insert new score into record list
  int cur = total_score;
  for (int i=0; i<256; ++i)
  {
    unsigned short record = (byte)EEPROM.read(2*i) + 256 * (byte)EEPROM.read(2*i + 1);
    
    if (record == 0 || cur < record)
    {
      // If it's a new record, then display a special message
      if (i == 0) // first place!
      {
        Serial.print("A new world record: "); Serial.print(cur); Serial.println("!");
        //digitalWrite(READY_LED, HIGH);
        lcd.setCursor(0,1); // 
        lcd.print("RECORD "); 
        //disp.scroll_text("BEST SCORE    ", 0);
        bRecord = true;
      }

      // otherwise, display the rank of the new score
      // but only if it's in the top 9
      if (cur == total_score && i < 9)
      {
        if (i == 0){
          lcd.setCursor(0,1); 
          lcd.print("FIRST PLACE "); 
          //disp.scroll_text("FIRST PLACE    ", 250);
        }
        else if (i == 1){
          lcd.setCursor(0,1); 
          lcd.print("SECOND PLACE "); 
        }
          //disp.scroll_text("SECOND PLACE    ", 250);
        else if (i == 2) {
          lcd.setCursor(0,1); 
          lcd.print("THIRD PLACE "); 
        }
          //disp.scroll_text("THIRD PLACE    ", 250);
        else
        {
          char string[] = "cTH PLACE    ";
          string[0] = i + '1';
          lcd.setCursor(0,1); 
          lcd.print(string); 
//          disp.scroll_text(string, 250);
        }
      }
          
      EEPROM.write(2 * i, (byte)cur);
      EEPROM.write(2 * i + 1, (byte)(cur / 256));
      Serial.print(i + 1); Serial.print(". "); Serial.print(cur);
      Serial.println(cur == total_score ? " <---" : "");
      cur = record;
    }
    else
    {
      Serial.print(i + 1); Serial.print(". "); Serial.println(record);
    }
  
    if (record == 0 || record == 0xFFFF)
      break;
  }

  


}

