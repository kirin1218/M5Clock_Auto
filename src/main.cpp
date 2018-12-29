#if 0
#include <M5Stack.h>
#include <WiFi.h>
#include "TimeLib.h"
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include <ESPmDNS.h>
#include <WiFiClient.h>
#include "WebServer.h"
#include <Preferences.h>
//--------NTP関連定義---------------
const int TIME_ZONE = 9; //Tokyo
const char *NTP_SERVER_NAME = "time.windows.com";
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov
EthernetUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

void sendNTPpacket(IPAddress &address);
String urlDecode(String input);
boolean checkConnection();
void startWebServer();
void setupMode();
String makePage(String title, String contents);
boolean restoreConfig();

const IPAddress apIP(192, 168, 4, 1);
const char* apSSID = "M5STACK_SETUP";
boolean settingMode;
String ssidList;
String wifi_ssid;
String wifi_password;

String wifi_home_ssid1;
String wifi_home_ssid2;
String wifi_iphone_ssid;
String wifi_home_pass1;
String wifi_home_pass2;
String wifi_iphone_pass;
// DNSServer dnsServer;
WebServer webServer(80);

// wifi config store
Preferences preferences;

bool connectWait(int timeout){
  int tick = millis();
  while( tick + timeout > millis() ){
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(10);
  }
  M5.Lcd.print("timeout wifi\n");
  return false;
}

bool connectWifi(){
  do{
    WiFi.begin("yukirin_my_angel", "x2a8xrqcfhrm9");
    if( connectWait(5000)==false){
      WiFi.begin("Buffalo-A-620E", "caix4x4iufnhx");
      if( connectWait(5000)==false){
        WiFi.begin("Buffalo-G-620E", "caix4x4iufnhx");
        if( connectWait(5000)==false){
          WiFi.begin("yukirin_my_angel", "x2a8xrqcfhrm9");
          if( connectWait(5000)==false){
            ;
          }else{
            M5.Lcd.print("connect yukirin_my_angel\n");
          }
        }else{
          M5.Lcd.print("connect Buffalo-G-620E\n");
        }
      }else{
        M5.Lcd.print("connect Buffalo-A-620E\n");
      }
    }else{
      M5.Lcd.print("connect yukirin_my_angel\n");
    }
  }while(WiFi.status() != WL_CONNECTED);

  return (bool)(WiFi.status() == WL_CONNECTED);
}

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + TIME_ZONE * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void setup() {
  m5.begin();
  // Connection Wifi
  if( connectWifi() != false ){
    // NTPサーバから情報を取得する
    Udp.begin(localPort);
    Serial.println("waiting for sync");
    setSyncProvider(getNtpTime);
  }
#if 0
  preferences.begin("wifi-config");

  delay(10);
  if (restoreConfig()) {
    if (checkConnection()) {
      settingMode = false;
      startWebServer();
      return;
    }
  }
  settingMode = true;
  setupMode();
#endif
}

void loop() {
  if (settingMode) {
  }
  webServer.handleClient();
}

boolean restoreConfig() {
  wifi_ssid = preferences.getString("WIFI_SSID");
  wifi_password = preferences.getString("WIFI_PASSWD");
  Serial.print("WIFI-SSID: ");
  M5.Lcd.print("WIFI-SSID: ");
  Serial.println(wifi_ssid);
  M5.Lcd.println(wifi_ssid);
  Serial.print("WIFI-PASSWD: ");
  M5.Lcd.print("WIFI-PASSWD: ");
  Serial.println(wifi_password);
  M5.Lcd.println(wifi_password);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  if(wifi_ssid.length() > 0) {
    return true;
} else {
    return false;
  }
}

boolean checkConnection() {
  int count = 0;
  Serial.print("Waiting for Wi-Fi connection");
  M5.Lcd.print("Waiting for Wi-Fi connection");
  while ( count < 30 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      M5.Lcd.println();
      Serial.println("Connected!");
      M5.Lcd.println("Connected!");
      return (true);
    }
    delay(500);
    Serial.print(".");
    M5.Lcd.print(".");
    count++;
  }
  Serial.println("Timed out.");
  M5.Lcd.println("Timed out.");
  return false;
}

