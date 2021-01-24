#include <Arduino.h>
#include "MHZ19.h"
#include <Thread.h>
#include <ThreadController.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BAUDRATE 9600                                      // Device to MH-Z19 Serial baudrate (should not be changed)
MHZ19 myMHZ19;                                             // Constructor for library

ThreadController threadControl = ThreadController();
Thread threadReadMHZ = Thread();

int CO2;
int8_t Temp;

void setup()
{
  Serial.begin(9600);                                     // Device to serial monitor feedback
  TXLED0;
  RXLED0;

  Serial1.begin(BAUDRATE);                               // (Uno example) device to MH-Z19 serial start
  myMHZ19.begin(Serial1);                                // *Serial(Stream) refence must be passed to library begin().
  myMHZ19.autoCalibration();                              // Turn auto calibration ON (OFF autoCalibration(false))

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();

  threadReadMHZ.onRun(readMHZ);
  threadReadMHZ.setInterval(2000);
  threadControl.add(&threadReadMHZ);
}


void readMHZ()
{
  /* note: getCO2() default is command "CO2 Unlimited". This returns the correct CO2 reading even
    if below background CO2 levels or above range (useful to validate sensor). You can use the
    usual documented command with getCO2(false) */

  CO2 = myMHZ19.getCO2();                             // Request CO2 (as ppm)
  Serial.print("CO2 (ppm): ");
  Serial.println(CO2);

  Temp = myMHZ19.getTemperature();                     // Request Temperature (as Celsius)
  Serial.print("Temperature (C): ");
  Serial.println(Temp);
  updateDisplay();
}


void updateDisplay()
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.print(F("CO2  ")); display.println(CO2); //display.println(F("ppm"));
  if (millis()<180000) {
    display.print(F("Warmup ")); display.print((int)(180-millis()/1000)); display.println(F("s"));
  } else {
    display.print(F("Temp ")); display.print(Temp); display.println(F("C"));
  }
  display.display();
}


void loop()
{
  threadControl.run();
}
