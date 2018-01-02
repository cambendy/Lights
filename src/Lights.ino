#include <limits.h>
#include <FastLED.h>
#include <Bounce2.h>
#include <QueueArray.h>

#define MODE_PIN 2
#define FUNC_PIN 3
int brightnessDial = A0;          // select the input pin for the potentiometer
int ledPin = 13;                  // select the pin for the LED

bool ledsOn = false;

unsigned int sensorValue = 0;              // variable to store the value coming from the sensor

bool updateMode = false;
unsigned long updateModeTime = 0; // Used to time out MODE changing

#define BRIGHTNESS 32
#define FRAMES_PER_SECOND 12

uint8_t gHue = 0;                 // rotating "base color" used by many of the patterns

#define NUM_SETTINGS_LEDS 50
#define SETTINGS_LEDS_PIN 4

#define NUM_TREETOP_LEDS 50
#define TREETOP_LEDS_PIN 5

#define NUM_BRANCH_LEDS 100
#define NUM_TREE_LEDS 650


#define TREE2_LEDS_PIN 6
#define TREE3_LEDS_PIN 7
#define TREE4_LEDS_PIN 8
#define TREE5_LEDS_PIN 9
#define TREE6_LEDS_PIN 10
#define TREE7_LEDS_PIN 11

CRGB allTreeLEDS[NUM_TREE_LEDS];

// Instantiate a Bounce object for the two buttons
Bounce modeButton = Bounce();
Bounce funcButton = Bounce();

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

QueueArray <uint16_t > glitterQ;

void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(MODE_PIN, INPUT_PULLUP);
  pinMode(FUNC_PIN, INPUT_PULLUP);

  modeButton.attach(MODE_PIN);
  modeButton.interval(15); // interval in ms

  funcButton.attach(FUNC_PIN);
  funcButton.interval(15); // interval in ms

  FastLED.addLeds<WS2811, TREE7_LEDS_PIN>(allTreeLEDS, 0, NUM_BRANCH_LEDS);
  FastLED.addLeds<WS2811, TREE6_LEDS_PIN>(allTreeLEDS, 1*NUM_BRANCH_LEDS, NUM_BRANCH_LEDS);
  FastLED.addLeds<WS2811, TREE5_LEDS_PIN>(allTreeLEDS, 2*NUM_BRANCH_LEDS, NUM_BRANCH_LEDS);
  FastLED.addLeds<WS2811, TREE4_LEDS_PIN>(allTreeLEDS, 3*NUM_BRANCH_LEDS, NUM_BRANCH_LEDS);
  FastLED.addLeds<WS2811, TREE3_LEDS_PIN>(allTreeLEDS, 4*NUM_BRANCH_LEDS, NUM_BRANCH_LEDS);
  FastLED.addLeds<WS2811, TREE2_LEDS_PIN>(allTreeLEDS, 5*NUM_BRANCH_LEDS, NUM_BRANCH_LEDS);
  FastLED.addLeds<WS2811, TREETOP_LEDS_PIN>(allTreeLEDS, 6*NUM_BRANCH_LEDS, NUM_TREETOP_LEDS);

  Serial.begin(9600);
  glitterQ.setPrinter(Serial);
  Serial.println("Initialised");
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*ModeList[])();

ModeList gModes = { blueLights,  
                    blueWithGlitter, 
                    oneColor, 
                    myrainbow, 
                    myrainbowWithGlitter, 
                    confetti, 
                    blueRedLights, 
                    justGlitter, 
                    rainbow,
                    rotateAllModes} ;
const char *Modes[] = { "blueLights", 
                        "blueWithGlitter", 
                        "oneColor", 
                        "myrainbow", 
                        "myrainbowWithGlitter", 
                        "confetti", 
                        "blueRedLights", 
                        "justGlitter", 
                        "rainbow"
                        "rotateAllModes"};

uint8_t gCurrentMode = (ARRAY_SIZE(gModes) - 1); // Index number of which mode is current - start at rotateAllModes

void loop()
{
  modeButton.update();
  funcButton.update();

  // Call the current pattern function once, updating the 'leds' array
  gModes[gCurrentMode]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  if (modeButton.fell())
  {
    ledsOn = !ledsOn;
    nextMode();
    Serial.println("Button Pressed");
  }
 
  SettingsUpdate();

  EVERY_N_MILLISECONDS(20)
  {
    gHue++; // slowly cycle the "base color" through the rainbow
  }
}

