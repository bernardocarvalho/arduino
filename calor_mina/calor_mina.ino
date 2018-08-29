#include <OneWire.h>
#include <DallasTemperature.h>
#include <FastLED.h>

#define LED_PIN     8
// Information about the LED strip itself
#define NUM_LEDS    37 // 60 cm 60
#define CHIPSET     WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

DEFINE_GRADIENT_PALETTE( heatmap_gp ) {
  0,     0,  0,  0,   //black
128,   255,  0,  0,   //red
224,   255,255,  0,   //bright yellow
255,   255,255,255 }; //full white

CRGBPalette16 gPal;
// See Fire2012WithPalette
bool gReverseDirection = false;
uint8_t heatindex;

#define BRIGHTNESS  128
#define FRAMES_PER_SECOND 60
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

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

#define FIRE_PERIOD 600
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
unsigned long changeTime, printTime, fireTime; // Last time relay was updated


void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void Fire2012WithPalette()
{
// Array of temperature reading#define TEMP_COOL 128000s at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
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
    fireTime   = currentTime + FIRE_PERIOD + 5;
    delay( 3000 ); // power-up safety delay
    // It's important to set the color correction for your LED strip here,
    // so that colors can be more accurately rendered through the 'temperature' profiles
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
    //gPal = HeatColors_p;//heatmap_gp;
     // These are other ways to set up the color palette for the 'fire'.
  // First, a gradient from black to red to yellow to white -- similar to HeatColors_p
  //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
  
  // Second, this palette is like the heat colors, but blue/aqua instead of red/yellow
     gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);
  
  // Third, here's a simpler, three-step gradient, from black to red to white
//   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);

    FastLED.setBrightness( BRIGHTNESS );
    for (int i = 0; i < NUM_LEDS; i++) {
      heatindex = (i+1) * 6 ; //(something from 0-255);
      leds[i] = ColorFromPalette( gPal, heatindex); // normal palette access
    }
    FastLED.show();


}

// https://github.com/FastLED/FastLED/wiki/Gradient-color-palettes
void loop() {
    static uint8_t starthue = 0;
    // Add entropy to random number generator; we use a lot of it.
    random16_add_entropy( random());

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
        sensors.requestTemperatures(); // Send the command https://github.com/FastLED/FastLED/wiki/Gradient-color-palettesto get temperature readings 
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
        Serial.print(", ");gPal = heatmap_gp;
        //  Serial.print(" T3: ");
        Serial.println(sensors.getTempCByIndex(2));
        // You can have more than one DS18B20 on the same bus.  
        // 0 refers to the first IC on the wire   Basic-usage
          // draw a generic, no-name rainbow
        //fill_rainbow( leds + 5, NUM_LEDS - 5, --starthue, 20);
        //sinelon();
        //FastLED.show();
        // insert a delay to keep the framerate modest
        //FastLED.delay(8);//1000/FRAMES_PER_SECOND); 
        //gHue++; 
        }
        if (currentTime  > fireTime) {
          fireTime  += FIRE_PERIOD;
          Fire2012WithPalette(); // run simulation frame, using palette colors
  
          FastLED.show(); // display this frame
        //FastLED.delay(1000 / FRAMES_PER_SECOND);
        }
        // do some periodic updates
        //EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    //delay(50);
}