void startWebServer() {
  if (settingMode) {
    Serial.print("Starting Web Server at ");
    M5.Lcd.print("Starting Web Server at ");
    Serial.println(WiFi.softAPIP());
    M5.Lcd.println(WiFi.softAPIP());
    webServer.on("/settings", []() {
      String s = "<h1>Wi-Fi Settings</h1><p>Please enter your password by selecting the SSID.</p>";
      s += "<form method=\"get\" action=\"setap\"><label>SSID: </label><select name=\"ssid\">";
      s += ssidList;
      s += "</select><br>Password: <input name=\"pass\" length=64 type=\"password\"><input type=\"submit\"></form>";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    });
    webServer.on("/setap", []() {
      String ssid = urlDecode(webServer.arg("ssid"));
      Serial.print("SSID: ");
      M5.Lcd.print("SSID: ");
      Serial.println(ssid);
      M5.Lcd.println(ssid);
      String pass = urlDecode(webServer.arg("pass"));
      Serial.print("Password: ");
      M5.Lcd.print("Password: ");
      Serial.println(pass);
      M5.Lcd.println(pass);
      Serial.println("Writing SSID to EEPROM...");
      M5.Lcd.println("Writing SSID to EEPROM...");

      // Store wifi config
      Serial.println("Writing Password to nvr...");
      M5.Lcd.println("Writing Password to nvr...");
      preferences.putString("WIFI_SSID", ssid);
      preferences.putString("WIFI_PASSWD", pass);

      Serial.println("Write nvr done!");
      M5.Lcd.println("Write nvr done!");
      String s = "<h1>Setup complete.</h1><p>device will be connected to \"";
      s += ssid;
      s += "\" after the restart.";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
      delay(3000);
      ESP.restart();
    });
    webServer.onNotFound([]() {
      String s = "<h1>AP mode</h1><p><a href=\"/settings\">Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("AP mode", s));
    });
  }
  else {
    Serial.print("Starting Web Server at ");
    M5.Lcd.print("Starting Web Server at ");
    Serial.println(WiFi.localIP());
    M5.Lcd.println(WiFi.localIP());
    webServer.on("/", []() {
      String s = "<h1>STA mode</h1><p><a href=\"/reset\">Reset Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("STA mode", s));
    });
    webServer.on("/reset", []() {
      // reset the wifi config
      preferences.remove("WIFI_SSID");
      preferences.remove("WIFI_PASSWD");
      String s = "<h1>Wi-Fi settings was reset.</h1><p>Please reset device.</p>";
      webServer.send(200, "text/html", makePage("Reset Wi-Fi Settings", s));
      delay(3000);
      ESP.restart();
    });
  }
  webServer.begin();
}

void setupMode() {
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  delay(100);
  Serial.println("");
  M5.Lcd.println("");
  for (int i = 0; i < n; ++i) {
    ssidList += "<option value=\"";
    ssidList += WiFi.SSID(i);
    ssidList += "\">";
    ssidList += WiFi.SSID(i);
    ssidList += "</option>";
  }
  delay(100);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  WiFi.mode(WIFI_MODE_AP);
  // WiFi.softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet);
  // WiFi.softAP(const char* ssid, const char* passphrase = NULL, int channel = 1, int ssid_hidden = 0);
  // dnsServer.start(53, "*", apIP);
  startWebServer();
  Serial.print("Starting Access Point at \"");
  M5.Lcd.print("Starting Access Point at \"");
  Serial.print(apSSID);
  M5.Lcd.print(apSSID);
  Serial.println("\"");
  M5.Lcd.println("\"");
}

String makePage(String title, String contents) {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += "</body></html>";
  return s;
}

String urlDecode(String input) {
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}

