#include <M5Stack.h>
#include "ESP32_WebGet.h" //beta ver 1.12-
#include "TimeLib.h" //Use Arduino time library ver1.5-

 //--------NTP関連定義---------------
const int TIME_ZONE = 9; //Tokyo
const char *NTP_SERVER_NAME = "time.windows.com";
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov

void NTP_Get(uint32_t get_interval);

ESP32_WebGet _EWG;
 
int32_t LastTime = 0;

int8_t prev_hour = -1;
int8_t now_hour = -1;
int8_t prev_min = -1;
int8_t now_min = -1;
int8_t prev_sec = -1;
int8_t now_sec = -1;
byte xcolon = 0, xsecs = 0;

//-----NTPサーバー時刻取得　引数初期化------------
uint32_t NTPGetLastTime = 0;
bool NTP_first_get = true;
 
//***********セットアップ****************************
void setup() {
  delay(1000);
  m5.begin();
  Serial.begin(115200);

  //--------Wi-Fiアクセスポイント接続---------
  _EWG.EWG_AP_Connect("Buffalo-G-620E", "caix4x4iufnhx"); //Wi-Fi ルーターと接続
  delay(1000);
  _EWG.EWG_NTP_TimeLib_init(9, "time.windows.com"); //NTPサーバー取得初期化
  _EWG.NTP_OtherServerSelect(9); //NTPサーバーと接続できなかった場合、他のNTPサーバーと接続できるか試す関数

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK); 
  //Font_Setup(); //フォントセットアップ
}
//***********メインループ****************************
void loop() {
  NTP_Get(600000); //10分毎に時刻取得補正
 
  if( LastTime != now() ){
    now_hour = hour();
    now_min = minute();
    now_sec = second();

    // Update digital time
    int xpos = 0;
    int ypos = 85; // Top left corner ot clock text, about half way down
    int ysecs = ypos + 24;

    if (prev_min != now_min) { // Redraw hours and minutes time every minute
      prev_hour = now_hour;
      // Draw hours and minutes
      if (now_hour < 10) xpos += M5.Lcd.drawChar('0', xpos, ypos, 8); // Add hours leading zero for 24 hr clock
      xpos += M5.Lcd.drawNumber(now_hour, xpos, ypos, 8);             // Draw hours
      xcolon = xpos; // Save colon coord for later to flash on/off later
      xpos += M5.Lcd.drawChar(':', xpos, ypos - 8, 8);
      if (now_min < 10) xpos += M5.Lcd.drawChar('0', xpos, ypos, 8); // Add minutes leading zero
      xpos += M5.Lcd.drawNumber(now_min, xpos, ypos, 8);             // Draw minutes
      xsecs = xpos; // Sae seconds 'x' position for later display updates
    }
    if (prev_sec != now_sec) { // Redraw seconds time every second
      prev_min = now_min;
      xpos = xsecs;

      if (now_min % 2) { // Flash the colons on/off
        M5.Lcd.setTextColor(0x39C4, TFT_BLACK);        // Set colour to grey to dim colon
        M5.Lcd.drawChar(':', xcolon, ypos - 8, 8);     // Hour:minute colon
        xpos += M5.Lcd.drawChar(':', xsecs, ysecs, 6); // Seconds colon
        M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);    // Set colour back to yellow
      }
      else {
        M5.Lcd.drawChar(':', xcolon, ypos - 8, 8);     // Hour:minute colon
        xpos += M5.Lcd.drawChar(':', xsecs, ysecs, 6); // Seconds colon
      }

      //Draw seconds
      if (now_sec < 10) xpos += M5.Lcd.drawChar('0', xpos, ysecs, 6); // Add leading zero
      M5.Lcd.drawNumber(now_sec, xpos, ysecs, 6);                     // Draw seconds
    }

    LastTime = now();
  }
}

//*************************************
void NTP_Get(uint32_t get_interval){
  if( (NTP_first_get == true) || ((millis() - NTPGetLastTime) > get_interval) ){
    setTime(EWG_Get_Ntp_Time());
    //Message(" NTP Getting Wait");
    NTP_first_get = false;
    //Message( " NTP Get OK! " + String( hour() ) + ":" + String( minute() ) );
    NTPGetLastTime = millis();
  }
}
