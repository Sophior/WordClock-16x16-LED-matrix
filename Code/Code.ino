#include <Arduino.h>
#include <iostream>
#include <vector>
#include <FastLED.h>
#include <WiFi.h>

//  WiFi.begin("Wokwi-GUEST", "", 6);
const char* ssid     = "Wokwi-GUEST";
const char* password  = "";

#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     1
#define UTC_OFFSET_DST 2


#define NUM_LEDS 256
#define LED_TYPE WS2812B
#define COLOR_ORDER RGB
#define DATA_PIN 13 // define pin to connect DIN

CRGB leds[NUM_LEDS];
uint8_t ledBrightness = 100;


// put function declarations here:
void showWord(int index);
void setPixelColor(int i, uint32_t c0);
uint8_t getRedValueFromColor(uint32_t c);
uint8_t getGreenValueFromColor(uint32_t c);
uint8_t getBlueValueFromColor(uint32_t c);
uint32_t Color(byte r, byte g, byte b);

void showClock();
void getTimeViaWifi();
void printLocalTime();

uint8_t _whipeColor = Color(255,0,0);
uint8_t _backColor = Color(0,0,0);
uint8_t _frontColor = Color(0,255,0);

String timezone = "CET-1CEST,M3.5.0,M10.5.0/3";
void setTimezone(String timezone);
void initTime(String timezone);

// We adress per string section instead of specific led. Because we have 2 leds for 1 letter
int words[27][7]  = {
{0,1,2}, // het
{4,5}, // is
{12,13,14,15}, // vijf
{21,22,23,24,25}, // kwart
{16,17,18,19}, // tien
{27,28,29,30}, // over
{44,45,46,47}, // voor
{49, 50, 51, 52}, // half
{108,109, 110}, // uur
{58, 59, 60},// EEN
{97,98, 99, 100},// TWEE
{88, 89, 90, 91},// DRIE
{102,103,104, 105},// VIER
{92, 93, 94, 95},// VIJF
{61, 61,61},// ZES
{74,75,76,77,78},// ZEVEn
{54,55,56,57},// ACHT
{80,81,82,83,84},// NEGEN
{64,65,66,67},// TIEN
{85,86,87},// ELF (uren)
{68,69,70,71,72,73},// TWAALF
{113},// PLUS
{115}, //1
{116}, //2
{117}, //3
{118}, //4
{120,121,122,123,124,125,126} // minuten
};
String _gridLayout = "HETPISAHALFKVIJFTIENFKWARTGOVERMPWIFIZBTIJDJVOORUHALFBACHTEENZESTIENTWAALFZEVENTNEGENELFDRIEVIJFUTWEERVIERVTUURBJ+Z1234SMINUTENX";

void setup() {
  Serial.begin(115200);
  Serial.println("Booted up");

  getTimeViaWifi();
  // LED Init
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setDither(0);
  FastLED.setBrightness(ledBrightness);
  FastLED.show();

}
int lastWord = 0;
void loop() {

   // testLCD();

showClock();
//  showWord(lastWord);
//  lastWord += 1;
  delay(10000);
}

