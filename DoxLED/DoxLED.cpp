#include "DoxLED.h" //include the declaration for this class

#include <FastLED.h> 

//===============================
const int LED_NUMBER_OF_LEDS = 12;
//===============================
 
const int LED_PIN = 15;
const int LED_DEFAULT_BRIGHTNESS = 5;
bool LED_GROUPFIND_ON = false;

CRGB leds[LED_NUMBER_OF_LEDS];
 
//<<constructor>> setup the LED, make pin 13 an OUTPUT
DoxLED::DoxLED(){
	FastLED.addLeds<NEOPIXEL, 15 >(leds, LED_NUMBER_OF_LEDS);
  	FastLED.setBrightness(LED_DEFAULT_BRIGHTNESS);	
}
 
//<<destructor>>
DoxLED::~DoxLED(){/*nothing to destruct*/}

void DoxLED::LED_Clear(){
  for (int led=0; led!=LED_NUMBER_OF_LEDS; led++){
    leds[led] = CRGB::Black;
  }
  FastLED.show();
}

void DoxLED::LED_Center_Blue(bool ON){
  LED_Clear();
  if (ON) {
    leds[0] = CRGB::Blue;
    FastLED.show();    
  }else{
    LED_Clear();  
  }
}

void DoxLED::LED_FIND_LIGHT_FOR_DOx(){
  fill_solid(leds, LED_NUMBER_OF_LEDS, CRGB(255,  255,  255));   //white
  delay(40);
  FastLED.show();
  fill_solid(leds, LED_NUMBER_OF_LEDS, CRGB(0,  0,  0));   //white
  delay(40);
  FastLED.show();  
}

void DoxLED::LED_sensor_return_code_Fade(int code){
  LED_Clear();
  FastLED.setBrightness(30);
      switch (code){
        case 1:   fill_solid(leds, LED_NUMBER_OF_LEDS, CRGB(0,    0,    255)); break;   //blue
        case 2:   fill_solid(leds, LED_NUMBER_OF_LEDS, CRGB(255,  0,      0)); break;   //red
        case 254: fill_solid(leds, LED_NUMBER_OF_LEDS, CRGB(255,  255,    0)); break;   //yellow
        case 255: fill_solid(leds, LED_NUMBER_OF_LEDS, CRGB(255,  255,  255)); break;   //white
      }
      for (int i=0; i<100; i++){
        delay(15);
        for (int j=0; j<7; j++){
          leds[j].fadeToBlackBy ( i );   
        }
        FastLED.show();
      }    
}

void DoxLED::LED_Alert(char TooWhat){
  int color;
  if(TooWhat=='H') color=50;
  else color=160;
  for(int v=0; v<15; v++){
     fill_solid(leds, LED_NUMBER_OF_LEDS, CRGB(255,  255,    0));          //yellow
     FastLED.show();
     delay(30);
     LED_Clear();
     delay(30);
     }
  LED_Clear();  
}

void DoxLED::LED_smoov(int L1, int L2, int color){
  for (int ww=0; ww<=255; ww=ww+(LED_NUMBER_OF_LEDS/7)){
    if (!LED_GROUPFIND_ON) ww=255+1;
    leds[L1]=CHSV(color,255,255-ww);
    leds[L2]=CHSV(color,255,ww);
    FastLED.show();
    //delay(5);
  }
}

void DoxLED::LED_Show_Group_Find_Color(int color){
  for (int ci=1; ci<=LED_NUMBER_OF_LEDS; ci++){
    int LEDTopIndex = LED_NUMBER_OF_LEDS-1;
    for(int cii=1; cii <= LEDTopIndex; cii++){
      int L1=cii;
      int L2=cii+1;
      if (L2 == LEDTopIndex+1) L2=1;
      if (!LED_GROUPFIND_ON) cii=LEDTopIndex+1;
      LED_smoov(L1,L2,color);
    }
  }
}