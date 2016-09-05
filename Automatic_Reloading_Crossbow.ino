// AUTOMATIC RELOADING CROSSBOW
// This is the code for the final configuration of the automatic reloading crossbow
// The crossbow will automatically draw back the bowstring, raise the trigger, and 
// drop a dart into the firing slot. After finishing the loading procedure, the
// red LED on the crossbow will turn on. The crossbow then determines when to fire. 
// This bow has three different firing modes; guard mode, manual, and automatic.
// in guard mode, the blue LED will be turned off, and the crossbow will use the
// ultrasonic sensor mounted in the front of the bow to determine the distance to the 
// closest object. When this distance changes a set amount or more, the crossbow will 
// fire and reload, waiting for more motion to fire. To turn manual mode on, place a 
// finger over the light sensor. The blue LED will turn on and manual mode will be set. 
// in this mode, merely press the pushbutton next to the light sensor to fire the bow.
// the crossbow will automatically reload, and you can fire again when the red indicator
// turns on. To turn automatic mode on, hold the button for at least 5 seconds. The blue 
// LED will start blinking and the crossbow will fire and reload as fast as it can. 
// To bring it out of automatic mode, press and hold the button again. Have fun! :3


// pin a0 is for the photocell (turn manual mode on)
// pin 2 is for draw motor 1, direction 1
// pin 3 is for draw motor 1, direction 2
// pin 4 is for draw motor 1, speed control
// pin 5 is for draw motor 2, direction 1
// pin 6 is for draw motor 2, direction 2
// pin 7 is for draw motor 2, speed control
// pin 8 is for the loader servo
// pin 9 is for the trigger servo
// pin 10 is for the ultrasonic signal pin
// pin 11 is for the pushbutton (manual mode fire)
// pin 12 is for the red LED (loaded indicator)
// pin 13 is for the blue LED (manual indicator)


#include <Servo.h>
Servo loader;  // create servo object to control the loader
Servo trigger;  // create servo object to control the trigger

// CONSTANT TO DETERMINE WHEN TO FIRE
const int fireDist = 40;   // threshold for guard mode firing, in centimeters

// some constant for the ultrasonic sensor
unsigned long echo = 0;   // define echo for the ultrasonic sensor
int ultraSoundSignal = 10; // ultrasound signal pin
unsigned long ultrasoundValue = 0;

// these are some constants for the draw motors
const int Motor1Direction1 = 2; 
const int Motor1Direction2 = 3;
const int Motor1Speed = 4;
const int Motor2Direction1 = 5;
const int Motor2Direction2 = 6;
const int Motor2Speed = 7;

// these are some constants for the photocell
int photocellPin = 0;     // the cell and 10K pulldown are connected to a0
int photocellReading;     // the analog reading from the analog resistor divider
int photoCheck;     // the ambient lighting comparator

// these are some constants for the pushbutton
int push = 0;     // variable for reading the pin status

// these are some constants for the automatic mode
int checkAuto;   // variable to determine whether in automatic mode or not

void setup() {
  
  loader.attach(8);  // attaches the loader servo on pin 9 to the loader object
  trigger.attach(9);  // attaches the trigger servo on pin 9 to the trigger object

// attaches all the pins to the h bridge stuff
  pinMode(Motor1Direction1, OUTPUT);  
  pinMode(Motor1Direction2, OUTPUT);
  pinMode(Motor1Speed, OUTPUT);
  
  pinMode(Motor2Direction1, OUTPUT);
  pinMode(Motor2Direction2, OUTPUT);
  pinMode(Motor2Speed, OUTPUT);

// debugging print 
  Serial.begin(9600);  
  Serial.println("");
  Serial.println("Welcome to the WinterWaffle Studios Automatic Reloading Crossbow");
  Serial.println("----------------------------------------------------------------");
  Serial.println("        Blue LED on:  Manual Mode on");
  Serial.println("  Blue LED blinking:  Automatic Mode on");
  Serial.println("       Blue LED off:  Guard Mode on");
  Serial.println("         Red LED on:  Crossbow loaded");
  Serial.println("");
  Serial.println("Cover light sensor to turn manual mode on, button to fire");
  Serial.println("Hold button for 5 seconds to turn automatic mode on");
  Serial.println("                 Have fun!");
  Serial.println("----------------------------------------------------------------");
  Serial.println("");
  delay (1000);
  
// initialize photocell stuff
    photocellReading = analogRead(photocellPin);    // check the photocell
    Serial.print("Ambient light reading: ");
    Serial.println(photocellReading);     // the raw analog reading
    if (photocellReading < 300)   // check if light is dim/dark
    {
      photoCheck = 400;
    }
    else
    {
      photoCheck = photocellReading;   // else, determine ambient lighting
    }

// initialize pushbutton stuff
  pinMode(11, INPUT);    // declare pushbutton as input

// initialize LED pin stuff
  pinMode(12, OUTPUT);    // initialize red LED
  pinMode(13, OUTPUT);    // initialize blue LED

}



