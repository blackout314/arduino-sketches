///----Libraries---///
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "TSL2561.h"

///---Defines---///
#define PIXEL_PIN 6  
#define PIXEL_COUNT 1

///---Initialization---///
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
TSL2561 tsl(TSL2561_ADDR_FLOAT); 

///---Setup---///
void setup() {
  Serial.begin(9600);
  strip.begin(); // Neopixel Setup
  strip.show(); // Initialize all pixels to 'off'
  if (tsl.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No sensor?");
    while (1);
  } 
  //tsl.enableAutoRange(true);
  tsl.setGain(TSL2561_GAIN_0X); //Avoids sensor saturation
  tsl.setTiming(TSL2561_INTEGRATIONTIME_101MS);
}

void loop(){
  for (time<
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  //delay(2000);
  //colorWipe(strip.Color(0, 0, 255), 50);//Blue
  //delay(2000);
  //colorWipe(strip.Color(255, 255, 0), 50); //Yellow
  //delay(2000);
  //colorWipe(strip.Color(0, 0, 0), 50);
  uint16_t x = tsl.getLuminosity(TSL2561_VISIBLE);  
  String stringOne = "The lumosity is: " ;
  String stringThree = stringOne + x;
  Serial.println(stringThree); 
  if (x<300){
    colorWipe(strip.Color(255, 255, 0), 50);
    delay(1000);
  }
  delay(1000);

}


void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
   }
 }