bool updateSettings = false;
unsigned long updateSettingsTime = ULONG_MAX; // Used to time out MODE changing
int prevAnalogVal = 0;
int analogVal;

bool isFade = false;

struct SetVars{       //Defaults
  int BlueFade;       //25
  int BlueRand;       //280
  int BlueGlitter;    //100

  int ColfadeVal;     //25
  int ColrandVal;     //280

  int RainbowVal;     //64
  int RainbowSat;     //254

  int RainbowGlitter; //25

  int BlueRedFade;    //25
  int BlueRedRand;    //280

  int JustGlitterFade;//90
  int JustGlitterRand;//100
   
};

SetVars mySet = {25,280,100, 25,280, 64, 254, 25, 25, 280, 90, 100};

void SettingsUpdate()
{
  
  if (funcButton.fell()){
    updateSettingsTime = millis();
  }

  while ((millis() - 3000) < updateSettingsTime){
    FlashLED();
    analogVal = analogRead(brightnessDial);
    if (abs(analogVal - prevAnalogVal) > 10) {
      //The dial was touched!
      Serial.println("Dial touched");
      updateSettingsTime = millis();
    }

      switch (gCurrentMode){
        case 0: //blueLights
          if (funcButton.fell())
          {
            updateSettingsTime = millis();
//            Serial.println("Func pressed");
            isFade = !isFade;
          }
          if (isFade)
          {
            mySet.BlueFade = map(analogRead(brightnessDial), 0, 1023, 1, 50);
          }
          else
          {
            mySet.BlueRand = map(analogRead(brightnessDial), 0, 1023, 1, 500);
          }
          Serial.print("BlueFade =  ");
          Serial.print(mySet.BlueFade);
          Serial.print(" BlueRand =  ");
          Serial.println(mySet.BlueRand);
          break;
        
        case 1: //blueWithGlitter", 
          mySet.BlueGlitter = map(analogRead(brightnessDial), 0, 1023, 1, 200);
          Serial.print(" BlueGlitter =  ");
          Serial.println(mySet.BlueGlitter);
          break;
        
        case 2: //oneColor
          if (funcButton.fell())
          {
            updateSettingsTime = millis();

            isFade = !isFade;
          }
          if (isFade)
          {
            mySet.ColfadeVal = map(analogRead(brightnessDial), 0, 1023, 1, 40);
          }
          else
          {
            mySet.ColrandVal = map(analogRead(brightnessDial), 0, 1023, 1, 500);
          }
          Serial.print("fadeVal =  ");
          Serial.print(mySet.ColfadeVal);
          Serial.print(" randVal =  ");
          Serial.println(mySet.ColrandVal);
          break;
        
        case 3: //myrainbow
          if (funcButton.fell())
          {
            updateSettingsTime = millis();
            isFade = !isFade;
          }
          if (isFade)
          {
            mySet.RainbowSat = map(analogRead(brightnessDial), 0, 1023, 1, 255);
          }
          else
          {
            mySet.RainbowVal = map(analogRead(brightnessDial), 0, 1023, 1, 255);
          }
          Serial.print("RainbowSat =  ");
          Serial.print(mySet.RainbowSat);
          Serial.print(" RainbowVal =  ");
          Serial.println(mySet.RainbowVal);
          break;
        
        case 4: //myrainbowWithGlitter
          mySet.RainbowGlitter = map(analogRead(brightnessDial), 0, 1023, 1, 200);
          Serial.print(" RainbowGlitter =  ");
          Serial.println(mySet.RainbowGlitter);

          break;
        
        case 5: //confetti
          break;
        
        case 6: //blueRedLights
          if (funcButton.fell())
          {
            updateSettingsTime = millis();
            isFade = !isFade;
          }
          if (isFade)
          {
            mySet.BlueRedFade = map(analogRead(brightnessDial), 0, 1023, 1, 50);
          }
          else
          {
            mySet.BlueRedRand = map(analogRead(brightnessDial), 0, 1023, 1, 500);
          }
          Serial.print("BlueRedFade =  ");
          Serial.print(mySet.BlueRedFade);
          Serial.print(" BlueRand =  ");
          Serial.println(mySet.BlueRedRand);
          break;
          
        case 7: //justGlitter
          if (funcButton.fell())
          {
            updateSettingsTime = millis();
            isFade = !isFade;
          }
          if (isFade)
          {
            mySet.JustGlitterFade = map(analogRead(brightnessDial), 0, 1023, 1, 50);
          }
          else
          {
            mySet.JustGlitterRand = map(analogRead(brightnessDial), 0, 1023, 1, 500);
          }
          Serial.print("JustGlitterFade =  ");
          Serial.print(mySet.JustGlitterFade);
          Serial.print(" JustGlitterRand =  ");
          Serial.println(mySet.JustGlitterRand);
          break;
        
        case 8: //rainbow
          break;
        
        case 9: //rotateAllModes
          break;
        
        default:
          break; //SHOULD NEVER HIT THIS!
      }
    
    gModes[gCurrentMode](); //Call the mode to update the display of lights as settings are changing
    prevAnalogVal = analogVal;
  } //Loop here while updateing settings - NOTE - LEDS wont display unless explicitly called 
  if (funcButton.fell()){
    updateSettingsTime = millis(); //Enter update mode
  }
}

