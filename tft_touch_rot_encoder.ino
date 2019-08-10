/* Map XPT2046 input to ILI9341 320x240 raster */
#include "SPI.h"
#include "AiEsp32RotaryEncoder.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h> /* https://github.com/PaulStoffregen/XPT2046_Touchscreen */

#define TFT_DC              4
#define _sclk               18
#define _mosi               23
#define _miso               19
#define TFT_CS              15 
#define TFT_RST             2
#define TFT_BACKLIGHT_PIN   5 
#define TOUCH_CS_PIN        33 
#define TOUCH_IRQ_PIN       35

#define TFT_NORMAL          1
#define TFT_UPSIDEDOWN      3

#define ROTARY_ENCODER_A_PIN      27
#define ROTARY_ENCODER_B_PIN      14
#define ROTARY_ENCODER_BUTTON_PIN 13
#define ROTARY_ENCODER_VCC_PIN    (-1)

const uint8_t TFT_ORIENTATION = TFT_NORMAL;

Adafruit_ILI9341 tft = Adafruit_ILI9341( TFT_CS, TFT_DC, TFT_RST );
XPT2046_Touchscreen touch( TOUCH_CS_PIN, TOUCH_IRQ_PIN );
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN);

void rotary_onButtonClick() {
  tft.fillScreen(ILI9341_BLACK);
  yield();
  rotaryEncoder.reset(100);
  tft.setCursor(10, 10);
  tft.println(100);
}

void rotary_loop() {
  //first lets handle rotary encoder button click
  if (rotaryEncoder.currentButtonState() == BUT_RELEASED) {
    //we can process it here or call separate function like:
    rotary_onButtonClick();
  }

  //lets see if anything changed
  int16_t encoderDelta = rotaryEncoder.encoderChanged();
  
  //optionally we can ignore whenever there is no change
  if (encoderDelta == 0) return;
  
  //for some cases we only want to know if value is increased or decreased (typically for menu items)
  if (encoderDelta>0) Serial.print("+");
  if (encoderDelta<0) Serial.print("-");
 
  //if value is changed compared to our last read
  if (encoderDelta!=0) {
    //now we need current value
    int16_t encoderValue = rotaryEncoder.readEncoder();
    tft.setCursor(10, 10);
    tft.println(encoderValue);
  } 
  
}

void setup() {
  Serial.begin( 115200 );

  rotaryEncoder.begin();
  rotaryEncoder.setup([]{rotaryEncoder.readEncoder_ISR();});
  //optionally we can set boundaries and if values should cycle or not
  rotaryEncoder.setBoundaries(0, 1000, false); //minValue, maxValue, cycle values (when max go to min and vice versa)
  
  SPI.begin( _sclk, _miso, _mosi );
  SPI.setFrequency( 60000000 );

  tft.begin( 20000000, SPI );
  tft.setRotation( TFT_ORIENTATION );
  tft.setTextColor( ILI9341_GREEN, ILI9341_BLACK );

  pinMode( TFT_BACKLIGHT_PIN, OUTPUT );
  digitalWrite( TFT_BACKLIGHT_PIN, HIGH );

  tft.fillScreen(ILI9341_RED);
  yield();
  delay(500);
  tft.fillScreen(ILI9341_GREEN);
  yield();
  delay(500);
  tft.fillScreen(ILI9341_BLUE);
  yield();
  delay(500);
  tft.fillScreen(ILI9341_BLACK);
  yield();
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK); 
  tft.setTextSize(2);
  
  touch.begin();
  Serial.println( "Touch screen ready." );
  rotaryEncoder.enable();
}


TS_Point rawLocation;

void loop() {
  rotary_loop();
  while ( touch.touched() )
  {
    rawLocation = touch.getPoint();
    
    if ( TFT_ORIENTATION == TFT_UPSIDEDOWN )
    {
      tft.drawPixel( mapFloat( rawLocation.x, 340, 3900, 0, 320 ),
                     mapFloat( rawLocation.y, 200, 3850, 0, 240 ),
                     ILI9341_GREEN );
    }
    if ( TFT_ORIENTATION == TFT_NORMAL )
    {
      tft.drawPixel( mapFloat( rawLocation.x, 340, 3900, 320, 0 ),
                     mapFloat( rawLocation.y, 200, 3850, 240, 0 ),
                     ILI9341_GREEN );
    }
  }
}

static inline __attribute__((always_inline)) float mapFloat( float x, const float in_min, const float in_max, const float out_min, const float out_max)
{
  return ( x - in_min ) * ( out_max - out_min ) / ( in_max - in_min ) + out_min;
}
