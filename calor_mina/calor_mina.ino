#include <OneWire.h>
#include <DallasTemperature.h>

//#include <Wire.h>

/********************************************************************/
// First we include the libraries
//#include <OneWire.h> 
//#include <DallasTemperature.h>
// Rele is plugged into port 8 on the Arduino
#define RELE 7
#define LED 13
//int led = 13;


/********************************************************************/
// Data wire is plugged into pin 2 on the Arduino 
#define ONE_WIRE_BUS 10 

// Temperature precision is 12 bits (max.)
#define TEMPERATURE_PRECISION 12

// Temperature is measured every  milliseconds
#define TEMP_HEAT 120000 // 
#define TEMP_COOL 120000 // 

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
unsigned long changeTime; // Last time relay was updated

// function to print a device address

void setup (void) {
  // Start serial port
  Serial.begin(115200);
    // Define rele pin as OUTPUT and set it turned off by default
  pinMode(RELE, OUTPUT);
  pinMode(LED, OUTPUT);      
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
      //Serial.print(tempDeviceAddress, HEX);
      Serial.println("");
      //Serial.println(sensors.getTempCByIndex(0));
      // Set the resolution
      sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
    }
  }
  currentTime = millis();
  changeTime = currentTime + TEMP_HEAT/2;   // Coool Half Cycle
}

void loop() {
  currentTime = millis();
  if (currentTime  > changeTime) {
    if (RELE_state)
      changeTime = currentTime + TEMP_HEAT;
    else  
      changeTime = currentTime + TEMP_COOL;
    digitalWrite(RELE, !RELE_state); // Rele is off when pin is HIGH
    digitalWrite(LED, RELE_state); 
    RELE_state = !RELE_state; // CHANGE RELAY STATE
  }
  //Serial.print(" Requesting temperatures..."); 
  sensors.requestTemperatures(); // Send the command to get temperature readings 
  //Serial.println("DONE"); 
  /********************************************************************/
  Serial.print(currentTime); 
  Serial.print(" H: "); 
  Serial.print(!RELE_state); 
  Serial.print(" T: "); 
  Serial.print(sensors.getTempCByIndex(0)); // Why "byIndex"?  
  Serial.print(" T2: "); 
  Serial.print(sensors.getTempCByIndex(1)); 
  Serial.print(" T3: "); 
  Serial.println(sensors.getTempCByIndex(2)); 
   // You can have more than one DS18B20 on the same bus.  
   // 0 refers to the first IC on the wire 
       
//    Serial.println(currentTime);
  delay(500);
}


