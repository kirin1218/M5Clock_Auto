#include <M5Stack.h>
#include "ESP32_WebGet.h" //beta ver 1.12-
#include "TimeLib.h" //Use Arduino time library ver1.5-
//--------WIFI関連----------------
bool WiFi_Connect(int retry = -1, int timeout = 5000);
void WiFi_DisConnect();

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
int8_t prev_month = -1;
int8_t now_month = -1;
int8_t prev_day = -1;
int8_t now_day = -1;
byte xcolon = 0, xsecs = 0;

//-----NTPサーバー時刻取得　引数初期化------------
uint32_t NTPGetLastTime = 0;
bool NTP_first_get = true;
 
//***********セットアップ****************************
void setup() {
  //delay(1000);
  m5.begin();
  Serial.begin(115200);

  //--------Wi-Fiアクセスポイント接続---------
  WiFi_Connect();
  
  //_EWG.EWG_AP_Connect("Buffalo-G-620E", "caix4x4iufnhx"); //Wi-Fi ルーターと接続
  _EWG.EWG_NTP_TimeLib_init(TIME_ZONE, NTP_SERVER_NAME); //NTPサーバー取得初期化
  _EWG.NTP_OtherServerSelect(9); //NTPサーバーと接続できなかった場合、他のNTPサーバーと接続できるか試す関数

  WiFi_DisConnect();

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK); 
  M5.Lcd.drawLine(0,120,320,120,TFT_WHITE);
  //Font_Setup(); //フォントセットアップ
}
//***********メインループ****************************
void loop() {
  //NTP_Get(600000); //10分毎に時刻取得補正
 
  if( LastTime != now() ){
    now_month = month();
    now_day = day();
    now_hour = hour();
    now_min = minute();
    now_sec = second();

    int x_day = 30;
    int xpos = 100;
    int ypos = 0; // Top left corner ot clock text, about half way down

    M5.Lcd.setTextSize(1);
    if (prev_day != now_day) { // Redraw hours and minutes time every minute
      prev_day = now_day;
      if (prev_month != now_month) { // Redraw hours and minutes time every minute
        xpos = x_day;
        ypos = 30;
        prev_month = now_month;
        if (now_month < 10) xpos += M5.Lcd.drawChar(' ', xpos, ypos, 8); // Add hours leading zero for 24 hr clock
        xpos += M5.Lcd.drawNumber(now_month, xpos, ypos, 8);             // Draw hours
      }
      xpos = x_day;
      ypos = 130;
      if (now_day < 10) xpos += M5.Lcd.drawChar(' ', xpos, ypos, 8); // Add hours leading zero for 24 hr clock
      xpos += M5.Lcd.drawNumber(now_day, xpos, ypos, 8);             // Draw hours
    }
    // Update digital time
    int x_time = 150;

    M5.Lcd.setTextSize(1);
    if (prev_min != now_min) { // Redraw hours and minutes time every minute
      prev_hour = now_hour;
      xpos = x_time;
      ypos = 30;
      if (now_hour < 10) xpos += M5.Lcd.drawChar('0', xpos, ypos, 8); // Add hours leading zero for 24 hr clock
      xpos += M5.Lcd.drawNumber(now_hour, xpos, ypos, 8);             // Draw hours
      xpos = x_time;
      ypos = 130;
      if (now_min < 10) xpos += M5.Lcd.drawChar('0', xpos, ypos, 8); // Add minutes leading zero
      xpos += M5.Lcd.drawNumber(now_min, xpos, ypos, 8);             // Draw minutes
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

bool connectWait(int timeout){
  int tick = millis();
  while( tick + timeout > millis() ){
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(10);
  }
  return false;
}
#include "Refs.h"
#include "CommonFuncs.h"
bool WiFi_Connect(int retry, int timeout){
  int retry_cnt = 0;
  char ssid_list[][32] = {IPHONE_SSID,HOME_SSID,HOME2_SSID};
  char pass_list[][32] = {IPHONE_PASS,HOME_PASS,HOME2_PASS};
  char dec_id[32];
  char dec_pass[32];

  do{
    int count = sizeof(ssid_list)/sizeof(ssid_list[0]);
    for( int i = 0; i < count; i++){
      DecString( ssid_list[i], dec_id, sizeof(ssid_list[i]));
      DecString( pass_list[i], dec_pass, sizeof(pass_list[i]));
      M5.Lcd.printf("connecting... id[%s]\r\n",dec_id);
      WiFi.begin(dec_id, dec_pass);
      connectWait(timeout);
      if( WiFi.status() == WL_CONNECTED ){
        M5.Lcd.printf("connected id[%s]\r\n",dec_id);
        goto CONNECT_LOOPOUT;
      }
    }
    // retry:-1 無限にリトライ
    if( retry != -1 ){
      if( retry <= retry_cnt){
        break;
      }
    }
    retry_cnt++;
  }while( WiFi.status() != WL_CONNECTED );
CONNECT_LOOPOUT:
  return (bool)(WiFi.status() == WL_CONNECTED);
}

void WiFi_DisConnect(){
  WiFi.disconnect();
}