#endif
#include <M5Stack.h>
//#include "ESP32_LCD_ILI9341_SPI_V2.h" //今回作成した新しいライブラリ
//#include "ESP32_Button_Switch.h"
//#include "ESP32_SD_ShinonomeFNT.h" //beta ver 1.22-
//#include "ESP32_SD_UTF8toSJIS.h" //beta ver 1.22-
#include "ESP32_WebGet.h" //beta ver 1.12-
#include "TimeLib.h" //Use Arduino time library ver1.5-

 //--------NTP関連定義---------------
const int TIME_ZONE = 9; //Tokyo
const char *NTP_SERVER_NAME = "time.windows.com";
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov

void NTP_Get(uint32_t get_interval);

const int8_t sck = 18; // SPI clock pin
const int8_t miso = -1; // MISO(master input slave output) don't using
const int8_t mosi = 23; // MOSI(master output slave input) pin
const int8_t cs = 14; // Chip Select pin
const int8_t dc = 27; // Data/Command pin
const int8_t rst = 33; // Reset pin
const int8_t LCD_LEDpin = 32;
 
const uint8_t CS_SD = 4; //SD card CS ( Chip Select )
 
const char* ssid = "xxxxxxxx"; //ご自分のルーターのSSIDに書き換えてください
const char* password = "xxxxxxxx"; //ご自分のルーターのパスワードに書き換えてください
 
const char* UTF8SJIS_file = "/font/Utf8Sjis.tbl"; //UTF8 Shift_JIS 変換テーブルファイル名を記載しておく
const char* Shino_Zen_Font_file = "/font/shnmk16.bdf"; //オリジナル東雲全角フォントファイル
const char* Shino_Half_Font_file = "/font/shnm8x16.bdf"; //半角フォントファイル名を定義
//const char* Shino_Zen_Font_file = "/font/MYshnmk16.bdf"; //自作改変全角東雲フォントファイル
 
const uint8_t MAX_TXT_NUM = 6; //スクロール表示最大数
 
#if 0
ESP32_LCD_ILI9341_SPI_V2 LCD; //今回作成した新しいライブラリクラス
ESP32_SD_ShinonomeFNT SFR(CS_SD, 40000000);
ESP32_Button_Switch BTN;
#endif
ESP32_WebGet _EWG;
#if 0
FONT_PARAM font[ MAX_TXT_NUM ]; //フォントパラメーター入力用クラス
ScrolleSET_PARAM scl[ MAX_TXT_NUM ]; //スクロール設定入力用クラス
#endif 
//------時刻文字表示系　引数初期化------------
uint8_t num = 0; //文字列番号
uint8_t time_sj_txt[ MAX_TXT_NUM ][ 20 ] = {}; //Shift_JIS文字コード格納
uint16_t time_sj_length[ MAX_TXT_NUM ] = {}; //文字列のShift_JISコード長
uint8_t time_font_buf[ MAX_TXT_NUM ][2][16] = {}; //16x16フォント全角１文字格納バッファ
uint16_t Fnt_Cnt[MAX_TXT_NUM] = {}; //半角フォント１文字スクロールカウント
 
//時刻数値を各桁に分割した変数定義
int8_t prev_hour1 = -1, prev_hour2 = -1;
int8_t now_hour1 = -1, now_hour2 = -1;
int8_t prev_min1 = -1, prev_min2 = -1;
int8_t now_min1 = -1, now_min2 = -1;
int8_t prev_sec1 = -1, prev_sec2 = -1;
int8_t now_sec1 = -1, now_sec2 = -1;
int32_t LastTime = 0;
bool startScrolle[ MAX_TXT_NUM ] = {};

int8_t prev_hour = -1;
int8_t now_hour = -1;
int8_t prev_min = -1;
int8_t now_min = -1;
int8_t prev_sec = -1;
int8_t now_sec = -1;
byte xcolon = 0, xsecs = 0;

