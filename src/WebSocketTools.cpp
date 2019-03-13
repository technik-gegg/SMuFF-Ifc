#include "WebSocketTools.h"

char b[256];

extern unsigned long dataCntDuet, dataCntPanelDue, dataCntSmuff, dataCntI2C;

/**
 * This method scans the available WiFi networks when requested by the index.html through parseCommand.
 * 
 * @returns The number of networks in range.
 */
int scanNetworks() {
  int n = WiFi.scanNetworks();
  __debug("WiFi scan done. %s networks found.\n", (n == 0) ? "No" : String(n).c_str());
  return n;
}

/**
 * Method to parse/alter settings.
 * 
 * @param     data      The additional data describing the setting.
 * @param     client    The client instance who initiated the action.
 * @returns   Nothing.
 */
void parseSetting(const char* data, AsyncWebSocketClient* client) 
{
  String cmd = (const char*)data;
  if(cmd == "BtMirrorMode") {
    _config.btMirrorMode = true;
    writeConfig();
  }
  if(cmd == "BtDebugMode") {
    _config.btMirrorMode = false;
    writeConfig();
  }
  if(cmd == "I2CMode") {
    _config.i2cMode = true;
    writeConfig();
  }
  if(cmd == "UartMode") {
    _config.i2cMode = false;
    writeConfig();
  }
  if((cmd == "BtMirrorMode") || (cmd == "BtDebugMode")) {
    sprintf(b, "M:%s", _config.btMirrorMode ? "1" : "0");
    client->text(b);
  }
  if((cmd == "I2CMode") || (cmd == "UartMode")) {
    sprintf(b, "I:%s", _config.i2cMode ? "1" : "0");
    client->text(b);
  }
}

/**
 * Method to parse commands from within the index.html and answer accordingly.
 * 
 * @param     data      The additional data describing the setting.
 * @param     client    The client instance who initiated the action.
 * @returns   Nothing.
 */
void parseCommand(const char* data, AsyncWebSocketClient* client) 
{
  String cmd = (const char*)data;
  if(cmd == "WifiMode") {
    if(_config.isAP)
      sprintf(b, "W:AP");
    else
      sprintf(b, "W:'%s'", WiFi.SSID().c_str());
    client->text(b);
  }
  else if(cmd == "BtMirrorMode") {
    sprintf(b, "M:%s", _config.btMirrorMode ? "1" : "0");
    client->text(b);
  }
  else if(cmd == "I2CMode") {
    sprintf(b, "I:%s", _config.i2cMode ? "1" : "0");
    client->text(b);
  }
  else if(cmd.startsWith("ChangeWifi")) {
    String wifi = (const char*)&data[11];
    if(wifi=="AP") {
      _config.isAP = true;
      memset(_config.ssid, 0,sizeof(_config.ssid));
      memset(_config.pwd, 0,sizeof(_config.pwd));
      sprintf(b, "X:AP");
    }
    else {
      _config.isAP = false;
      wifi.toCharArray(_config.ssid, sizeof(_config.ssid));
      sprintf(b, "X:%s", _config.ssid);
    }
    client->text(b);
  }
  else if(cmd == "Reset") {
    writeConfig();
    delay(500);
    ESP.restart();
  }
  else if(cmd.startsWith("ChangePwd")) {
    size_t len;
    unsigned char encodedPwd[64];
    unsigned char cipherPwd[64];
    cipher((unsigned char*)&data[10], cipherPwd);
    int result = mbedtls_base64_encode(encodedPwd, sizeof(encodedPwd), &len, cipherPwd, 16);
    switch(result) {
      case MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL:
          client->text("E:BUFFER_TOO_SMALL");
          return;
      case MBEDTLS_ERR_BASE64_INVALID_CHARACTER:
          client->text("E:INVALID_CHARACTER");
          return;
    }
    strlcpy(_config.pwd, (const char*)encodedPwd, sizeof(_config.pwd));
    sprintf(b, "P:%s", encodedPwd);
    client->text("P:OK");
  }
  else if(cmd == "Scan") {
    int cnt = scanNetworks();
    
    for (int i = 0; i < cnt; ++i) {
      sprintf(b, "S:%3d\t%s\t%d\t%s", i+1,  WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "*" : "");
      client->text(b);
      __debug(">>> %s\n", b);
      delay(10);
    }
    sprintf(b, "S:-");
    client->text(b);
  }
  else if(cmd == "DataCounter") {
    sprintf(b, "C:%lu\t%lu\t%lu\t%lu", dataCntDuet, dataCntPanelDue, dataCntSmuff, dataCntI2C);
    client->text(b);
    //__debug(">>> %s\n", b);
  }
  else {
    client->text("E:Unknown function");
  }
 
}
