// SwagBlaster 3001 code
// wire guide:
// yellow- gate1                            pin 2         
// taped purple- gate2                      pin 3
// taped white- pusher motor mosfet         pin 4
// taped brown- pusher motor switch         pin 5
// taped yellow- trigger switch signal      pin 6
// green- flywheel motor switch signal      pin 7                                                             
// blue- firing mode button                 pin 8             // grey- flywheel motor mosfet              pin 8
// taped orange- magazine gate              pin 9
// brown- rotary encoder pushbutton         pin 10
// taped blue - rev LED signal              pin 11
// grey- rotary encoder 1                   pin 12
// white- rotary encoder 2                  pin 13
// yellow wire - magazine light red         pin 15 (A1)
// red wire tape - magazine light blue      pin 16 (A2)

// resistors - for voltage detection        pin A0
// orange - LCD SDA                         pin A4
// purple - LCD SCL                         pin A5

//----------------------------------INCLUDE STUFF------------------------------------------------------------------------------------
#include <Wire.h>                 // some other required library
#include <LCD.h>                  // LCD library
#include <LiquidCrystal_I2C.h>    // another LCD library
#include <EnableInterrupt.h>      // the interrupt library

//---------------------------------OTHER CONSTANTS-----------------------------------------------------------------------------------
const int burstConst = 3; // CONSTANT TO DETERMINE HOW MANY DARTS FIRED IN BURST FIRE MODE
const int batteryCells = 8;   // define the number of AA NiMH batteries there are

//----------------------------------PIN CONSTANTS------------------------------------------------------------------------------------
const int gatePin1 = 2;
const int gatePin2 = 3;
const int pushFet = 4;
const int pushSwitch = 5;
const int trigSwitch = 6;
const int flySwitch = 7;
const int modeSwitch = 8;
const int magazinePin = 9;
const int encoderButton = 10;
const int revLEDPin  = 11;
const int encoderPin1 = 12;
const int encoderPin2 = 13;
const int magazineRed = 15;
const int magazineBlue = 16;
#define sensePin A0     // analog pin for voltmeter

//-----------------------------------TIMER VARIABLES----------------------------------------------------------------------------------
long modeTimer; 
volatile long pushTimer;   
long battTimer;
volatile long triggerTimer;
volatile long gate1Millis; 
volatile long gate2Millis;
long revMillis;     

//-----------------------------------ENCODER VARIABLES---------------------------------------------------------------------------------
volatile int lastEncoded = 0; 
volatile long encoderValue = 0;
long lastencoderValue = 0;
int lastMSB = 0;      // most significant bit
int lastLSB = 0;      // least significant bit

//----------------------------------DEBOUNCING VARIABLES--------------------------------------------------------------------------------
int encoderDebounce = 0;    // debouncing for encoder
int modeDebounce = 0;       // debouncing for mode button
int magazineDebounce = 0;   // debouncing for magazine detector

//------------------------------------RANDOM VARIABLES----------------------------------------------------------------------------------
int modeStatus = 0;       // constant to check mode:   0 = semi-auto fire, 1 = burst fire, 2 = automatic fire
volatile int fireCount = 0;        // variable to check how many times we've fired
int muzzleSpeed = 0;      // variable which holds most recent muzzle speed
int dartsTotal = 22;       // variable to check the total amount of darts in the magazine
volatile int dartsLeft = 0;        // variable to check the darts left in the magazine
volatile int updateDarts = 0;      // variable to tell us when to update dart counter
int voltage;          // variable to hold the current battery voltage
int magazineOut;      // variable to check if the magazine is in or not
int trigState;       // variable to hold the debounced state of the trigger
int reading;          // current un-debounced value
int counter = 0;      // variables for counting debounce timess
int revLED = 0;         // analog variable for controlling the rev LED
volatile int dartFired = 0;     // variable that tells when a dart is fired
 