//時刻コロンフォント表示用変数
//FONT_PARAM colon1_fnt, colon2_fnt;
uint8_t colon1_buf[ 2 ][ 16 ] = {}, colon2_buf[ 2 ][ 16 ] = {};
uint16_t colon1_sj_len = 0, colon2_sj_len = 0;
uint32_t colon_LastTime = 0;
bool on_ColonDisp = false;
 
//-----NTPサーバー時刻取得　引数初期化------------
uint32_t NTPGetLastTime = 0;
bool NTP_first_get = true;
 
//-----ボタンスイッチ　引数初期化-----------
const uint8_t buttonA_GPIO = 39;
const uint8_t buttonB_GPIO = 38;
const uint8_t buttonC_GPIO = 37;
//uint8_t btn_stateA = _Release;
//uint8_t btn_stateB = _Release;
//uint8_t btn_stateC = _Release;
//-----文字チェンジ用変数定義---------------
uint32_t change_font_LastTime = 0;
bool change_font = false;



//***********セットアップ****************************
void setup() {
  delay(1000);
  m5.begin();
  Serial.begin(115200);
#if 0
  pinMode(buttonA_GPIO, INPUT); //GPIO #39 は内部プルアップ無し
  pinMode(buttonB_GPIO, INPUT); //GPIO #38 は内部プルアップ無し
  pinMode(buttonC_GPIO, INPUT); //GPIO #37 は内部プルアップ無し
 
  SFR.SD_Shinonome_Init3F(UTF8SJIS_file, Shino_Half_Font_file, Shino_Zen_Font_file); //ライブラリ初期化。3ファイル同時に開く
 
  LCD.ILI9341_Init(sck, miso, mosi, cs, dc, rst, LCD_LEDpin, 40000000, false); //microSDを使う場合、必ず false にする
  LCD.Display_Clear();
  LCD.Brightness(255); //LCD LED Full brightness
 
  num = 0; //時、十の位の文字列をフォント変換
  TimeStrFontConv( num, "　拾二三四五六七八九" );
  num = 1; //時、一の位
  TimeStrFontConv( num, "零壱弐参四伍六七八九" );
  num = 2; //分、十の位
  TimeStrFontConv( num, "零拾二三四五六七八九" );
  num = 3; //分、一の位
  TimeStrFontConv( num, "零壱弐参四伍六七八九" );
  num = 4; //秒、十の位
  TimeStrFontConv( num, "零拾二三四五六七八九" );
  num = 5; //秒、一の位
  TimeStrFontConv( num, "零壱弐参四伍六七八九" );
  //コロン文字フォント変換
  colon1_sj_len = SFR.StrDirect_ShinoFNT_readALL("時", colon1_buf);
  colon2_sj_len = SFR.StrDirect_ShinoFNT_readALL("分", colon2_buf);
 
  //メッセージウィンドウ表示
  Message(" WiFi Connecting...");
#endif 
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
 
  if( change_font == false && LastTime != now() ){
    on_ColonDisp = true; //コロン表示開始フラグ
 
    //YMDW_disp(); //年月日、曜日表示
#if 0 
    //時刻文字をフォント変換
    now_hour1 = hour() / 10; //時、十の//位
    if( now_hour1 != prev_hour1 ){
      num = 0;
      Fnt_Cnt[num] = now_hour1 * 2; //全角は8x16フォント２つ分なので、フォントカウントを2倍進める
      prev_hour1 = now_hour1;
      startScrolle[num] = true;
    }
    now_hour2 = hour() % 10; //時、一の位
    if( now_hour2 != prev_hour2 ){
      num = 1;
      Fnt_Cnt[num] = now_hour2 * 2;
      prev_hour2 = now_hour2;
      startScrolle[num] = true;
    }
    now_min1 = minute() / 10; //分、十の位
    if( now_min1 != prev_min1 ){
      num = 2;
      Fnt_Cnt[num] = now_min1 * 2;
      prev_min1 = now_min1;
      startScrolle[num] = true;
    }
    now_min2 = minute() % 10; //分、一の位
    if( now_min2 != prev_min2 ){
      num = 3;
      Fnt_Cnt[num] = now_min2 * 2;
      prev_min2 = now_min2;
      startScrolle[num] = true;
    }
    now_sec1 = second() / 10; //秒、十の位
    if( now_sec1 != prev_sec1 ){
      num = 4;
      Fnt_Cnt[num] = now_sec1 * 2;
      prev_sec1 = now_sec1;
      startScrolle[num] = true;
    }
    now_sec2 = second() % 10; //秒、一の位
    if( now_sec2 != prev_sec2 ){
      num = 5;
      Fnt_Cnt[num] = now_sec2 * 2;
      prev_sec2 = now_sec2;
      startScrolle[num] = true;
    }
    for( num = 0; num < MAX_TXT_NUM; num++ ){ //フォント変換
      scl[num].Zen_or_Han = SFR.Sjis_inc_FntRead(time_sj_txt[num], time_sj_length[num], &Fnt_Cnt[num], time_font_buf[num]);
    }
#endif

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
  }else if( change_font == true ){
    if( now() - change_font_LastTime > 2 ){
      change_font = false;
      //Message(" Change Font OK!");
    }
  }
#if 0 
  num = 0; //時、十の位文字スクロール
  if( startScrolle[num] ){
    if( LCD.Rev_Scrolle_Inc_8x16_Font( font[num], scl[num], time_sj_length[num], time_font_buf[num] ) ){
      startScrolle[num] = false; //一文字スクロール後、スクロールストップ
    }
  }
  num = 1; //時、一の位文字スクロール
  if( startScrolle[num] ){
    if( LCD.Y_Scrolle_Inc_8x16_Font( font[num], scl[num], time_sj_length[num], time_font_buf[num] ) ){
      startScrolle[num] = false;
    }
  }
  num = 2; //分、十の位文字スクロール
  if( startScrolle[num] ){
    if( LCD.Scrolle_Inc_8x16_Font( font[num], scl[num], time_sj_length[num], time_font_buf[num] ) ){
      startScrolle[num] = false;
    }
  }
  num = 3; //分、一の位文字スクロール
  if( startScrolle[num] ){
    if( LCD.Ydown_Scrolle_Inc_8x16_Font( font[num], scl[num], time_sj_length[num], time_font_buf[num] ) ){
      startScrolle[num] = false;
    }
  }
  num = 4; //秒、十の位文字スクロール
  if( startScrolle[num] ){
    if( LCD.Rev_Scrolle_Inc_8x16_Font( font[num], scl[num], time_sj_length[num], time_font_buf[num] ) ){
      startScrolle[num] = false;
    }
  }
  num = 5; //秒、一の位文字スクロール
  if( startScrolle[num] ){
    if( LCD.Y_Scrolle_Inc_8x16_Font( font[num], scl[num], time_sj_length[num], time_font_buf[num] ) ){
      startScrolle[num] = false;
    }
  }
 
  Colon_Disp(); //時刻コロン表示
  button_action(); //※ボタン操作でヒープ領域の配列を超えてしまう場合があるので、同じタスクに置くこと
  #endif
}
#if 0
//************ 時刻コロン表示 ********************************
void Colon_Disp(){
  if( on_ColonDisp ){
    LCD.XYsizeUp_8x16_Font_DisplayOut(colon1_fnt, colon1_sj_len, colon1_buf);
    LCD.XYsizeUp_8x16_Font_DisplayOut(colon2_fnt, colon2_sj_len, colon2_buf);
    on_ColonDisp = false;
    colon_LastTime = millis();
  }else if( millis() - colon_LastTime > 500 ){
    LCD.Display_Clear(colon1_fnt.x0, colon1_fnt.y0, colon1_fnt.x0 + colon1_fnt.Xsize * 16, colon1_fnt.y0 + colon1_fnt.Ysize * 16 );
    LCD.Display_Clear(colon2_fnt.x0, colon2_fnt.y0, colon2_fnt.x0 + colon2_fnt.Xsize * 16, colon2_fnt.y0 + colon2_fnt.Ysize * 16 );
    colon_LastTime = millis();
  }
}
//******** ニュース記事取得中、待機メッセージ表示*******
void Message(String str){
  uint8_t f_buf[ 20 ][ 16 ] = {};
  uint16_t len = SFR.StrDirect_ShinoFNT_readALL(str, f_buf);
  FONT_PARAM fnt_msg;
  delay(10);
  fnt_msg.Xsize = 2, fnt_msg.Ysize = 2;
  fnt_msg.red = 31, fnt_msg.green = 63, fnt_msg.blue = 31;
  fnt_msg.bg_red = 0, fnt_msg.bg_green = 0, fnt_msg.bg_blue = 0;
  fnt_msg.x0 = 1, fnt_msg.y0 = 4 * 48 + 1;
  LCD.Display_Clear(1, fnt_msg.y0 + 1, 318, 238);
  fnt_msg.y0 = fnt_msg.y0 + 10;
  LCD.XYsizeUp_8x16_Font_DisplayOut(fnt_msg, len, f_buf);
  LCD.Draw_Rectangle_Line(0, 4 * 48, 319, 239, 31, 63, 31);
}
//*******************************************************
void Font_Setup(){
  uint8_t X_max_txt = 2; //横スクロール時刻文字、各桁全角１文字表示(半角２文字分）
  uint8_t Y_max_txt = 1; //縦スクロール時刻文字、各桁全角１文字表示(半角２文字分）
  for( num = 0; num < MAX_TXT_NUM; num++ ){
    LCD.Array2_Delete( scl[num] ); //ヒープ領域メモリ、配列消去
  }
  for( num = 0; num < MAX_TXT_NUM; num++ ){
    font[num].Xsize = 3, font[num].Ysize = 6;
    font[num].y0 = 0; //各文字開始位置
  }
 
  num = 0; //時表示十の位
  scl[num].SclSpeed = 50;
  font[num].x0 = 0; //各文字開始位置
  font[num].red = 0, font[num].green = 0, font[num].blue = 0;
  font[num].bg_red = 0, font[num].bg_green = 50, font[num].bg_blue = 0;
  //水平方向スクロール文字フォント配列生成、および初期化
  LCD.X_Scrolle_Font_2Array_Init( font[num], scl[num], X_max_txt, font[num].Xsize, font[num].Ysize);
 
  num = 1; //時表示一の位
  scl[num].SclSpeed = 50;
  font[num].x0 = 48;
  font[num].red = 0, font[num].green = 0, font[num].blue = 0;
  font[num].bg_red = 29, font[num].bg_green = 58, font[num].bg_blue = 0;
  //垂直方向スクロール文字フォント配列生成、および初期化
  LCD.Y_Scrolle_Font_2Array_Init( font[num], scl[num], Y_max_txt, font[num].Xsize, font[num].Ysize );
 
  num = 2; //分表示十の位
  scl[num].SclSpeed = 50;
  font[num].x0 = 112;
  font[num].red = 31, font[num].green = 63, font[num].blue = 31;
  font[num].bg_red = 29, font[num].bg_green = 0, font[num].bg_blue = 0;
  //水平方向スクロール文字フォント配列生成、および初期化
  LCD.X_Scrolle_Font_2Array_Init( font[num], scl[num], X_max_txt, font[num].Xsize, font[num].Ysize );
 
  num = 3; //分表示一の位
  scl[num].SclSpeed = 50;
  font[num].x0 = 160;
  font[num].red = 0, font[num].green = 0, font[num].blue = 0;
  font[num].bg_red = 31, font[num].bg_green = 63, font[num].bg_blue = 31;
  //垂直方向スクロール文字フォント配列生成、および初期化
  LCD.Y_Scrolle_Font_2Array_Init( font[num], scl[num], Y_max_txt, font[num].Xsize, font[num].Ysize );  LCD.Scrolle_Font_SetUp( font[num], scl[num] );
 
  num = 4; //秒表示十の位
  scl[num].SclSpeed = 30;
  font[num].x0 = 224;
  font[num].red = 31, font[num].green = 63, font[num].blue = 31;
  font[num].bg_red = 20, font[num].bg_green = 0, font[num].bg_blue = 20;
  //水平方向スクロール文字フォント配列生成、および初期化
  LCD.X_Scrolle_Font_2Array_Init( font[num], scl[num], X_max_txt, font[num].Xsize, font[num].Ysize );
 
  num = 5; //秒表示一の位
  scl[num].SclSpeed = 24;
  font[num].x0 = 272;
  font[num].red = 31, font[num].green = 63, font[num].blue = 31;
  font[num].bg_red = 0, font[num].bg_green = 31, font[num].bg_blue = 0;
  //垂直方向スクロール文字フォント配列生成、および初期化
  LCD.Y_Scrolle_Font_2Array_Init( font[num], scl[num], Y_max_txt, font[num].Xsize, font[num].Ysize );
 
  for( num = 0; num < MAX_TXT_NUM; num++ ){
    LCD.Scrolle_Font_SetUp( font[num], scl[num] );
  }
 
  //コロンフォント初期化
  colon1_fnt.x0 = 96, colon1_fnt.y0 = 32;
  colon1_fnt.Xsize = 1, colon1_fnt.Ysize = 4;
  colon1_fnt.red = 31, colon1_fnt.green = 63, colon1_fnt.blue = 31;
  colon1_fnt.bg_red = 0, colon1_fnt.bg_green = 0, colon1_fnt.bg_blue = 0;
 
  colon2_fnt.x0 = 208, colon2_fnt.y0 = 32;
  colon2_fnt.Xsize = 1, colon2_fnt.Ysize = 4;
  colon2_fnt.red = 31, colon2_fnt.green = 63, colon2_fnt.blue = 31;
  colon2_fnt.bg_red = 0, colon2_fnt.bg_green = 0, colon2_fnt.bg_blue = 0;
 
  Serial.printf("Free Heap Size = %d\r\n", esp_get_free_heap_size());
}
#endif
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
#if 0
//********** 時刻文字列フォント変換 *****************
void TimeStrFontConv(uint8_t num, String str){
  Fnt_Cnt[num] = 0;
  time_sj_length[num] = SFR.UTF8toSJIS_convert( str, time_sj_txt[num]);
  scl[num].Zen_or_Han = SFR.Sjis_inc_FntRead(time_sj_txt[num], time_sj_length[num], &Fnt_Cnt[num], time_font_buf[num]);
}
//********** 年月日曜日表示 *************************
void YMDW_disp(){
  //Timeライブラリ weekday()関数は1～7の範囲数値を返す
  char week_c[7][4] = { "日", "月", "火", "水", "木", "金", "土" };
  String ymd_str = "";
  char month_chr[ 3 ] = {}, day_chr[ 3 ] = {};
  uint8_t ymd_buf[40][16] = {};
  FONT_PARAM ymd_fnt;
 
  sprintf( month_chr, "%2d", month() );
  sprintf( day_chr, "%02d", day() );
  ymd_str = String( year()) + "年";
  ymd_str += String( month_chr) + "月";
  ymd_str += String( day_chr) + '(';
  ymd_str += String( week_c[ weekday() - 1 ] ) + "曜日)";
 
  uint16_t ymd_len = SFR.StrDirect_ShinoFNT_readALL( ymd_str, ymd_buf );
  ymd_fnt.x0 = 0;
  ymd_fnt.y0 = 106;
  ymd_fnt.Xsize = 2, ymd_fnt.Ysize = 2;
  ymd_fnt.red = 31, ymd_fnt.green = 63, ymd_fnt.blue = 31;
  ymd_fnt.bg_red = 0, ymd_fnt.bg_green = 0, ymd_fnt.bg_blue = 0;
  LCD.XYsizeUp_8x16_Font_DisplayOut(ymd_fnt, ymd_len, ymd_buf);
}
//****************************************
void Time_Disp_Reset(){
  Message(" Change Font Wait...");
  prev_hour1 = -1, prev_hour2 = -1;
  now_hour1 = -1, now_hour2 = -1;
  prev_min1 = -1, prev_min2 = -1;
  now_min1 = -1, now_min2 = -1;
  prev_sec1 = -1, prev_sec2 = -1;
  now_sec1 = -1, now_sec2 = -1;
  change_font = true;
  LastTime = 0;
  change_font_LastTime = now();
}
//****************************************
void button_action(){
  btn_stateA = BTN.Button(0, buttonA_GPIO, true, 10, 300);
  switch( btn_stateA ){
    case _MomentPress:
      Serial.println("Button A Moment Press");
      for( num = 0; num < MAX_TXT_NUM; num++ ){
        TimeStrFontConv( num, "０１２３４５６７８９" );
      }
      colon1_sj_len = SFR.StrDirect_ShinoFNT_readALL("：", colon1_buf);
      colon2_sj_len = SFR.StrDirect_ShinoFNT_readALL("：", colon2_buf);
      Time_Disp_Reset();
      break;
    case _ContPress:
      Serial.println("-------------Button A Cont Press");
      for( num = 0; num < MAX_TXT_NUM; num++ ){
        TimeStrFontConv( num, "れひふみよいむなやこ" );
      }
      colon1_sj_len = SFR.StrDirect_ShinoFNT_readALL("じ", colon1_buf);
      colon2_sj_len = SFR.StrDirect_ShinoFNT_readALL("ふ", colon2_buf);
      Time_Disp_Reset();
      break;
    default:
      break;
  }
 
  btn_stateB = BTN.Button(1, buttonB_GPIO, true, 10, 300);
  switch( btn_stateB ){
    case _MomentPress:
      Serial.println("Button B Moment Press");
      for( num = 0; num < MAX_TXT_NUM; num++ ){
        TimeStrFontConv( num, "〇一二三四五六七八九" );
      }
      colon1_sj_len = SFR.StrDirect_ShinoFNT_readALL("時", colon1_buf);
      colon2_sj_len = SFR.StrDirect_ShinoFNT_readALL("分", colon2_buf);
      Time_Disp_Reset();
      break;
    case _ContPress:
      for( num = 0; num < MAX_TXT_NUM; num++ ){
        TimeStrFontConv( num, "零壱弐参四伍六七八九" );
      }
      colon1_sj_len = SFR.StrDirect_ShinoFNT_readALL("時", colon1_buf);
      colon2_sj_len = SFR.StrDirect_ShinoFNT_readALL("分", colon2_buf);
      Time_Disp_Reset();
      break;
    default:
      break;
  }
 
  btn_stateC = BTN.Button(2, buttonC_GPIO, true, 10, 300);
  switch( btn_stateC ){
    case _MomentPress:
      Serial.println("Button C Moment Press");
      for( num = 0; num < 4; num++ ){
        font[num].red = 0, font[num].green = 63, font[num].blue = 31;
        font[num].bg_red = 0, font[num].bg_green = 0, font[num].bg_blue = 0;
        LCD.Scrolle_Font_Color_Set( font[num] );
      }
      for( num = 4; num < 6; num++ ){
        font[num].red = 31, font[num].green = 10, font[num].blue = 25;
        font[num].bg_red = 0, font[num].bg_green = 0, font[num].bg_blue = 0;
        LCD.Scrolle_Font_Color_Set( font[num] );
      }
      Time_Disp_Reset();
      break;
    case _ContPress:
      Serial.println("-------------Button C Cont Press");
      Font_Setup();
      Time_Disp_Reset();
      break;
    default:
      break;
  }
}
#endif