void loop() {
  // reload the crossbow /////////////////////////////////////////////////////////////
  reload();

  // wait for signal to fire dart/////////////////////////////////////////////////////

  int breakloop = 1;  // set up condition to stop loop

  while (breakloop)
  {
      if (checkHeld())   // check if weapon is being held
      {
       Serial.println("Switching to manual mode"); // switch to manual
       manualMode();
      }
      else
      {
       Serial.println("Continuing guard mode"); // continuing guard mode
       if (checkMotion())   // check if there is motion ahead
       {
            fire();
            breakloop = 0;       // break the loop
       }
      }
  }
}



int checkMotion()
{  
  int x, xOld, xDif;    // setup ping stuff
  int y, yOld, yDif;    // setup ping stuff
  int z, zOld, zDif;    // setup ping stuff
  int i;   
  xOld = ping();
  yOld = ping();
  zOld = ping();
  for (i = 0; i < 10; i++)
  {
       x = ping();
       y = ping();
       z = ping();
       xDif = x - xOld;
       yDif = y - yOld;
       zDif = z - zOld;
       if ((abs(xDif) > fireDist) && (abs(yDif) > fireDist) && (abs(zDif) > fireDist))  // check if there is change in distance
       {
        Serial.print("Ultrasonic distance: ");
        Serial.print(x);
        Serial.print(", ");
        Serial.print(y);
        Serial.print(", ");
        Serial.print(z);
        Serial.print(" cm - ");
        Serial.print("Distance difference: ");
        Serial.print(xDif);
        Serial.print(", ");
        Serial.print(yDif);
        Serial.print(", ");
        Serial.print(zDif);
        Serial.print(" cm - ");
        return 1;
       }
       delay(75); //delay a bit
    x = xOld;
    y = yOld;
    z = zOld; 
   }
   x = ping();
   y = ping();
   z = ping();
   Serial.print("Ultrasonic distance: ");
   Serial.print(x);
   Serial.print(", ");
   Serial.print(y);
   Serial.print(", ");
   Serial.print(z);
   Serial.print(" cm - ");
   return 0;
}



int checkHeld()
{
    photocellReading = analogRead(photocellPin);    // check the photocell
    Serial.print("Light reading: ");
    Serial.print(photocellReading);     // the raw analog reading
    Serial.print(" - ");
    if (photocellReading < (photoCheck - 200))   // check if weapon is held
    {
      return 1;
    }
    return 0;
}



int checkPushed()
{
  int i;
  for (i = 0; i < 15; i++)
  {
    int held = 0;
    while (digitalRead(11) == HIGH)
    {
      delay(100);
      held++;
    }
    if (held > 50)
    {
    Serial.print("Button pushed for more than 5 seconds - ");   // print if held
    return 2;  // the button was pressed for more than 5 seconds
    }
    else if (held > 0)
    {
    Serial.print("Button pushed - ");   // print if pushed  
    return 1;  // the button was pressed for less than 5 seconds
    }
    delay(50);
  }
  Serial.print("Button unpushed - ");   // print if no pushed
  return 0;   // the button was not pressed at all
}



void manualMode()
{
  digitalWrite(13, HIGH);    // turn the blue LED on by making the voltage HIGH
  int testPushed;
  while (1)
   {
    testPushed = checkPushed();
    if (testPushed == 1)
    {
      fire();     // fire and reload
      reload();
    }
    else if (testPushed == 2)
    {
      automaticMode();
    }
    if (checkHeld())   // keep checking if weapon is held
    {
        Serial.println("Continuing manual mode");
    }
    else
    {
        Serial.println("Switching to guard mode");
        delay(1000); //delay a bit 
        digitalWrite(13, LOW);    // turn the blue LED off by making the voltage LOW
        return;
    }
  }
}



