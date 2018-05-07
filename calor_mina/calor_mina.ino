#include <OneWire.h>
#include <DallasTemperature.h>
#include <FastLED.h>

#define LED_PIN     8
// Information about the LED strip itself
#define NUM_LEDS    36 // 60 cm 60
#define CHIPSET     WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
uint8_t gHue = 0; // rotating "base color" used by many of the patterns


#define BRIGHTNESS  128

/********************************************************************/
// Rele is plugged into pin 7 on the Arduino
#define RELE 7
#define BOARD_LED 13  

/********************************************************************/
  
// Data wire is plugged into pin 10 on the Arduino 
#define ONE_WIRE_BUS 10 

// Temperature precision is 12 bits (max.)
#define TEMPERATURE_PRECISION 12

// Temperature is measured every  milliseconds
#define TEMP_HEAT 128000
#define TEMP_COOL 128000

#define PRINT_PERIOD 500

/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress; // Stores a found device address

unsigned long devAdd; 

int numberOfDevices; // Number of temperature devices found

  
bool RELE_state = 1; // Rele ON
unsigned long currentTime; 
unsigned long changeTime, printTime; // Last time relay was updated


void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void setup (void) {
    // Start serial port500
    Serial.begin(115200);
    // Define rele pin as OUTPUT and set it turned off by default
    pinMode(RELE, OUTPUT);
    pinMode(BOARD_LED, OUTPUT);      
    digitalWrite(RELE, RELE_state);
    // Start up the library
    sensors.begin();
    // Grab a count of devices on the wire
    numberOfDevices = sensors.getDeviceCount();
    Serial.print("No devices ");
    Serial.println(numberOfDevices);
    // Loop through each device
    for (int i = 0; i < numberOfDevices; i++) {
        // Search the wire for address
        if (sensors.getAddress(tempDeviceAddress, i)) {
            Serial.print("S: ");
            Serial.print(i);
            Serial.print(" A: ");
            //devAdd = tempDeviceAddress;
            //Serial.print(tempDeviceAddress, HEX);500
            Serial.println("");
            //Serial.println(sensors.getTempCByIndex(0));
            // Set the resolution
            sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
        }
    }
    currentTime = millis();
    changeTime = currentTime + TEMP_HEAT/2;    // Cool Half Cycle
    printTime  = currentTime + PRINT_PERIOD + 55;
    delay( 3000 ); // power-up safety delay
    // It's important to set the color correction for your LED strip here,
    // so that colors can be more accurately rendered through the 'temperature' profiles
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
    FastLED.setBrightness( BRIGHTNESS );

}

void loop() {
    static uint8_t starthue = 0;
    currentTime = millis();
    if (currentTime  > changeTime) {
        if (RELE_state)
            changeTime += TEMP_HEAT;
        else
            changeTime += TEMP_COOL;
        digitalWrite(RELE, !RELE_state); // Rele is off when pin is HIGH
        digitalWrite(BOARD_LED, RELE_state);
        RELE_state = !RELE_state; // CHANGE RELAY STATE
    }
    //Serial.print(" Requesting temperatures..."); 
    if (currentTime  > printTime) {
        printTime  += PRINT_PERIOD;
        sensors.requestTemperatures(); // Send the command to get temperature readings 
        /********************************************************************/
        Serial.print(currentTime);
        Serial.print(", ");
        //  Serial.print(" H: ");
        Serial.print(!RELE_state);
        Serial.print(", ");
        //  Serial.print(" T: ");
        Serial.print(sensors.getTempCByIndex(1)); // Why "byIndex"?  
        Serial.print(", ");
        //  Serial.print(" T2: ");
        Serial.print(sensors.getTempCByIndex(0));
        Serial.print(", ");
        //  Serial.print(" T3: ");
        Serial.println(sensors.getTempCByIndex(2));
        // You can have more than one DS18B20 on the same bus.  
        // 0 refers to the first IC on the wire   
          // draw a generic, no-name rainbow
        //fill_rainbow( leds + 5, NUM_LEDS - 5, --starthue, 20);
        sinelon();
        FastLED.show();
        // insert a delay to keep the framerate modest
        FastLED.delay(8);//1000/FRAMES_PER_SECOND); 
        //gHue++; 
        }
        // do some periodic updates
        EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    //delay(50);
}


