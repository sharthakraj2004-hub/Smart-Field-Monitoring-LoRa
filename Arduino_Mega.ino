#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <LoRa.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ---------------- PIN DEFINITIONS ----------------
#define SOIL_PIN      A0      // YL-69 analog output
#define ONE_WIRE_BUS  3       // DS18B20 data pin

#define GREEN_LED     7       // Motor ON indicator
#define RED_LED       8       // Motor OFF indicator

// Ultrasonic
#define TRIG_PIN      4
#define ECHO_PIN      5

// LoRa pins for Mega
#define LORA_SS    10     // Chip select
#define LORA_RST   9      // Reset
#define LORA_DIO0  2      // IRQ pin

// ---------------- OBJECTS ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2); // default, will detect in setup
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ---------------- VARIABLES ----------------
int soilRaw = 0;
int soilPercent = 0;
byte msgCount = 0;
byte MasterNode = 0xFF;
byte Node1 = 0xBB;
bool lcdFound = false;

long duration;
int waterLevel = 0;
int waterPercent = 0;
bool motorStatus = false;  // Track motor state

// Thresholds
const int SOIL_THRESHOLD = 30;   // % soil moisture
const float TEMP_THRESHOLD = 35; // °C

// ---------------- I2C SCANNER FUNCTION ----------------
byte scanI2C() {
  byte address;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    if(Wire.endTransmission() == 0) {
      Serial.print("I2C device found at 0x");
      Serial.println(address, HEX);
      return address;
    }
  }
  Serial.println("No I2C devices found!");
  return 0;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // ---------------- LCD ----------------
  byte lcdAddr = scanI2C();
  if(lcdAddr != 0) {
    lcdFound = true;
    lcd = LiquidCrystal_I2C(lcdAddr, 16, 2);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("LCD Detected!");
  } else {
    lcdFound = false;
    Serial.println("LCD not found!");
  }

  // ---------------- DS18B20 ----------------
  sensors.begin();

  // ---------------- LoRa ----------------
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  Serial.println("Initializing LoRa...");
  if (!LoRa.begin(433E6)) {  // 433 MHz
    Serial.println("LoRa init failed!");
    if(lcdFound){
      lcd.setCursor(0,1);
      lcd.print("LoRa Failed!");
    }
    while(1);
  }
  Serial.println("LoRa init successful!");
  if(lcdFound){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("System Ready!");
    delay(2000);
  }

  // ---------------- LEDs ----------------
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);

  // ---------------- Ultrasonic ----------------
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // ---------------- Soil Moisture ----------------
  soilRaw = analogRead(SOIL_PIN);
  soilPercent = map(soilRaw, 1023, 0, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  // ---------------- Temperature ----------------
  sensors.requestTemperatures(); 
  float tempC = sensors.getTempCByIndex(0); 

  // ---------------- Ultrasonic Water Level ----------------
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  waterLevel = duration * 0.034 / 2; // in cm
  waterLevel = constrain(waterLevel, 0, 30); // max 30 cm

  // Convert water level to percentage
  waterPercent = map(waterLevel, 0, 30, 0, 100);
  waterPercent = constrain(waterPercent, 0, 100);

  // ---------------- Motor LED Control ----------------
  if(soilPercent < SOIL_THRESHOLD && tempC < TEMP_THRESHOLD){
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    motorStatus = true;   // Motor ON
  } else {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
    motorStatus = false;  // Motor OFF
  }

  // ---------------- Display on LCD ----------------
  if(lcdFound){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("S:");
    lcd.print(soilPercent);
    lcd.print("% T:");
    if(tempC != DEVICE_DISCONNECTED_C){
      lcd.print(tempC,1);
      lcd.print((char)223); lcd.print("C");
    } else lcd.print("Err");

    lcd.setCursor(0,1);
    lcd.print("W:");
    lcd.print(waterPercent);
    lcd.print("%");
    lcd.print(" M:");
    if(motorStatus) lcd.print("ON");
    else lcd.print("OFF");
  }

  // ---------------- Send LoRa Packet ----------------
  String outgoing = String(soilRaw) + "," 
                  + String(soilPercent) + "," 
                  + String(tempC) + "," 
                  + String(waterPercent) + ","   // percentage
                  + String(motorStatus);   // add motor status

  LoRa.beginPacket();
  LoRa.write(MasterNode);  // destination
  LoRa.write(Node1);       // sender
  LoRa.write(msgCount++);
  LoRa.write(outgoing.length());
  LoRa.print(outgoing);
  LoRa.endPacket();

  // ---------------- Serial Output ----------------
  Serial.print("Soil: "); Serial.print(soilPercent);
  Serial.print("%, Temp: "); Serial.print(tempC);
  Serial.print("C, Water: "); Serial.print(waterPercent);
  Serial.print("%, Motor: ");
  if(motorStatus) Serial.println("ON");
  else Serial.println("OFF");

  delay(2000);
}
