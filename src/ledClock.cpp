#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(20, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduinois  and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

uint32_t hours_digit0_color, hours_digit1_color;
uint32_t minutes_digit0_color, minutes_digit1_color;
uint32_t seconds_color;
void setTimeColors();

/*
 * Led strip order
 *   |_0__1__2__3__|x
 * 4 | 4  5  14 15
 * 3 | 3  6  13 16
 * 2 | 2  7  12 17
 * 1 | 1  8  11 18
 * 0 | 0  9  10 19
 * y
 */
const uint8_t width = 4;
const uint8_t height = 5;
uint32_t ledMatrixState[width][height];
void init_matrix();
void updateMatrix();
void setLed(uint8_t x, uint8_t y, uint32_t c);
void setAll(uint32_t);

uint32_t Wheel(byte WheelPos);


void setTimeColors(){
    hours_digit0_color = strip.Color(255, 0, 0);
    hours_digit1_color = strip.Color(0, 255, 0);

    minutes_digit0_color = strip.Color(0, 0, 255);
    minutes_digit1_color = strip.Color(255, 0, 0);

    seconds_color = strip.Color(255, 255, 255);
}


int loop256(int i){
    while (i > 255){
        i-= 256;
    }
    while (i < 0){
        i+= 256;
    }
    return i;
}


unsigned long previousMillis = 0;

void test(){
  for (int i = 0; i < 256; i++){
    /*for (uint16_t j = 0; j < strip.numPixels(); j++){
      strip.setPixelColor(j, Wheel(loop256(i-j*2)));
      strip.show();
    }*/
    for (int x = 0; x < 4; x++){
      for (int y = 0; y < 5; y++){
        setLed(x, y, Wheel(loop256(i-x*20+y*20)));
      }
    }
    updateMatrix();
    unsigned long currentMillis = millis();
    while (millis() - currentMillis < 10) {

      updateMatrix();
      delay(10);
    }
  }
}


void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
tmElements_t tm;
void readRTC(){
    if (RTC.read(tm)) {
        Serial.print("Ok, Time = ");
        print2digits(tm.Hour);
        Serial.write(':');
        print2digits(tm.Minute);
        Serial.write(':');
        print2digits(tm.Second);
        Serial.print(", Date (D/M/Y) = ");
        Serial.print(tm.Day);
        Serial.write('/');
        Serial.print(tm.Month);
        Serial.write('/');
        Serial.print(tmYearToCalendar(tm.Year));
        Serial.println();
      } else {
          if (RTC.chipPresent()) {
              Serial.println("The DS1307 is stopped.  Please run the SetTime");
              Serial.println("example to initialize the time and begin running.");
              Serial.println();
          } else {
              Serial.println("DS1307 read error!  Please check the circuitry.");
              Serial.println();
          }
          delay(9000);
      }
      delay(5);
}



void setup() {
    Serial.begin(9600);
    while(!Serial); //wait for serial
    delay(200);
    Serial.println("Start Serial");
    Serial.println("DS2312RTC Read Test");


    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    setTimeColors();


    updateMatrix();
    strip.show();

    Serial.println(loop256(513));
}

void loop() {
  /*for (int i = 0; i < 256; i++){
    setAll(Wheel(i));
      strip.show();
      delay(10);
  }*/
  test();
  readRTC();
}



void init_matrix(){
    for (uint8_t x = 0; x < width; x++){
        for (uint8_t y = 0; y < height; y++){
            ledMatrixState[x][y] = 0;
        }
    }
}
void updateMatrix(){
  uint8_t i = 0;
  for (uint8_t x = 0; x < width; x++){
    if (x%2 == 0){
      i = x*height;
    }else{
      i = x*height + height - 1;
    }
    for (uint8_t y = 0; y < height; y++){
      strip.setPixelColor(i, ledMatrixState[x][y]);
      if (x%2 == 0){
        i++;
      }else{
        i--;
      }
    }
  }
  strip.show();
}
void setLed(uint8_t x, uint8_t y, uint32_t c){
  ledMatrixState[x][y] = c;
}
void setAll(uint32_t c){
  for (uint16_t i = 0; i < strip.numPixels(); i++){
    strip.setPixelColor(i, c);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
