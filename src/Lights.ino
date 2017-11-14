// #include <bitswap.h>
// #include <chipsets.h>
// #include <color.h>
// #include <colorpalettes.h>
// #include <colorutils.h>
// #include <controller.h>
// #include <cpp_compat.h>
// #include <dmx.h>
#include <FastLED.h>
// #include <fastled_config.h>
// #include <fastled_delay.h>
// #include <fastled_progmem.h>
// #include <fastpin.h>
// #include <fastspi.h>
// #include <fastspi_bitbang.h>
// #include <fastspi_dma.h>
// #include <fastspi_nop.h>
// #include <fastspi_ref.h>
// #include <fastspi_types.h>
// #include <hsv2rgb.h>
// #include <led_sysdefs.h>
// #include <lib8tion.h>
// #include <noise.h>
// #include <pixelset.h>
// #include <pixeltypes.h>
// #include <platforms.h>
// #include <power_mgt.h>

#include <Bounce2.h>

#define MODE_PIN 2
#define FUNC_PIN 3
int brightnessDial = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED


int sensorValue = 0;  // variable to store the value coming from the sensor

int ledState = HIGH;         // the current state of the output pin
bool ledsOn = false;

int lastLEDState = LOW;         // the current state of the LED Light
unsigned long lastLEDTime = 0;  // the last time the LED was toggled

bool updateMode = false;
unsigned long updateModeTime = 0;  // Used to time out MODE changing


#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  12

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

#define NUM_SETTINGS_LEDS 12
#define SETTINGS_LEDS_PIN 4

// This is an array of leds.  One item for each led in your strip.
CRGB settingsLEDS[NUM_SETTINGS_LEDS];

// Instantiate a Bounce object
Bounce modeButton = Bounce();
Bounce funcButton = Bounce();

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))



void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  pinMode(MODE_PIN, INPUT_PULLUP);
  pinMode(FUNC_PIN, INPUT_PULLUP);

  modeButton.attach(MODE_PIN);
  modeButton.interval(15); // interval in ms

  funcButton.attach(FUNC_PIN);
  funcButton.interval(15); // interval in ms

  FastLED.addLeds<WS2811, SETTINGS_LEDS_PIN, RGB>(settingsLEDS, NUM_SETTINGS_LEDS);

  Serial.begin(9600);
  Serial.println("Initialised");


}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*ModeList[])();
ModeList gModes = {MoveWhiteLed, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, oneColor };
char* Modes[]  = {"MoveWhiteLed", "rainbow", "rainbowWithGlitter", "confetti", "sinelon", "juggle", "bpm", "oneColor" };

uint8_t gCurrentMode = 0; // Index number of which mode is current



void loop() {
  // put your main code here, to run repeatedly:
  modeButton.update();
  funcButton.update();

  // Call the current pattern function once, updating the 'leds' array
  gModes[gCurrentMode]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  //  int modeValue = modeButton.read();
  //  int funcValue = funcButton.read();
  if ( modeButton.fell() ) {
    ledsOn = !ledsOn;
    nextMode();
    Serial.println("Button Pressed");
  }
  //  FlashLED();
  //  MoveWhiteLed();
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }
}

void ModeChange()
{

}

void nextMode()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentMode = (gCurrentMode + 1) % ARRAY_SIZE( gModes);
  Serial.print("Mode = ");
  Serial.print(gCurrentMode);
  Serial.print(" - " );
  Serial.println (Modes[gCurrentMode]);
  updateModeTime = millis();
  for (int actLed = 0; actLed < NUM_SETTINGS_LEDS; actLed = actLed + 1) {
    // Turn our current led on to white, then show the leds
    settingsLEDS[actLed] = CRGB::Black;
  }
    settingsLEDS[gCurrentMode ] = CRGB::Red;
   FastLED.show();
   delay(2000);
  

}

int delaySpeed = 100;
void MoveWhiteLed()
{
  // Move a single white led
  for (int whiteLed = 0; whiteLed < NUM_SETTINGS_LEDS; whiteLed = whiteLed + 1) {
    // Turn our current led on to white, then show the leds
    settingsLEDS[whiteLed] = CRGB::Violet;

    // Show the leds (only one of which is set to white, from above)
    FastLED.show();

    FastLED.delay(delaySpeed);

    // Wait a little bit
//    delay(delaySpeed);

    // Turn our current led back to black for the next loop around
    settingsLEDS[whiteLed] = CRGB::Black;
  }

}

void FlashLED()
{
  if (ledsOn == true)  {
    // read the value from the sensor:
    sensorValue = analogRead(brightnessDial);
    //Serial.print(" sensorValue = ");
    //Serial.println (sensorValue );
    if ((millis() - lastLEDTime) > sensorValue) {
      //Flash led
      //Store lastLed
      lastLEDState = !lastLEDState;
      digitalWrite(ledPin, lastLEDState);
      lastLEDTime = millis();
    }
  }
}


void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( settingsLEDS, NUM_SETTINGS_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    settingsLEDS[ random16(NUM_SETTINGS_LEDS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( settingsLEDS, NUM_SETTINGS_LEDS, 10);
  int pos = random16(NUM_SETTINGS_LEDS);
  settingsLEDS[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( settingsLEDS, NUM_SETTINGS_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_SETTINGS_LEDS - 1 );
  settingsLEDS[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_SETTINGS_LEDS; i++) { //9948
    settingsLEDS[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( settingsLEDS, NUM_SETTINGS_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    settingsLEDS[beatsin16( i + 7, 0, NUM_SETTINGS_LEDS - 1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

bool isFade = false;
int fadeVal = 25;
int randVal = 40;
void oneColor()
{
//  funcButton.update();
  fadeToBlackBy( settingsLEDS, NUM_SETTINGS_LEDS, fadeVal);
  if ( random8() < randVal) {
    int pos = random16(NUM_SETTINGS_LEDS);
    settingsLEDS[pos] +=  CHSV( gHue + random8(64), 200, 255);
  }
  
  if ( funcButton.fell() ) {
    Serial.println("Func pressed");
    isFade = !isFade;
  }
    if (isFade) {
      fadeVal = map(analogRead(brightnessDial), 0, 1023, 1, 40);
    } else {
      randVal = map(analogRead(brightnessDial), 0, 1023, 1, 100);
    }
    Serial.print("fadeVal =  ");
    Serial.print(fadeVal);
    Serial.print(" randVal =  ");
    Serial.println(randVal);

}