void nextMode()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentMode = (gCurrentMode + 1) % ARRAY_SIZE(gModes);
  Serial.print("Mode = ");
  Serial.print(gCurrentMode);
  Serial.print(" - ");
  Serial.println(Modes[gCurrentMode]);
  updateModeTime = millis();
  for (int actLed = 0; actLed < NUM_SETTINGS_LEDS; actLed = actLed + 1)
  {
    // Turn our current led on to white, then show the leds
    allTreeLEDS[actLed] = CRGB::Black;
  }
  FlashLED();
  allTreeLEDS[gCurrentMode] = CRGB::Red;
  FastLED.show();
  delay(500);
}

unsigned int delaySpeed = 100;
int whiteLed = 0;
/* void MoveWhiteLed()
{
    if ((millis() - lastLEDTime) > delaySpeed)
    {
    //  settingsLEDS[whiteLed] = CRGB::Black;
      if (whiteLed == NUM_SETTINGS_LEDS - 1) {
        whiteLed = 0;
        }
      else  {
        whiteLed++;
      }
     // settingsLEDS[whiteLed] = CRGB::Violet;
      FastLED.show();
      lastLEDTime = millis();
    }
 
}

 */

int ledState = HIGH;              // the current state of the output pin
byte blinkCount = 0;
int lastLEDState = LOW;           // the current state of the LED Light
unsigned long lastLEDTime = 0;    // the last time the LED was toggled

void FlashLED()
{
  unsigned long currentMillis = millis();
 
  if(currentMillis - lastLEDTime > 200) {
    // save the last time you blinked the LED 
    lastLEDTime = currentMillis;   
 
    if (blinkCount <= gCurrentMode)
    // if the LED is off turn it on and vice-versa:
      if (ledState == LOW)
      {
       ledState = HIGH;
       blinkCount++;
     }
     else
       ledState = LOW;
   else
   {
      ledState = LOW;
      blinkCount++;
      if (blinkCount > 15)
        blinkCount = 0;
   }
    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }}

void myrainbow()
{
  myfill_rainbow(allTreeLEDS, NUM_TREE_LEDS, gHue, 1);
}

void rainbow()
{
  fill_rainbow(allTreeLEDS, NUM_TREE_LEDS, gHue, 1);
}

void myrainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  myrainbow();
  glitter(mySet.RainbowGlitter, false);
}


void myfill_rainbow( struct CRGB * pFirstLED, int numToFill,
                  uint8_t initialhue,
                  uint8_t deltahue )
{
    CHSV hsv;
    hsv.hue = initialhue;
    hsv.val = mySet.RainbowVal;
    hsv.sat = mySet.RainbowSat;

    for( int i = 0; i < numToFill; i++) {
        pFirstLED[i] = hsv;
        hsv.hue += deltahue;
    }
}

int maxGlitter = 2;

void addGlitter(fract8 chanceOfGlitter, CRGB * liteArray, int ledCount, bool blackOut )
{
  if (random8() < chanceOfGlitter)
  {
    uint16_t ledNbr = random16(ledCount);
    if (blackOut == true) {
      glitterQ.push(ledNbr);
      if (glitterQ.count()>maxGlitter)
      {
        liteArray[glitterQ.pop()] = CRGB::Black;
      }
    }   
    liteArray[ledNbr] += CRGB::White;
  }
}

