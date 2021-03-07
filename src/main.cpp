#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
#include <CheapStepper.h>
#include "TimerOne.h"
#include <OLED_I2C.h>

#define DEBUG true

// --------- Display ----------------------------
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

OLED myOLED(SDA, SCL);
extern uint8_t SmallFont[];
extern uint8_t TinyFont[];

// ---------- Stepper ----------------------------
CheapStepper stepper(A0, A1, A2, A3);
#define stepsPerRevolution 4096
float stepsDelayMicroscondsDecimal = ((stepsPerRevolution * 20.0) / 8.8838);
int stepsDelayMicrosconds = stepsDelayMicroscondsDecimal;
bool machineState[] = {false, true}; //machineState[power, direction] --> [Off, Counter-Clockwise]

// ---------- Buttons ----------------------------
const int buttonPinLeft = 2; // Left
const int buttonPinSelect = 3; // Select
const int buttonPinRight = 4; // Right

int buttonStateLeft = 0; 
int buttonStateSelect = 0;
int buttonStateRight = 0;

// --------- MENU ---------------------------------
const int numOfScreens = 2;
int currentScreen = 0;
int selected = 0;

String screens[numOfScreens] = {"FOCUS", ""};

// --------- FUNCTIONS ----------------------------
void callback()
{
  // digitalWrite(10, digitalRead(10) ^ 1);
  Serial.println("Interupt");
}

void focusStop()
{
  if (DEBUG)
  {
    Serial.println("Focus stopped");
  }
  machineState[0] = false; // Turn the machine on.
}

void focusLeftStart()
{
  if (DEBUG)
  {
    Serial.println("Focus left started");
  }

  machineState[1] = false; // Direction
  machineState[0] = true;  //turn the machine on.
}

void focusRightStart()
{
  if (DEBUG)
  {
    Serial.println("Focus right started");
  }

  machineState[1] = true; // Direction
  machineState[0] = true; //turn the machine on.
}

// --------------------------------------------------------------

void setup()
{
  Serial.begin(9600);

  myOLED.begin(SSD1306_128X32);
  myOLED.setFont(SmallFont);
  myOLED.clrScr();
  myOLED.print("Focus Puller", CENTER, 0);
  myOLED.print("***", CENTER, 10);
  myOLED.print("Intervalometer", CENTER, 20);
  myOLED.update();

  delay(500);
  
  // Buttons
  pinMode(buttonPinLeft, INPUT_PULLUP);
  pinMode(buttonPinSelect, INPUT_PULLUP);
  pinMode(buttonPinRight, INPUT_PULLUP);

  // Setup Timer Interups
  Timer1.initialize(1000000);
  Timer1.pwm(9, 512);
  Timer1.attachInterrupt(callback);
}

void loop()
{
  buttonStateLeft = digitalRead(buttonPinLeft);
  buttonStateSelect = digitalRead(buttonPinSelect);
  buttonStateRight = digitalRead(buttonPinRight);

   if(buttonStateLeft == LOW){
     Serial.print("Left");

     myOLED.clrScr();
     myOLED.print("Left", CENTER, 0);
     myOLED.update();
   }

   if(buttonStateSelect == LOW){
     Serial.print("Select");

     myOLED.clrScr();
     myOLED.print("Select", CENTER, 0);
     myOLED.update();
   }

   if(buttonStateRight == LOW){
     Serial.print("Right");

     myOLED.clrScr();
     myOLED.print("Right", CENTER, 0);
     myOLED.update();
   }
}
