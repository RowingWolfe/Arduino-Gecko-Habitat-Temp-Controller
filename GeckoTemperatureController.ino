/*
 * The Yarrbeard Habitat Temperature Controller 
 * Version 0.1
 * Currently in testing. Do not use this as your primary controller for your own reptile habitats
 * without a backup in place. 
 * This works using a voltage divider circuit with a 10k ohm thermistor and a 10k ohm resistor.
 * It finds the difference in resistance between the resistor and thermistor and through some math outputs
 * a temperature. 
 * Using this temperature (T) we control a 4-pin OLED display and a relay to power on/off a habitat heating system.
 * 
 * The next iteration is enabling this to call an endpoint on Mantilogs (another project of mine) and log
 * the data from the thermistor to a DB.
 * 
 * This code is provided as-is with no warranties, garauntees or promises as to it's functionality.
 * I will say again, do not use this as your primary habitat temperature controller as it is barely tested yet.
 * If you do use this and find bugs or other issues... 
 * please let me know @ yarr@yarrdev.com or through the github repository you
 * got this from.
 */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SWITCH_PIN 13 // The pin that controls the relay.

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Thermistor Setup.
int ThermistorPin = 0;
int Vo;
float R1 = 10000;
float logR2, R2, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

//Track heating/cooling
bool heating = false;

void setup() {
  Serial.begin(9600);
  //Make sure the display is hooked up right.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  pinMode(SWITCH_PIN, OUTPUT);    // sets the digital pin 13 as output
}

void loop() {
  //Math to convert values from the voltage divider into usable temp data.
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T = T - 273.15;
  T = (T * 9.0)/ 5.0 + 32.0; 

  //Serial Debuggery
  Serial.print("Temperature: "); 
  Serial.print(T);
  Serial.println(" F"); 


  //Clear and then update the OLED Display.
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display Text.
  display.println("Temperature:");
  display.setTextSize(3);
  display.println(T);

  // Set the heating bool based on temperatures(T)
  // In this case we are working with Leopard Geckos so we want to maintain
  // a temperature range between 85 and 90F.
  if(T <= 85.0){
    heating = true;
  }else if (T >= 90.0){
    heating = false;
  }

  //Display heating/cooling info and turn On/Off the switch controlling the heating system.
  if(heating){
    display.setTextSize(2);
    display.println("Heating");
    digitalWrite(SWITCH_PIN, HIGH);
  }else{
    display.setTextSize(2);
    display.println("Cooling");
    digitalWrite(SWITCH_PIN, LOW);
  }

  // Show The display and wait.
  display.display(); 
  delay(3000); //The heating system I use is slow enough to heat up that I only update after 3 sec. YMMV.
}