void glitter(fract8 chanceOfGlitter, bool blackOut )
{
  addGlitter(chanceOfGlitter, allTreeLEDS, NUM_TREE_LEDS, blackOut);
}


void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeNow(10);

  randomConfetti(allTreeLEDS, NUM_TREE_LEDS);
}

void randomConfetti(CRGB * liteArray, int ledCount)
{
  if (random8(5) == 1) {
    int pos = random16(ledCount);
    liteArray[pos] += CHSV(gHue + random8(64), 200, 255);
  }
}

void oneColor()
{
  fadeNow(mySet.ColfadeVal);
  randomColor(allTreeLEDS, NUM_TREE_LEDS, mySet.ColrandVal);
}

void randomColor(CRGB * liteArray, int ledCount, int howRandom)
{
    randomSetColor(liteArray, ledCount,  howRandom, CHSV(gHue + random8(64), 200, 255));
}

void randomSetColor(CRGB * liteArray, int ledCount, int howRandom, CRGB aColor)
{
  if (random8() < howRandom)
  {
    int pos = random16(ledCount);
    liteArray[pos] += aColor;
    liteArray[pos].fadeLightBy(random8(20));
  }
}

void fadeNow(int fadeBy)
{
  fadeToBlackBy(allTreeLEDS, NUM_TREE_LEDS, fadeBy);
}


void blueLights()
{
  fadeNow(mySet.BlueFade);
  randomSetColor(allTreeLEDS, NUM_TREE_LEDS, mySet.BlueRand, aRandomBlue());
}

void blueWithGlitter()
{
  blueLights();
  glitter(mySet.BlueGlitter, true);
}


void blueRedLights()
{
  fadeNow(mySet.BlueRedFade);
  randomSetColor(allTreeLEDS, NUM_TREE_LEDS, mySet.BlueRedRand, aRandomBlueRed());
}


CRGB aRandomBlue()
{
   return ColorFromPalette( OceanColors_p, random8(15) );
}

extern const TProgmemRGBPalette32 BlueRedColors_p FL_PROGMEM =
{
  CRGB::MidnightBlue,
  CRGB::DarkBlue,
  CRGB::MidnightBlue,
  CRGB::Navy,

  CRGB::Red,
  CRGB::DarkRed,
  CRGB::Maroon,
  CRGB::DarkRed,
   
  CRGB::DarkBlue,
  CRGB::MediumBlue,
  CRGB::SeaGreen,
  CRGB::Teal,
   
  CRGB::CadetBlue,
  CRGB::Blue,
  CRGB::DarkCyan,
  CRGB::CornflowerBlue,
   
  CRGB::Aquamarine,
  CRGB::SeaGreen,
  CRGB::Aqua,
  CRGB::LightSkyBlue,

  CRGB::DarkRed,
  CRGB::DarkRed,
  CRGB::Red,
  CRGB::Purple,

  CRGB::MidnightBlue,
  CRGB::DarkBlue,
  CRGB::MidnightBlue,
  CRGB::Navy,

  CRGB::Red,
  CRGB::DarkRed,
  CRGB::Maroon,
  CRGB::DarkRed
};

CRGB aRandomBlueRed()
{
   return ColorFromPalette( BlueRedColors_p, random8(31) );
}

void justGlitter()
{
  fadeNow(mySet.JustGlitterFade);
  glitter(mySet.JustGlitterRand, false);
}

uint8_t gTempCurrentMode = 0; // Index number of which mode is current
unsigned long tempModeTime = 0; // Used to time out MODE changing
unsigned long tempModeLength = 120000; //How long to let each mode run for

void rotateAllModes()
{
  if (tempModeTime + tempModeLength < millis()) {
    //Time to change modes - but lets skip the full rainbow and the all modes, of course!
    gTempCurrentMode = (gTempCurrentMode + 1) % (ARRAY_SIZE(gModes) - 2);
    Serial.print("Temp Mode = ");
    Serial.print(gTempCurrentMode);
    Serial.print(" - ");
    Serial.println(Modes[gTempCurrentMode]);
    tempModeTime = millis();
  }
  
  //run the temp mode pattern 
  gModes[gTempCurrentMode](); 
}