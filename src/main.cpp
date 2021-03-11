#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
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
const int SHORT_PRESS_TIME = 250;
const int LONG_PRESS_TIME = 150;

int lastState = LOW;
unsigned long pressedTime = 0;
unsigned long releasedTime = 0;
bool isPressing = false;
bool isLongDetected = false;

const int buttonPinLeft = 2;   // Left
const int buttonPinSelect = 3; // Select
const int buttonPinRight = 4;  // Right

int buttonStateLeft = LOW;
int buttonStateSelect = LOW;
int buttonStateRight = LOW;

// --------- INTERVALOMETER -----------------------
int timeout = 30;
int timeoutVal = 30;
bool isIntervalRunning = false;
int captureCount = 0;

// --------- MENU ---------------------------------
const int numOfTopMenus = 2;
int currentTopMenu = 0;

const int numOfIntervalScreens = 3;
int currentIntervalScreen = 0;

int counter = 0;

// --------- FUNCTIONS ----------------------------
void callback()
{
  // digitalWrite(10, digitalRead(10) ^ 1);
  // Serial.println("Interupt");
  counter++;

  if (isIntervalRunning)
  {
    timeoutVal--;
    if (timeoutVal < 0)
    {
      timeoutVal = timeout;
    }
  }

  Serial.println(isIntervalRunning);
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

  delay(1000);

  // Buttons
  pinMode(buttonPinLeft, INPUT_PULLUP);
  pinMode(buttonPinSelect, INPUT_PULLUP);
  pinMode(buttonPinRight, INPUT_PULLUP);

  // Setup Timer Interups
  Timer1.initialize(1000000);
  Timer1.pwm(9, 512);
  Timer1.attachInterrupt(callback);

  isIntervalRunning = false;
}

void focusPage()
{
  myOLED.print("** FOCUS PULLER **", CENTER, 0);

  // LEFT
  if (buttonStateLeft == LOW)
  {
    myOLED.invertText(true);
    myOLED.print("<< L", 0, 20);
    myOLED.invertText(false);
  }
  else
  {
    myOLED.invertText(false);
    myOLED.print("<< L", 0, 20);
  }

  //RIGHT
  if (buttonStateRight == LOW)
  {
    myOLED.invertText(true);
    myOLED.print("R >>", 105, 20);
    myOLED.invertText(false);
  }
  else
  {
    myOLED.invertText(false);
    myOLED.print("R >>", 105, 20);
  }
}

void intervPage()
{
  // EXPOSURE PAGE
  if (currentIntervalScreen == 0)
  {
    myOLED.print("** INT/EXPOSURE **", CENTER, 0);

    // LEFT
    if (buttonStateLeft == LOW)
    {
      timeout--;
      timeoutVal = timeout;
      myOLED.invertText(true);
      myOLED.print("-", 0, 20);
      myOLED.invertText(false);
      delay(200);
    }
    else
    {
      myOLED.invertText(false);
      myOLED.print("-", 0, 20);
    }

    //RIGHT
    if (buttonStateRight == LOW)
    {
      timeout++;
      timeoutVal = timeout;
      myOLED.invertText(true);
      myOLED.print("+", 105, 20);
      myOLED.invertText(false);
      delay(200);
    }
    else
    {
      myOLED.invertText(false);
      myOLED.print("+", 105, 20);
    }

    myOLED.printNumI(timeoutVal, CENTER, 20);
  }

  if(currentIntervalScreen == 1)
  {
    myOLED.print("** INT/DELAY **", CENTER, 0);
  }

  if(currentIntervalScreen == 2)
  {
    myOLED.print("** INT/CTRL **", CENTER, 0);

    // LEFT
    if (buttonStateLeft == LOW)
    {
      myOLED.invertText(true);
      myOLED.print("STOP", 0, 20);
      myOLED.invertText(false);
      delay(400);
    }
    else
    {
      myOLED.invertText(false);
      myOLED.print("STOP", 0, 20);
    }

    //RIGHT
    if (buttonStateRight == LOW)
    {
      myOLED.invertText(true);
      myOLED.print("START", 95, 20);
      myOLED.invertText(false);
      delay(400);
    }
    else
    {
      myOLED.invertText(false);
      myOLED.print("START", 95, 20);
    }
    // String s = timeoutVal + String("/") + weight + String(" kg");
    // myOLED.print(s, CENTER, 20);
    // myOLED.printNumI(timeoutVal, CENTER, 20);
  }
}

void loop()
{
  buttonStateLeft = digitalRead(buttonPinLeft);
  buttonStateSelect = digitalRead(buttonPinSelect);
  buttonStateRight = digitalRead(buttonPinRight);

  if (lastState == HIGH && buttonStateSelect == LOW)
  { // button is pressed
    pressedTime = millis();
    isPressing = true;
    isLongDetected = false;
  }
  else if (lastState == LOW && buttonStateSelect == HIGH)
  {
    // button is released
    isPressing = false;
    releasedTime = millis();

    long pressDuration = releasedTime - pressedTime;

    if (pressDuration < SHORT_PRESS_TIME)
    {
      Serial.println("A short press is detected");
      if (currentTopMenu == 1)
      {
        currentIntervalScreen++;
        if (currentIntervalScreen > numOfIntervalScreens)
        {
          currentIntervalScreen = 0;
        }

        /*
      isIntervalRunning = !isIntervalRunning;
      if(!isIntervalRunning)
      {
        timeoutVal = timeout;
      }
      */
      }
    }
  }

  if (isPressing == true && isLongDetected == false)
  {
    long pressDuration = millis() - pressedTime;

    if (pressDuration > LONG_PRESS_TIME)
    {
      Serial.println("A long press is detected");
      isLongDetected = true;

      currentTopMenu++;
      if (currentTopMenu > numOfTopMenus)
      {
        currentTopMenu = 0;
      }
    }
  }

  // save the the last state
  lastState = buttonStateSelect;

  // DRAW SCREEN
  myOLED.clrScr();

  if (currentTopMenu == 0)
  {
    focusPage();
  }
  if (currentTopMenu == 1)
  {
    intervPage();
  }

  if (isIntervalRunning)
  {
    if (counter % 2 == 0)
    {
      myOLED.drawRect(0, 0, 2, 2);
    }
    else
    {
      myOLED.clrRect(0, 0, 2, 2);
    }
  }

  myOLED.update();
}