//---------------------------------------LCD STUFF-------------------------------------------------------------------------------------
LiquidCrystal_I2C  lcd(0x27,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified module


//----------------------------------------SETUP STUFF----------------------------------------------------------------------------------  
void setup() 
{
  pinMode(pushFet, OUTPUT);     // enable pin stuff
  pinMode(pushSwitch, INPUT);
  pinMode(trigSwitch, INPUT);
  pinMode(flySwitch, INPUT);
  pinMode(modeSwitch, INPUT);
  pinMode(sensePin, INPUT);
  pinMode(magazineRed, OUTPUT);
  pinMode(magazineBlue, OUTPUT);
  pinMode(encoderButton, INPUT);
  pinMode(encoderPin1, INPUT);
  pinMode(encoderPin2, INPUT);
  digitalWrite(encoderPin1, HIGH); //turning pullup resistor on
  digitalWrite(encoderPin2, HIGH); 
  digitalWrite(encoderButton, HIGH); 
  

  enableInterrupt(pushSwitch, semi, RISING);     // enable interrupt stuff
  enableInterrupt(trigSwitch, trigger, RISING);
  enableInterrupt(gatePin1, gate1, FALLING);    // stuff for photogates
  enableInterrupt(gatePin2, gate2, FALLING);                                         
  enableInterrupt(encoderPin1, updateEncoder, CHANGE);
  enableInterrupt(encoderPin2, updateEncoder, CHANGE);  

  analogReference(DEFAULT);     // setting voltage reference to 5V
  
  Serial.begin(9600);
  Serial.println("Welcome to the SwagBlaster v1");

 LCDstart(); // lcd start routine

}

void loop() 
{
//-------------------------------------MODE SWITCH CHECKER-----------------------------------------------------------------------------
  if (digitalRead(modeSwitch))    // if the glowy button pressed, change modes
  {
    if (modeDebounce == 0)
    {
      modeDebounce++;
      return;
    }
    modeDebounce = 0;  
    modeChange();
  }
  else
  {
    modeDebounce = 0;
  }
//-------------------------------------BATTERY CHECKER---------------------------------------------------------------------------------  
  if (millis() - battTimer > 250)    // when to update battery
  {
     LCDupdate(1);
     battTimer = millis();
     gate1Millis = 0;
     gate2Millis = 0;
  }
//-----------------------------------DART SPEED CALCULATOR-----------------------------------------------------------------------------
  if (gate1Millis != 0 && gate2Millis != 0)   // if a dart has passed through the barrel, check speed
  {
  unsigned int travelMillis = gate2Millis - gate1Millis;    // calculate time difference
  float speed = (1000/(travelMillis))*.625;     // calculate speed
  if ((speed > 5) && (speed < 200))
  {
    speed = speed + random(-5,5);
//    Serial.print(speed);
//    Serial.println(" fps");
    muzzleSpeed = speed;
    LCDupdate(0);
  }
  gate1Millis = 0;      // reset the gate milli 
  gate2Millis = 0;
  }
//----------------------------------------DART FIRED -----------------------------------------------------------------------------------
if (dartFired == 1)
{
    dartFired = 0;
    dartsLeft--;    // decrement dartsLeft by one
    encoderValue = encoderValue - 4;    // decrement encoderValue by 4
    revLED = 25;   // -----------------------------this is what the lights go back to every time a dart is fired
    analogWrite(revLEDPin, revLED*2); // updating the rev LED 
    if (dartsLeft == -1)
    {
      dartsLeft = 0;
      encoderValue = 0;
    }
    LCDupdate(3);   // update the dart counter too!
}

//------------------------------------ROTARY BUTTON STUFF-------------------------------------------------------------------------------
  if(digitalRead(encoderButton) == 0)   // if the rotary button is pressed
  {
      if (encoderDebounce == 0)
      {
        encoderDebounce++;
        return;
      }
//      Serial.println("button pressed");
      dartsTotal = dartsLeft;
      encoderDebounce = 0;
      LCDupdate(3);   // update dart counter
  }
  else  // encoder debouncing stuff
  {
    encoderDebounce = 0;
  }
//---------------------------WHEN TO UPDATE DARTS BASED ON ROTARY ENCODER---------------------------------------------------------------
  if (updateDarts)
  {
    if(encoderValue<0)
    {
      encoderValue = 0;
    }
    else if(encoderValue>396)
    {
      encoderValue = 396;
    }
    if (magazineOut)
    {
      dartsLeft = 0;
      encoderValue = 0;
    }
    dartsLeft = encoderValue/4;   // update the dart total
    LCDupdate(3);
    updateDarts = 0;
//    Serial.println("darts updated");
  }
  
//------------------------------------TRIGGER DEBOUNCING--------------------------------------------------------------------------------
  reading = digitalRead(trigSwitch);

  if(reading == trigState && counter > 0)
  {
    counter--;
  }
  if(reading != trigState)
  {
     counter++; 
  }
  // If the Input has shown the same value for long enough let's switch it
  if(counter >= 30)     // 20 IS DEBOUNCECOUNT
  {
    counter = 0;
    trigState = reading;
  }
//--------------------------------------MAGAZINE GATE------------------------------------------------------------------------------------
  reading = digitalRead(magazinePin);
  if(reading == magazineOut && magazineDebounce > 0)
  {
    magazineDebounce--;
  }
  if(reading != magazineOut)
  {
     magazineDebounce++; 
  }
  // If the Input has shown the same value for long enough let's switch it
  if(magazineDebounce > 5000)     // 5000 IS DEBOUNCECOUNT
  {
    magazineDebounce = 0;
    magazineOut = reading;
//    Serial.println("changed magazine");
    if(magazineOut == 0)
    {
//      Serial.println("reset darts");
      encoderValue = dartsTotal*4;
      dartsLeft = dartsTotal;
      LCDupdate(3);
    }
    if(magazineOut)
    {
      encoderValue = 0;
      dartsLeft = 0;
      LCDupdate(3);
    }
  }
  if (magazineOut)
  {
    digitalWrite(magazineRed, HIGH);
    digitalWrite(magazineBlue, LOW);
  }
  else
  {
    digitalWrite(magazineRed, LOW);
    digitalWrite(magazineBlue, HIGH);
  }

//--------------------------------------FLYWHEEL LEDS------------------------------------------------------------------------------------

  if (digitalRead(flySwitch) && (revMillis + 15) < millis())      // increasing
  {
//        Serial.println("flyingggg");
    revLED ++;
    if (revLED > 95)     // setting the upper and lower limits
    {
       revLED = 95;
    }
    revMillis = millis();
    analogWrite(revLEDPin, revLED*2);
  }
  else if ((revMillis + 50) < millis() && (revLED != 0))      // decreasing 
  {
//        Serial.println("crashingggg");
    if (revLED > 42)    // faster decreasing if high
    {
      revLED = revLED - 5;
    }
    revLED --;
    if (revLED < 0)
    {
      revLED = 0;
    }
    revMillis = millis();
    analogWrite(revLEDPin, revLED*2);
  }



// end of loop---
}

//-----------------------------------MODECHANGE FUNCTION----------------------------------------------------------------------------------
void modeChange()
// button got pressed, change the mode
{
  if (millis() - modeTimer < 750)
  {
    return;
  }
   modeStatus++;  // increment!
   Serial.println(modeStatus);  
   if (modeStatus > 2)   // if firing mode is above 2, reduce the mode to 0
   {
    modeStatus = 0;
   }
   if (modeStatus == 0)
   {
    enableInterrupt(pushSwitch,semi,RISING);     // this stuff makes it single fire
//    Serial.println("we in mode 0");
   }
   else if (modeStatus == 1)
   {
    enableInterrupt(pushSwitch,burst,RISING);
//    Serial.println("we in mode 1");
   }
   else if (modeStatus == 2)
   {
    enableInterrupt(pushSwitch,automatic,RISING);
//    Serial.println("we in mode 2");    
   }
 lcd.off();
 delay(25);
 lcd.on();
 LCDupdate(4);
 LCDupdate(2);    //update the firing mode on the LCD
 modeTimer = millis();
 return;
}

//-------------------------------------TRIGGER INTERRUPT-------------------------------------------------------------------------------------
void trigger()
{
  if(trigState)
  {
    return;
  }
  digitalWrite(pushFet, HIGH);
//                                               Serial.println("trigger pulled");
  fireCount = 0;
}

//--------------------------------------SEMI INTERRUPT----------------------------------------------------------------------------------------
void semi()
{
  if (digitalRead(pushSwitch))
  {
    digitalWrite(pushFet, LOW);
//                                                Serial.println("pusher reset");
  }
}

//--------------------------------------BURST INTERRUPT---------------------------------------------------------------------------------------
void burst()
{
  if (digitalRead(pushSwitch))
  {
    if (millis() - pushTimer < 50)
    {
      return;
    }
    fireCount++;
//                                                  Serial.print("current count is");
    Serial.println(fireCount);
    if (fireCount == 3)
    {
      digitalWrite(pushFet, LOW);
//                                                    Serial.println("now we done");
    }
    pushTimer = millis();
  }
}

//------------------------------------AUTOMATIC INTERRUPT-------------------------------------------------------------------------------------
void automatic()
{
//  if (digitalRead(pushSwitch) == 0)
  {
    if (digitalRead(trigSwitch))
    {
      return;
    }
    digitalWrite(pushFet, LOW);
  }
}

//------------------------------------GATE1 INTERRUPT----------------------------------------------------------------------------------------          
void gate1()
// stuff for photogate interrupts
{
  if (gate1Millis != 0)
  {
    return;
  }
  if(digitalRead(gatePin1))
  {
    return;
  }
  gate1Millis = millis();
  dartFired = 1;
//  Serial.println("interrupt 1");
}

//----------------------------------------GATE2 INTERRUPT--------------------------------------------------------------------------------------
void gate2()
{
  if (gate2Millis != 0)
  {
    return;
  }
  if(digitalRead(gatePin2))
  {
    return;
  }
  gate2Millis = millis();
//  Serial.println("interrupt 2");
}

//--------------------------------------UPDATEENCODER INTERRUPT----------------------------------------------------------------------------------
void updateEncoder()
{
    int MSB = digitalRead(encoderPin1); //MSB = most significant bit
    int LSB = digitalRead(encoderPin2); //LSB = least significant bit
  
    int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
    int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
  
    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;
  
    lastEncoded = encoded; //store this value for next time
    updateDarts = 1;
  }
  
//-------------------------------------CUSTOM CHARACTER STUFF----------------------------------------------------------------------------------
byte battleft[8] = {
  B00111,
  B00110,
  B00110,
  B00110,
  B00110,
  B00110,
  B00111,
};
byte battfull[8] = {
  B11111,
  B00000,
  B11111,
  B11111,
  B11111,
  B00000,
  B11111,
};
byte battempty[8] = {
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
};
byte battright[8] = {
  B11100,
  B01100,
  B01110,
  B01110,
  B01110,
  B01100,
  B11100,
};

//----------------------------------------LCD START FuNCTION----------------------------------------------------------------------------------
void LCDstart()
{
  Serial.println("doing lcd stuff"); 
  lcd.begin(20, 4);     // initialize LCD
  lcd.setBacklightPin(3,POSITIVE);      // initialize backlight
  lcd.backlight(); // turn backlight on
  
  lcd.createChar(0,battleft);         // create custom characters
  lcd.createChar(1,battfull);
  lcd.createChar(2,battempty);
  lcd.createChar(3,battright);   
  
  lcd.clear();      // clear
  lcd.setCursor(3,0);       // display title and stuff
  lcd.print("Welcome to the");
  lcd.setCursor(2,1);
  lcd.print("SwagBlaster 3001");
  delay(750);
  lcd.setCursor(0,3);
  lcd.print("Loading:");      // do the fancy loading bar thing
  digitalWrite(magazineRed, HIGH);    // turn on magazine gate LED
  digitalWrite(magazineBlue, LOW);
  for (int i=0; i<12; i++)
  {
    delay(random(50,420));      // delay for random time 
    lcd.print(char(255));
    analogWrite(revLEDPin, i*12);
  }
  delay(750);
  lcd.setCursor(0,3);       // clear the bottom line
  lcd.print("                    ");
  lcd.setCursor(2,3);
  lcd.print("Loading Complete");
  delay(1200);
  lcd.setCursor(0,3);       // clear the bottom line
  lcd.print("                    ");
  for (int i=0; i<2; i++)
  {
    lcd.setCursor(6,3);
    lcd.print("Have fun!");     // do the flashy have fun
    delay(350);
    lcd.setCursor(6,3);
    lcd.print("         ");
    delay(150);
  }
  lcd.setCursor(6,3);
  lcd.print("Have fun!");
  delay(1000);
  lcd.clear();      // clear for next display

  lcd.home();
  lcd.print("SwagBlaster 3001");
  lcd.setCursor(0,1);
  lcd.print("Battery:       ");
  lcd.write(byte(0));
  lcd.print("   ");
  lcd.write(byte(3));
  lcd.setCursor(0,3);
  lcd.print("Darts Left:   /");

                // the first round of updates
  LCDupdate(1);
  LCDupdate(2);
  LCDupdate(3);
  Serial.println("done with LCD stuff");
  analogWrite(revLEDPin, 0);
  return;
}

//---------------------------------------LCD UPDATE FUNCTION------------------------------------------------------------------------------------
void LCDupdate(int m)
{
  // m denotes what to update: 0=muzzle speed, 1=battery, 2=mode, 3=dart counter
  if (m==0)
  {
    lcd.setCursor(0,0);
    lcd.print("                    ");  
    lcd.setCursor(0,0); 
    lcd.print("Muzzle Speed: ");
    lcd.print(muzzleSpeed);
    lcd.print(" fps");
    return;
  }
  if (m==1)
  {
    int rawvolt = analogRead(sensePin);
    float voltage = rawvolt*.0156403;
    lcd.setCursor(9,1);
    lcd.print(voltage);
    lcd.setCursor(13,1);
    lcd.print("V ");
    lcd.setCursor(16,1);
    if (voltage > 1.2*batteryCells)   // this is higher end threshold for NiMH battery
    {
      lcd.write(byte(1));
      lcd.write(byte(1));
      lcd.write(byte(1));
      return;
    }
    if (voltage > 1.1*batteryCells)   // medium ish
    {
      lcd.write(byte(2));
      lcd.write(byte(1));
      lcd.write(byte(1));
      return;
    }
     if (voltage > batteryCells)    // low threshold
    {
      lcd.write(byte(2));
      lcd.write(byte(2));
      lcd.write(byte(1));
      return;
    }
        // replace your batteries!
     lcd.write(byte(2));
     lcd.write(byte(2));
     lcd.write(byte(2));
     return;
  }
  if (m==2)     //this will write out the firing mode
  {
    lcd.setCursor(0,2);   
    if (modeStatus==0)    // depending on mode status
    {
      lcd.print("Semi-Automatic mode ");
      return;
    }
    if (modeStatus==1)
    {
      lcd.print("Burst-Fire mode     ");
      return;
    } 
    lcd.print("Automatic-Fire mode ");   
  }
  if (m==3)   // this will write out the dart counter
  {
    lcd.setCursor(0,3);
    if (magazineOut)
    {
      lcd.print("No Magazine: ");
    }
    else if (dartsLeft < (dartsTotal/5))   // low ammo threshold
    {
      lcd.print("LOW AMMO!!!   /");
    }
    else
    {
      lcd.print("Darts Left:   /");
    }
    lcd.setCursor(12,3);
    if (dartsLeft < 10)
    {
      lcd.setCursor(13,3);
    }
    lcd.print(dartsLeft);
    lcd.setCursor(15,3);
    lcd.print(dartsTotal);
    lcd.print("   ");
    return;
  }
  if (m==4)       // every time we reset the LCD
  {
  lcd.setCursor(0,1);
  lcd.print("Battery:       ");
  lcd.write(byte(0));
  lcd.print("   ");
  lcd.write(byte(3));
  lcd.setCursor(0,3);
  lcd.print("Darts Left:   /");

  LCDupdate(0);    // the first round of updates
  LCDupdate(1);
  LCDupdate(2);
  LCDupdate(3);
  }
}





//-------------------------------THE FUNCTION GRAVEYARD-----------------------------------------------------------------------------------------
/* 
*/ 


