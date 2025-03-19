// Reheatable_Metal_Container.ino
//
// The Reheatable Metal Container is a container that reheats food served within the canteen which heats
// them using PTC heating element up to 165 Fahrenheit in the time frame [10, 15, 20]. 
//
// Version    YY//MM/DD   Comments
// =======    =========   ==============================================================================
// 1.00       25/02/24    Created the foundation of the coding base of the RMC
// 1.10       25/02/25    Changed the temp sensor to the LM35 instead of the DHT11
// 1.20       25/03/02    Converted the Celcius and translated it into Fahrenheit
// 1.31       25/03/03    Fixed some codes that causes the timer to be slightly slower
// 1.41       25/03/09    Added the Debouncing function within the buttons to remove multiple readings
// 1.43       25/03/10    Slight changes to the assigned pins of modules
// 1.44       25/03/12    Birthday fixing
// 1.54       25/03/15    Added code to shut of the relay when temperature reaches 165 Fahrenheit

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <ezButton.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHT_TYPE DHT11  // Change to DHT11 when using DHT11 comp.
#define DHT_PIN 8    // DHT Sensor Pin
#define RELAY_PIN 9  // Relay Module Pin

#define temp_max 165
#define temp_min 157

const int reed = 10;
int reedstate;

ezButton button1(7);
ezButton button2(6);
ezButton button3(5);
ezButton button4(4);

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT_TYPE);

int heatingTime = 0;
bool heating = false;
unsigned long startTime = 0;

void setup() {
  // Testing the accuracy of the LM35 sensor Celcius to Faren conversion
  Serial.begin(9600);  // Wow so many

  // Initiate relay module
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  pinMode(reed, INPUT_PULLUP); // Mode for reed

  // Initiate DHT and LCD
  dht.begin();
  lcd.begin(16, 2);
  lcd.backlight();

  // Debounce for the Buttons
  button1.setDebounceTime(50);
  button2.setDebounceTime(50);
  button3.setDebounceTime(50);
  button4.setDebounceTime(50);
}

void loop() {
// Reads the current temperature of the system
  int sensorValue = analogRead(A3);
  float voltage = sensorValue * (5.0 / 1023.0);
  int temperatureC = voltage * 100;
  int temperatureF = (temperatureC * 9 / 5) + 32;

button1.loop();
button2.loop();
button3.loop();
button4.loop();

  // Codes for the Buttons
 if (button1.isPressed()) {
    stopHeating();
  } 
  else if (button2.isPressed()) {
    heatingTime = 10;
    startHeating();
  } 
  else if (button3.isPressed()) {
    heatingTime = 15;
    startHeating();
  } 
  else if (button4.isPressed()) {
    heatingTime = 20;
    startHeating();
  }


// Temperature-based control of the Relay
// Uses a Hysteresis-based control code to prevent rapid relay switching 
// by creating a buffer zone between the temperatures
  if(heating) {
    if(temperatureF >= temp_max) {
      digitalWrite(RELAY_PIN, LOW);
      heating = false;
    } 
    else if(temperatureF <= temp_min) {
      digitalWrite(RELAY_PIN, HIGH);
      heating = true;
    }
  }

  if (heating) {
    unsigned long elapsedTime = millis() - startTime;
    unsigned long remainingTime = (heatingTime * 60000) - elapsedTime;

    printTime(remainingTime);
    printTH();

    if (elapsedTime >= (heatingTime * 60000)) {
      stopHeating();
    }
  }
// Calls forth thy debugging bs
 debugSensors();
   delay(200);

  reedstate = digitalRead(reed);

  if(digitalRead(reed) == HIGH) {
    reedon();
  } 
    else {
      reedoff();
  }
}

void printTH() {
  // Code for LM35 Temperature sensor
  int sensorvalue = analogRead(A3);
  // Converts the analog reading of the LM35 to a readable version in voltage
  float voltage = sensorvalue * (5.0 / 1023.0);
  // Converts the voltage to temperature
  // Also converts the float function to an integer function to remove decimal temperatures 00.xx C
  int temperatureC = voltage * 100;
  int temperatureF = temperatureC * 9 / 5 + 32;

  // Code for the dht11 Sensor
  int hum = dht.readHumidity();

  // Print format: T: <Temperature>F H: <Humidity>%
  lcd.setCursor(0, 0);
    lcd.print("T: ");
    lcd.print(temperatureF);
    lcd.print((char)223);
    lcd.print("F ");
    lcd.print("H: ");
    lcd.print(hum);
    lcd.print("%  ");
    delay(200);
}

// Code for time function in the RMC
void printTime(unsigned long ms) {
  int minutes = (ms / 60000);
  int seconds = (ms / 1000) % 60;

  // Print format: Time: <mm:ss>
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  if (minutes < 10) {
    lcd.print("0");
  }

  lcd.print(minutes);
  lcd.print(":");
  if (seconds < 10) {
    lcd.print("0");
  }

  lcd.print(seconds);
  lcd.print("  ");
}

// Uses the relay to turn ON the heating
void startHeating() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Heating...");

  digitalWrite(RELAY_PIN, HIGH);
  heating = true;
  startTime = millis();
}

// Uses the relay to turn OFF the heating
void stopHeating() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Heating Done!");

  digitalWrite(RELAY_PIN, LOW);
  heating = false;

  delay(2000);
  reedon();
}

void reedoff() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lid Open!");
  lcd.setCursor(0, 1);
  lcd.print("Please Close!");

  digitalWrite(RELAY_PIN, LOW);
  heating = false;
}

void reedon() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select Time:");
}

// Debugging Code

void debugSensors() {
  int sensorValue = analogRead(A3);
  float voltage = sensorValue * (5.0 / 1023.0);
  int temperatureC = voltage * 100;
  int temperatureF = temperatureC * 9 / 5 + 32;

  Serial.print("LM35 Temp: ");
  Serial.print(temperatureC);
  Serial.print("°C / ");
  Serial.print(temperatureF);
  Serial.print("°F");
}