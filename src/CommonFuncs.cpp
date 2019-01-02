#include "CommonFuncs.h"

void EncString( char* pSrc, char* pEnc, size_t len ){
  uint8_t mac[6];
  esp_efuse_mac_get_default(mac);
  int i;
  for( i = 0; i < len; i++ ){
    pEnc[i] = (pSrc[i] + mac[i%6]) & 0xFF;
  }
}

void DecString( char* pSrc, char* pDec, size_t len){
  uint8_t mac[6];
  esp_efuse_mac_get_default(mac);

  int i;
  for( i = 0; i < len; i++ ){
    pDec[i] = (pSrc[i] + 0x100 - mac[i%6]) & 0xFF;
  }
}