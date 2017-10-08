#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 10 on the Arduino
#define ONE_WIRE_BUS 10

// Temperature precision is 12 bits (max.)
#define TEMPERATURE_PRECISION 12

// Rele is plugged into port 8 on the Arduino
#define RELE 8

// Temperature is measured every 1000 milliseconds
#define TEMP_INTERVAL 1000

// Period is 30 seconds by default
#define DEFAULT_PERIOD 1800000

// Rele is on for the period by default
#define DEFAULT_ON_TIME 0.25*DEFAULT_PERIOD

// Setup a OneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass the oneWire reference to Dallas Temperature
DallasTemperature sensors(&oneWire);

DeviceAddress tempDeviceAddress; // Stores a found device address

int numberOfDevices; // Number of temperature devices found
String input = ""; // String to store the requests from the serial
int numero; // Index number of the intended temperature device
// Temperature values
float temperatura0;
float temperatura1;
float temperatura2;

bool RELE_state = 0; // Rele state
unsigned long prevTime; // Last time the temperature was updated
unsigned long prevTime2; // Last time the rele was updated
unsigned long prevTime3; // Last time an instruction was received from Pi
unsigned long onTime = DEFAULT_ON_TIME;
unsigned long onTime_next = DEFAULT_ON_TIME;
unsigned long period = DEFAULT_PERIOD;
unsigned long period_next = DEFAULT_PERIOD;
bool overwrite;

void setup (void) {
  // Start serial port
  Serial.begin(115200);

  // Start up the library
  sensors.begin();

  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();

  // Loop through each device
  for (int i = 0; i < numberOfDevices; i++) {
    // Search the wire for address
    if (sensors.getAddress(tempDeviceAddress, i)) {
      // Set the resolution
      sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
    }
  }

  // Define rele pin as OUTPUT and set it turned off by default
  pinMode(RELE, OUTPUT);
  digitalWrite(RELE, LOW);

  pinMode(LED_BUILTIN, OUTPUT);
}

void serial_protocol() {
  while (Serial.available() > 0) {
    char lastRecvd = Serial.read();
    
    // if end of message has arrived
    if (lastRecvd == '\n') {
      switch (input[0]) {
        // Request for temperature readout
        case 'T':
            Serial.print("T1=");
            Serial.print(temperatura2);
            Serial.print(", ");
            Serial.print("T2=");
            Serial.print(temperatura1);
            Serial.print(", ");
            Serial.print("T3=");
            Serial.print(temperatura0);
            Serial.print(", ");
            Serial.print("S=");
            Serial.print(String(RELE_state));
            Serial.print("\r\n");
            input="";
            break;

        // Order for new period
        case 'P':
          input = input.substring(2); // Format is P=___
          
          period_next = input.toInt() * 1000; // Convert to milliseconds
          
          input = "";
          break;
          
        // Order for new on time
        case 'D':
          input = input.substring(2); // Format is D=___

          // if new on time is between 0 and 1000
          if (input.toInt() >= 0 && input.toInt() <= 1000) {
            onTime_next = input.toInt();
          }
          
          input = "";
          break;
        
        // Overwrite
        case 'O':
          overwrite = 1;
          input = "";
          break;
          
        default:
          input = "";
          break;
      }
     } else {
      input += lastRecvd;
    }
  }
}

void loop (void) {
  unsigned long currentTime = millis();
  
  serial_protocol();

  // Call sensors.requestTemperatures() to issue a global temperature
  // Request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures

  // If current time minus previous time exceeds TEMP_INTERVAL
  if (currentTime - prevTime > TEMP_INTERVAL) {
    prevTime = currentTime;

    // Update temperatures
    temperatura0 = sensors.getTempCByIndex(0);
    temperatura1 = sensors.getTempCByIndex(1);
    temperatura2 = sensors.getTempCByIndex(2);
  }

  // Control of rele
  // onTime defines for how much time of one hour it is on
    if (currentTime - prevTime2 > onTime) {
  
      if (RELE_state == 1 && onTime != period) {
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(RELE, LOW); // Rele is off when pin is HIGH
       
        RELE_state = 0;
      }
  
      // If period has passed, update onTime for the next hour
      if (currentTime - prevTime2 > period || overwrite == 1) {
        prevTime2 = currentTime;
  
        if (onTime_next != onTime) {
          onTime = (onTime_next*period)/1000;
        }
  
        // Change time to adapt to next period
        if (period_next != period) {
          onTime = (onTime*1000)/period; // Convert to permillage
          period = period_next;
          onTime = (onTime*period)/1000; // Convert back to milliseconds mantaining the proportions
          onTime_next = onTime;
        }
  
        overwrite = 0;
      }
      
    } else {
      if (overwrite == 1) {
        prevTime2 = currentTime;
  
        if (onTime_next != onTime) {
          onTime = (onTime_next*period)/1000;
        }
  
        // Change time to adapt to next period
        if (period_next != period) {
          onTime = (onTime*1000)/period; // Convert to permillage
          period = period_next;
          onTime = (onTime*period)/1000; // Convert back to milliseconds mantaining the proportions
          onTime_next = onTime;
        }
        
        overwrite = 0; 
      }
      
      if (RELE_state == 0) {
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(RELE, HIGH); // Rele is on when pin is LOW
        RELE_state = 1;
      }
    }
}