int iLastMinute = 0;
int iLastHour = 0;
int iMinute = 0;
int iHour = 0;
void showClock()
{
  printLocalTime();
  if(iLastMinute != iMinute || iLastHour != iHour)
  {
    for(int r=0;r<16;r++)
    {
      for(int c=0;c<16;c++)
      {
          setPixelColor((r*16)+c, Color(255,0,0));
          if(c>0)
          {
            setPixelColor((r*16)+c-1, Color(0,0,0)); // previous one
          }
      }
      setPixelColor((r*16)+15, Color(0,0,0)); // last in row
    }
  
  // divide minute by 5 to get value for display control
  int minDiv = iMinute / 5;


    showWord(0); // HET
    showWord(1); // IS
   
    // VIJF: (Minuten) x:05, x:25, x:35, x:55
    if ((minDiv == 1) || (minDiv == 5) || (minDiv == 7) || (minDiv == 11)) {
      //setLED(104, 107, 1);
      showWord(2);
    }
    // KWART: x:15, x:45
    if ((minDiv == 3) || (minDiv == 9)) {
     // setLED(51, 55, 1);
       showWord(3);
   
    }
    // TIEN: (Minuten) x:10, x:50
    if ((minDiv == 2) || (minDiv == 10)) {
     // setLED(108, 111, 1);
       showWord(4);
   
    }
    // TIEN: (TIEN VOOR HALF, TIEN OVER HALF) x:20, x:40 (on request not set to TWINTIG OVER)
    if ((minDiv == 4) || (minDiv == 8)) {
      //setLED(108, 111, 1);
       showWord(4);
    }
    // OVER: x:05, x:10, x:15, x:35, x:40
    if ((minDiv == 1) || (minDiv == 2) || (minDiv == 3) || (minDiv == 7) || (minDiv == 8)) {
      // setLED(140, 143, 1);
      showWord(5);
    }
    // VOOR: x:20, x:25, x:45, x:50, x:55
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 9) || (minDiv == 10) || (minDiv == 11)) {
      //setLED(135, 138, 1);
      showWord(6);
    }
    // HALF:
    if ((minDiv == 4) || (minDiv == 5) || (minDiv == 6) || (minDiv == 7) || (minDiv == 8)) {
      //setLED(130, 133, 1);
      showWord(7);
    }


    //set hour from 1 to 12 (at noon, or midnight)
    int xHour = (iHour % 12);
    if (xHour == 0)
      xHour = 12;
    // at minute 20 hour needs to be counted up:
    // tien voor half 2 = 13:20
    if (iMinute >= 20) {
      if (xHour == 12)
        xHour = 1;
      else
        xHour++;
    }


    showWord(xHour+8);

    if(minDiv>0 && minDiv<5)
    {
    showWord(21); // plus
    showWord(21+minDiv);
    showWord(26); // minuten
    }
    else if( minDiv == 0)
    {
    showWord(8); // "UUr"
    }

    FastLED.show();
  }

  iLastMinute = iMinute;
  iLastHour = iHour;


}
void getTimeViaWifi()
{
   WiFi.begin(ssid, password, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");
  initTime(timezone);
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}
void initTime(String timezone)
{
  struct tm timeinfo;
  Serial.println("Setting up time");
  configTime(0, 0, "pool.ntp.org");    // First connect to NTP server, with 0 TZ offset
  if(!getLocalTime(&timeinfo)){
    Serial.println("  Failed to obtain time");
    return;
  }
  Serial.println("  Got the time from NTP");
  // Now we can set the real timezone
  setTimezone(timezone);
}
void setTimezone(String timezone){
  Serial.printf("  Setting Timezone to %s\n",timezone.c_str());
  setenv("TZ",timezone.c_str(),1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}

void printLocalTime(){
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Connection Err");
    return;
  }
  iMinute = timeinfo.tm_min;
  iHour = timeinfo.tm_hour;

  Serial.println(&timeinfo, "%H:%M:%S");
  Serial.println(&timeinfo, "%d/%m/%Y   %Z");
}

void showWord(int index)
{

  int rows = sizeof(words) / sizeof(words[0]); // 2 rows  
  int cols = sizeof(words[index]) / sizeof(int); // 5 cols
   std::vector<int> _activeLettersFromWords;
    for(int w=0;w<cols;w++)
    {
      if(w<=0 || words[index][w] != 0)
      {
        // Serial.print(words[r][w]);
        _activeLettersFromWords.push_back(words[index][w]);
      }
    }
    Serial.println("Leds retrieved");
    /*
    for (int a : _activeLeds)
    {
        Serial.print(a);
    }
    Serial.println("-------------");*/
  //}
int _wordmodifier = 0;
for(int x=0;x<16;x++)
{
  if( x>0&&x%2==0) 
  {
	  _wordmodifier = floor(x/2)*16;
  	if(_wordmodifier%2!=0)
	  {
  	  _wordmodifier-=1;
	  }
  }
  Serial.print("Modifier ");
  Serial.println(_wordmodifier);
  
	for(int y=0;y<16;y++)
	{
    Serial.print(_wordmodifier+y);

    for(int wordletter : _activeLettersFromWords)
    {
      if(wordletter == _wordmodifier+y)
      {
     		Serial.print( _gridLayout[(_wordmodifier + y )]);
         Serial.print( " on " );
         Serial.print(((x*16) + y )); // LED
if(x%2!=0) // skip the extra row
{       setPixelColor(((x*16) + y), Color(0,0,255));
}
      
       
         Serial.println();
      }
      /*
      if(wordletter == ((_wordmodifier*x) + y))
      {
        Serial.print("Modified ");
     		Serial.print( _gridLayout[(_wordmodifier + y )]);
         Serial.print( " on " );
         Serial.print(((_wordmodifier*x) + y)); // LED
         setPixelColor(((_wordmodifier*x) + y), Color(255,0,0));
         Serial.println();
      }*/
       // Serial.println("Letter:" + _letter);
//          Serial.println("LED:");
  //        Serial.println((x*16) + y );

      //Serial.println(_wordmodifier);
	    }

    }
  }
}

void setPixelColor(int i, uint32_t c0)
{
    leds[i].setRGB(getRedValueFromColor(c0), getBlueValueFromColor(c0), getGreenValueFromColor(c0));
    
  Serial.print("Activate led ");
  Serial.print(i);
  Serial.println("");
  FastLED.show();
}
uint8_t getRedValueFromColor(uint32_t c) {
    return c >> 16;
}

uint8_t getGreenValueFromColor(uint32_t c) {
    return c >> 8;
}

uint8_t getBlueValueFromColor(uint32_t c) {
    return c;
}
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}