void automaticMode()
{
  checkAuto = 1;
  int i;
  for (i = 0; i < 10; i++)     // do the blue LED blink sequence
  {
  digitalWrite(13, HIGH);    // turn the blue LED on by making the voltage HIGH
  delay (500);
  digitalWrite(13, LOW);    // turn the blue LED off by making the voltage LOW
  delay (500);
  }
  Serial.println("Entering automatic mode");
  while (1)
  {
  reload();
  if (checkPushed() > 0)
  {
    checkAuto = 0;
    Serial.println("Exiting automatic mode");
    for (i = 0; i < 5; i++)     // do the blue LED blink sequence
    {
    digitalWrite(13, HIGH);    // turn the blue LED on by making the voltage HIGH
    delay (500);
    digitalWrite(13, LOW);    // turn the blue LED off by making the voltage LOW
    delay (500);
    }
    return;
  }
  Serial.println("Continuing automatic mode");
  }
  fire();
}



void reload()
{
  Serial.println("Reloading... please wait");
  loader.attach(8);  // reattach loader servo
  
  // pulling back the draw motors ////////////////////////////////////////////////////
  digitalWrite(Motor1Direction1, HIGH);
  digitalWrite(Motor1Direction2, LOW);
  analogWrite(Motor1Speed, 256);
  digitalWrite(Motor2Direction1, HIGH);
  digitalWrite(Motor2Direction2, LOW);
  analogWrite(Motor2Speed, 256);
  trigger.write(170);   // set the trigger up
  loader.write(180);  // still chambering another dart
  if (checkAuto)
  {
    digitalWrite(13, LOW);    // blink blue LED to show you're in automatic mode
  }
  delay(2500);  // delay to wait for pullback

  // stopping the draw motors, set up trigger and loader /////////////////////////////
  digitalWrite(Motor1Direction1, HIGH);
  digitalWrite(Motor1Direction2, LOW);
  analogWrite(Motor1Speed, 0);
  digitalWrite(Motor2Direction1, HIGH);
  digitalWrite(Motor2Direction2, LOW);
  analogWrite(Motor2Speed, 0);
  loader.write(170);  // set the trigger up
  trigger.write(80);   // set the trigger up
  if (checkAuto)
  {
    digitalWrite(13, HIGH);    // blink blue LED to show you're in automatic mode
  }
  delay(2500);  // delay to wait for setup 

  // unwind the draw motors /////////////////////////////////////////////////////////
  digitalWrite(Motor1Direction1, LOW);
  digitalWrite(Motor1Direction2, HIGH);
  analogWrite(Motor1Speed, 256);
  digitalWrite(Motor2Direction1, LOW);
  digitalWrite(Motor2Direction2, HIGH);
  analogWrite(Motor2Speed, 256);
  loader.write(30);  // still dropping the dart
  trigger.write(80);   // still setting the trigger up

  delay(1500);  // delay to wait for draw motors to unwind

  // stop the draw motors ///////////////////////////////////////////////////////////
  digitalWrite(Motor1Direction1, LOW );
  digitalWrite(Motor1Direction2, HIGH);
  analogWrite(Motor1Speed, 0);
  digitalWrite(Motor2Direction1, LOW);
  digitalWrite(Motor2Direction2, HIGH);
  analogWrite(Motor2Speed, 0);
  
  loader.detach();  // turn off the loader servo to save power
  digitalWrite(12, HIGH);    // turn the red LED high by making the voltage HIGH
  return;
}



void fire()
{
  Serial.println("Firing!");
  trigger.write(170);   // release the triggger
  digitalWrite(12, LOW);    // turn the red LED off by making the voltage LOW
  
  return;
}



unsigned long ping()
{ 
  pinMode(ultraSoundSignal, OUTPUT); // Switch signalpin to output
  digitalWrite(ultraSoundSignal, LOW); // Send low pulse 
  delayMicroseconds(2); // Wait for 2 microseconds
  digitalWrite(ultraSoundSignal, HIGH); // Send high pulse
  delayMicroseconds(5); // Wait for 5 microseconds
  digitalWrite(ultraSoundSignal, LOW); // Holdoff
  pinMode(ultraSoundSignal, INPUT); // Switch signalpin to input
  digitalWrite(ultraSoundSignal, HIGH); // Turn on pullup resistor
  // please note that pulseIn has a 1sec timeout, which may
  // not be desirable. Depending on your sensor specs, you
  // can likely bound the time like this -- marcmerlin
  // echo = pulseIn(ultraSoundSignal, HIGH, 38000)
  echo = pulseIn(ultraSoundSignal, HIGH); //Listen for echo
  ultrasoundValue = (echo / 58.138); //convert to centimeters
  return ultrasoundValue;
}
