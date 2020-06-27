#if defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS)


#include <Adafruit_ST7789.h>
#include "Arduino_APDS9960.h"

#define ARCADA_TFT_SPI SPI
#define ARCADA_TFT_CS 	A2  // Display CS Arduino pin number
#define ARCADA_TFT_DC 	A3  // Display D/C Arduino pin number
#define ARCADA_TFT_RST 	A4 // Display reset Arduino pin number
#define ARCADA_TFT_LITE A5
#define ARCADA_TFT_ROTATION 0
#define ARCADA_TFT_DEFAULTFILL 0xFFFF
#define ARCADA_TFT_WIDTH 240
#define ARCADA_TFT_HEIGHT 240

#define ARCADA_SD_CS 10

//#define ARCADA_TOUCHOUT_PIN 48
//#define ARCADA_SPEAKER_ENABLE 49
//#define ARCADA_NEOPIXEL_PIN 8
//#define ARCADA_NEOPIXEL_NUM 4
#define ARCADA_AUDIO_OUT A0

//#define ARCADA_LIGHT_SENSOR A7
#define ARCADA_BATTERY_SENSOR A6

//#define ARCADA_BUTTON_SHIFTMASK_RIGHT 9
//#define ARCADA_BUTTON_SHIFTMASK_A 10

#define CPLAY_RIGHTBUTTON 9
//#define CPLAY_ABUTTON 10

#define ARCADA_CALLBACKTIMER 4
#define ARCADA_CALLBACKTIMER_HANDLER TC4_Handler

#define ARCADA_USE_JSON

#include "arcadatype.h"

class Adafruit_Arcada : public Adafruit_Arcada_SPITFT {
public:
  Adafruit_Arcada(void){};

  bool variantBegin(void) { 			//setup
	
    APDS.begin();
	pinMode(CPLAY_RIGHTBUTTON, INPUT);
  
	return true; 
  }
  /*citation [1] - START- */
  int buttonState;
  int prevState = LOW;
  long btDebounce    = 30;
  long btMultiClick  = 600;
  long btLongClick   = 500;
  long btLongerClick = 2000;
  long btTime = 0, btTime2 = 0;
  int clickCnt = 1;
  
  // 0=idle, 1,2,3=click, -1,-2=longclick
  int checkButton()
  {
    int state = digitalRead(CPLAY_RIGHTBUTTON);
    if( state == HIGH && prevState == LOW ) { btTime = millis(); prevState = state; return 0; } // button just pressed
    if( state == LOW && prevState == HIGH ) { // button just released
      prevState = state;
      if( millis()-btTime >= btDebounce && millis()-btTime < btLongClick ) { 
        if( millis()-btTime2<btMultiClick ) clickCnt++; else clickCnt=1;
        btTime2 = millis();
        return clickCnt; 
      } 
    }
    if( state == HIGH && millis()-btTime >= btLongerClick ) { prevState = state; return -2; }
    if( state == HIGH && millis()-btTime >= btLongClick ) { prevState = state; return -1; }
    return 0;
  }
  
  int prevButtonState=0;

  int handleButton()
  {
    prevButtonState = buttonState;
    buttonState = checkButton();
    return buttonState;
  }

  /*citation [1] -END- */

  uint32_t variantReadButtons(void) {	//loop	
    uint32_t buttons = 0;
	
	handleButton();
    if (buttonState > 0) {
		buttons |= ARCADA_BUTTONMASK_A;			//buttons |= ARCADA_BUTTONMASK_DOWN;
	}
	if (buttonState < 0) {
		buttons |= ARCADA_BUTTONMASK_B;			//buttons |= ARCADA_BUTTONMASK_A;
	}
	
	/*// APDS-9960 PROXIMITY SENSOR :: NEAR
	if (APDS.proximityAvailable()) {
		int proximity = APDS.readProximity();
		Serial.println(proximity);
		
		if (proximity == 0) {
			buttons |= ARCADA_BUTTONMASK_A;
			//Serial.println(proximity);
		} 
	}*/
	
	// APDS-9960 GESTURE SENSOR :: UP DOWN LEFT RIGHT
	if (APDS.gestureAvailable()) {
		int gesture = APDS.readGesture();
		
		switch (gesture) {
			
			case GESTURE_UP:
				buttons |= ARCADA_BUTTONMASK_UP;
				break;
				
			case GESTURE_DOWN:
				buttons |= ARCADA_BUTTONMASK_DOWN;
				break;
			
			case GESTURE_LEFT:
				buttons |= ARCADA_BUTTONMASK_LEFT;
				break;
			
			case GESTURE_RIGHT:
				buttons |= ARCADA_BUTTONMASK_RIGHT;
				break;
		}
	}
    return buttons;
  }

  void displayBegin(void) {
    Adafruit_ST7789 *tft = new Adafruit_ST7789(&ARCADA_TFT_SPI, ARCADA_TFT_CS,
                                               ARCADA_TFT_DC, ARCADA_TFT_RST);
    tft->init(240, 240);
    tft->setRotation(ARCADA_TFT_ROTATION);
    tft->fillScreen(ARCADA_TFT_DEFAULTFILL);
    uint8_t rtna =
        0x01; // Screen refresh rate control (datasheet 9.2.18, FRCTRL2)
    tft->sendCommand(0xC6, &rtna, 1);
    tft->setSPISpeed(50000000); // yes fast	
    display = tft;
  }
};

#endif

/* [1] https://github.com/cbm80amiga/ST7735_SDMediaPlayer/blob/master/ST7735_SDMediaPlayer.ino*/
