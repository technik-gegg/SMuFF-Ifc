/**
 * SMuFF IFC Firmware
 * Copyright (C) 2019 Technik Gegg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <HardwareSerial.h>
#include <BluetoothSerial.h>
#include <Wire.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "SmuffIfcConfig.h"
#include "Cipher.h"
#include "WebSocketTools.h"
#include "driver/i2c.h"

#define HOST_NAME     "smuffifc"                  // basic host name
#define RXD0          3
#define TXD0          1
#define RXD1          16
#define TXD1          17
#define RXD2          18
#define TXD2          19
#define SIGNAL1_PIN   12
#define SIGNAL2_PIN   13
#define SIGNAL3_PIN   14
#define LED_PIN       2

#define DATA_LENGTH           128
#define I2C_SLAVE_ADDR        0x58
#define I2C_SLAVE_TX_BUF_LEN  (2 * DATA_LENGTH)
#define I2C_SLAVE_RX_BUF_LEN  (2 * DATA_LENGTH)
#define I2C_SLAVE_SCL_IO      GPIO_NUM_26
#define I2C_SLAVE_SDA_IO      GPIO_NUM_25

extern SmuffIfcConfig  _config;

AsyncWebServer  webServer(80);
AsyncWebSocket  winsock("/smuffifc");     // access at ws://[esp ip-address]/smuffifc (see index.html)
HardwareSerial  SerialDuet(0);
HardwareSerial  SerialSmuff(1);
HardwareSerial  SerialPanelDue(2);
BluetoothSerial SerialBT;                 // used for debugging or mirroring trafic to PanelDue 

const char* passwordAP = "12345678";      // default password when in AP mode

IPAddress apLocalIp(192, 168, 44, 1);     // IP-Address when in AP mode (must be defined for DHCP)
IPAddress apGateway(192, 168, 44, 1);
IPAddress apNetmask(255, 255, 255, 0);

unsigned long baudrate = 57600;           // must match the settings of the Duet3D/PanelDue
static bool   smuffMode = false;
static bool   cmdMode = false;
static int    pinSelected = 0;
IPAddress     localIp;
bool          webServerRunning = false;
bool          mdnsRunning = false;
unsigned long dataCntSmuff = 0;         // data counters for debugging
unsigned long dataCntDuet = 0;
unsigned long dataCntPanelDue = 0;
unsigned long dataCntI2C = 0;
byte          i2cBuffer[40];

String _hostname = HOST_NAME;

/**
 * Function to output some debug information to a serial port (Bluetooth in this case).
 * 
 * @param fmt   The desired format
 * @param ...   A list of parameters that match the format string
 * @returns     Nothing
 */
void __debug(const char* fmt, ...) {
  if(!_config.btMirrorMode) {
    char _tmp[1024];
    va_list arguments;
    va_start(arguments, fmt); 
    vsnprintf(_tmp, 1024, fmt, arguments);
    va_end (arguments); 
    SerialBT.print(_tmp);
  }
}

/**
 * Toggles the LED. Used for testing purposes.
 */
void blinkLED() {
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

/**
 * Function to decrypt an encrypted password.
 * This method is used to decrypt the encrypted WiFi password stored in the 
 * configuration file if you decide to use STA mode instead of AP mode.
 * 
 * @param output  A pointer to the character array receiving the dechipherd password.
 */
void getDecipherdPwd(unsigned char* output)
{
    size_t len;
    unsigned char decodedPwd[64] ;
    int result = mbedtls_base64_decode(decodedPwd, sizeof(decodedPwd), &len, (const unsigned char*)_config.pwd, strlen(_config.pwd));
    switch(result) {
      case 0: break;
      case MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL:
          __debug("BUFFER_TOO_SMALL\n");
          break;
      case MBEDTLS_ERR_BASE64_INVALID_CHARACTER:
          __debug("INVALID_CHARACTER\n");
          break;
    }
    decipher(decodedPwd, output);
}

/**
 * Sets up the Over-The-Air update feature of the ESP.
 * Might be useful if it comes to update the SMuFF-Ifc firmware while attached to the other electronics. 
 * 
 * @returns	  Nothing
 */
void setupOTA() {

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH)
			type = "sketch";
		else // U_SPIFFS
			type = "filesystem";
			__debug("SMuFF-OTA: Start updating %s\n", type);
		}).onEnd([]() {
		__debug("SMUFF-OTA: End\n");
	}).onProgress([](unsigned int progress, unsigned int total) {
		__debug("SMuFF-OTA: Progress: %u%%\r", (progress / (total / 100)));
	}).onError([](ota_error_t error) {
		__debug("SMuFF-OTA: Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR)          __debug("Auth Failed\n");
		else if (error == OTA_BEGIN_ERROR)    __debug("Begin Failed\n");
		else if (error == OTA_CONNECT_ERROR)  __debug("Connect Failed\n");
		else if (error == OTA_RECEIVE_ERROR)  __debug("Receive Failed\n");
		else if (error == OTA_END_ERROR)      __debug("End Failed\n");
	});
	ArduinoOTA.begin();
}

/**
 * Sets up the WiFi connection depending on the settings.
 * 
 * @returns	  Nothing
 */
void setupWiFi() {
  unsigned char output[16];

  WiFi.mode(WIFI_AP_STA);   // always use AP + STA mode

  if(!_config.isAP) {
    getDecipherdPwd(output);
    esp_wifi_set_ps(WIFI_PS_NONE);
    WiFi.begin(_config.ssid, (char*)output);
    delay(1000);

    wl_status_t stat;
    int t = millis();
    do {
      stat = WiFi.status();
      switch(stat) {
        case WL_IDLE_STATUS:      break;
        case WL_NO_SSID_AVAIL:    __debug("No SSID\n"); break;
        case WL_NO_SHIELD:        __debug("No WiFi hardware found\n"); break;
        case WL_SCAN_COMPLETED:   __debug("Scan completed\n"); break;
        case WL_CONNECT_FAILED:   __debug("Connect failed\n"); break;
        case WL_CONNECTION_LOST:  __debug("Connection lost\n"); break;
        case WL_DISCONNECTED:     __debug("Disconnected\n");break;
        default: break;
      }
      if(millis()-t > 20000)
        break;
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    } while(stat != WL_CONNECTED);
    digitalWrite(LED_PIN, LOW);
    
    if(WiFi.status() != WL_CONNECTED) {
      _config.isAP = true;
    }
    localIp = WiFi.localIP();
  }
  
  // always enable AP mode - just in case
  esp_wifi_set_ps(WIFI_PS_NONE);
  String appendix = WiFi.softAPmacAddress().substring(9);
  appendix.replace(":", "");
  _hostname = String(HOST_NAME) + "_" + appendix;
  if(!WiFi.softAPConfig(apLocalIp, apGateway, apNetmask))  {
    __debug("WiFi AP config failed!\n");
  }
  WiFi.softAP(_hostname.c_str(), passwordAP);
  delay(1000);
  if(_config.isAP)
    localIp =  WiFi.softAPIP();

  if(WiFi.status() == WL_CONNECTED) {
    for(int i=0; i< 10; i++) {
      blinkLED();
      delay(150);
    }
    __debug("IP-Address: %s\n", localIp.toString().c_str());
    SerialDuet.printf("M118 S\"[SMuFF-IFC] IP-Address: %s\"\n", localIp.toString().c_str());
  }
  else {
    SerialDuet.printf("M118 S\"[SMuFF-IFC] Failed to connect to WiFi network %s.\"", localIp.toString().c_str());
  }
}

/**
 * Sets up the mobile DNS.
 * Uses the hostname and the last 6 character of the MAC address to form the unique name. 
 * 
 * @returns   Nothing. Sets mdnsRunning flag accordingly.
 */
void setupMdns() {
  if(MDNS.begin(_hostname.c_str())) {
    MDNS.addService("_http", "_tcp", 80);
    MDNS.addServiceTxt("_http", "_tcp", "board", "ESP32");
    MDNS.addServiceTxt("_http", "_tcp", "interface", "SMuFF");
    mdnsRunning = true;
  } 
  else {
    mdnsRunning = false;
  }
}

/**
 * Event handler for the WebSocket communication.
 * This event handles only text data. Binary data will be responded with Error.
 * The first two byte of data designate whether its a function call "F:" or a setting call "S:".
 * 
 * @see       parseCommand, parseSetting
 * 
 * @param     server  The server instance
 * @param     client  The connected client instance
 * @param     type    The event type
 * @param     arg     The arguments depending on the event type
 * @param     data    The data which has been received from the client
 * @param     len     The length of the data
 * @returns   Nothing
 */
void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len)
{
  if(type == WS_EVT_CONNECT){
    __debug(">>> WebSock client #%u has connected\n", client->id());
  } 
  else if(type == WS_EVT_DISCONNECT){
    __debug(">>> WebSock client #%u has disconnected\n", client->id());
  } 
  else if(type == WS_EVT_ERROR){
    __debug(">>> WebSock client #%u error(%u): %s\n", client->id(), *((uint16_t*)arg), (char*)data);
  } 
  else if(type == WS_EVT_DATA){
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if(info->final && info->index == 0 && info->len == len){
      if(info->opcode == WS_TEXT){
        data[len] = 0;
        String pfx = (const char*)data;
        if(pfx.startsWith("F:")) {
          parseCommand((const char*)&data[2], client);
        }
        else if(pfx.startsWith("S:")) {
          parseSetting((const char*)&data[2], client);
        }
        else {
          client->text("E:Wrong request");
        }
      } 
      else {
        client->text("E:No binary data supported yet");
        __debug("Binary data received from WinSock\n");
        for(size_t i=0; i < info->len; i++){
          __debug("%02x ", data[i]);
        }
        __debug("\n");
      }
    } 
  }
}

/**
 * Sets up the Web server.
 * 
 * @returns   Nothing. Sets webServerRunning flag accordingly.
 */
void setupWebServer() {

  winsock.onEvent(onEvent);
  webServer.addHandler(&winsock);
  //webServer.rewrite("/index.html", "index-ap.html").setFilter(ON_AP_FILTER);
  webServer.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html").setCacheControl("max-age=1");
  webServer.serveStatic("/js", SPIFFS, "/www/js/");
  webServer.serveStatic("/css", SPIFFS, "/www/css/");
  webServer.serveStatic("/img", SPIFFS, "/www/img/");
  
  webServer.onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });

  webServer.begin();
  webServerRunning = true;
}

/**
 * Browse SPIFFS for files.
 * 
 * @param     root      The folder where to start
 * @param     numtabs   The number of tabs to insert for each nested level
 * @returns   Nothing.
 */
void browseDir(File root, int numTabs) {
  while(true) {
    File entry =  root.openNextFile();
    if (!entry)
      break;

    for (int i = 1; i < numTabs; i++) {
      __debug("\t");
    }
    __debug(entry.name());
    if (entry.isDirectory()) {
      __debug("/\r\n");
      browseDir(entry, numTabs + 1);
    } 
    else {
      __debug("\t\t%u\r\n", entry.size());
    }
    entry.close();
  }
}

/**
 * Browses and lists the files in a directory recursively.
 * 
 * @param     fs        The file descriptor
 * @param     dirname   The directory to browse
 * @returns   Nothing.  Calls itself though (recursion), to browse and list sub directories as well.
 */
void listDir(fs::FS &fs, const char* dirname){
    
    __debug("Directory: %s\r\n", dirname);
    File root = fs.open(dirname);
    if(!root){
        __debug("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        __debug(" - not a directory");
        return;
    }
    browseDir(root, 0);
}

/**
 * Handles incoming characters from the Duet3D serial port (usually the PanelDue plug).
 * This method parses the character stream for a "\s" sequence, which will route all subsequent
 * characters until the next "\n" (Linefeed or Newline) to the SMuFF instead of the PanelDue.
 * Otherwise, it will forward all data to the PanelDue serial port and - if the Bluetooth mirror 
 * function is enabled - also to the Bluetooth serial port.
 * 
 * @returns Nothing.
 */
void handleDuetSerial() {
    char in = SerialDuet.read();
    dataCntDuet++;
    if(smuffMode && in != '\\')
      __debug("[%02X]", in);
    if(in == '\\') {
      while(!SerialDuet.available())  
        ;                             // wait for next character
      char nxt = SerialDuet.read();
      switch(nxt) {
        case '\\': in = '\\'; break;
        case 'n': in = '\n'; break;
        case 'r': in = '\r'; break;
        case 'a': in = '\a'; break;
        case 'b': in = '\b'; break;
        case 't': in = '\t'; break;
        case 's': in = 0;   // mode 1 will route data to SMuFF instead to the PanelDue
                  smuffMode = true;
                  __debug("SMuFF mode\n"); 
                  break;
        default:  SerialPanelDue.write(in);
                  SerialPanelDue.write(nxt);
                  return;
      }
    }
    if(smuffMode) {
      if(in != 0)
        SerialSmuff.write(in);
    }
    else {
      SerialPanelDue.write(in);
      if(_config.btMirrorMode) {
        SerialBT.write(in);
      }
    }
    if(in == '\n' && smuffMode) {
      smuffMode = false;
      __debug("\nPanelDue mode\n"); 
    }
}

/**
 * Handles incoming characters from the PanelDue and passes them to the Duet3D.
 * 
 * @returns   Nothing.
 */
void handlePanelDueSerial() {
  char in = SerialPanelDue.read();
  dataCntPanelDue++;
  SerialDuet.write(in);
}

/**
 * Handles incoming characters from the SMuFF.
 * This routine scans the character stream for a escape sequence (starting with the ESC character 27 or 0x1b).
 * The escape sequences allow to set or reset a specific output pin on the ESP.
 * Those signal are used to trigger the endstop pins on the Duet3D, which are used to stop/continue the processing
 * of the current macro running on the Duet3D. 
 * If no escape sequence is present, the data received is passed on to the Duet3D
 * (which works only in Serial mode, not in I2C mode).
 * 
 * @returns   Nothing.
 */
void handleSmuffSerial() {
  char in = SerialSmuff.read();
  dataCntSmuff++;
  /*
    if the character sent is ESC (0x1b or 27) switch to CMD-Mode
    and read the following character to determine which port to switch
  */
  if(in == 0x1b) {
    cmdMode = true;
    return;
  }
  if(cmdMode && (in > 0 && in <10)) {
    switch(in) {
      case 1: pinSelected = SIGNAL1_PIN; break;
      case 2: pinSelected = SIGNAL2_PIN; break;
      case 3: pinSelected = SIGNAL3_PIN; break;
      case 4: pinSelected = LED_PIN; break;
    }
    //SerialDuet.printf("Pin selected: %d\n", pinSelected);
    return;
  }
  /*
    received '0' to switch pin off (LOW)
  */
  if(cmdMode && in == 0x30) {
    digitalWrite(pinSelected, LOW);
    cmdMode = false;
    return;
  }
  /*
    received '1' to switch pin on (HIGH)
  */
  if(cmdMode && in == 0x31) {
    digitalWrite(pinSelected, HIGH);
    cmdMode = false;
    return;
  }
  /*
    received '@' to blink LED (test mode)
    pinSelected doesn't mind here
  */
  if(cmdMode && in == 0x40) {
    cmdMode = false;
    for(int i=0; i< 5; i++) {
      blinkLED();
      delay(1000);
    }
    digitalWrite(LED_PIN, LOW);
    return;
  }
  /*
    otherwise... pass all data coming from SMuFF to the Duet3D board
  */
  SerialDuet.write(in);
}

/**
 * Handles incoming characters from the Bluetooth serial port (SPP).
 * In debug mode, this routine allows you to request the status of the ESP 
 * and list the contents of the SPIFFS.
 * In Bluetooth mirror mode it passes the data coming from the Bluetooth port to the Duet3D. 
 * 
 * @returns   Nothing.
 */
void handleBluetoothSerial() {
  char in = SerialBT.read();
  SerialDuet.write(in);
  if(!_config.btMirrorMode) {
    if(in == '1') {
      _config.i2cMode = !_config.i2cMode;
       __debug("I2C mode: %s\n", _config.i2cMode ? "Enabled" : "Disabled");
    }
    if(in == '2') {
      int n = WiFi.scanNetworks();
      for (int i = 0; i < n; ++i) {
        __debug(">>> %3d\t%s\t%d\t%s\n", i+1,  WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "*" : "");
      }  
    }
    if(in == '3') {
    }
    if(in == '4') {
      listDir(SPIFFS, "/");
    }
    if(in == '5') {
      __debug("Status\n---------------------------------\n");
      wifi_mode_t mode;
      esp_wifi_get_mode(&mode);
      WiFi.printDiag(SerialBT);
      if(mode != WIFI_MODE_NULL)
        __debug("IP-Address: %s\n", localIp.toString().c_str());
      __debug("Wifi %s connected\n", WiFi.status() == WL_CONNECTED ? "" : "not");
      __debug("Web %s server started\n", webServerRunning ? "" : "not");  
      __debug("mDNS %s started (%s.local)...\n", mdnsRunning ? "" : "not", _hostname.c_str());  
      __debug("Bluetooth mode: %s\n", _config.btMirrorMode ? "Mirror" : "Debug");
      __debug("I2C mode: %s\n", _config.i2cMode ? "Enabled" : "Disabled");
      __debug("\nTraffic:\n");
      __debug("\tSMuFF: %u bytes\n", dataCntSmuff);
      __debug("\tDuet3D: %u bytes\n", dataCntDuet);
      __debug("\tPanelDue: %u bytes\n", dataCntPanelDue);
      __debug("\tI2C: %u bytes\n", dataCntI2C);
      __debug("---------------------------------\n");
    }
  }
  else {
    if(!_config.i2cMode) {
      SerialDuet.write(in);
    }
  }
}

/**
 * Handles incoming bytes from the I2C/TWI port.
 * Send any byte received directly to the SMuFF without parsing or processing.
 * 
 * @param     bytes   The number of bytes in the buffer
 * @returns   Nothing.
 */
void handleI2C(int bytes)  {
  for(int i=0; i < bytes; i++) {
    __debug("%c", i2cBuffer[i]);
    SerialSmuff.write(i2cBuffer[i]);
    dataCntI2C++;
  }
}

/**
 * Method used to initialize the I2C in SLAVE mode as published by Espressif.
 * Be aware, that this method doesn't use the SDA/SCL pins normally used for the communication
 * in I2C master mode but the  I2C_SLAVE_SDA_IO, I2C_SLAVE_SCL_IO pins instead 
 * (see definition at the very start of this file).
 * Using the internal pullups should be suffienct. If not, consider turning them off and add 
 * external pullups instead.
 * 
 * @returns   The current initializing status.  
 */
static esp_err_t i2c_slave_init()
{
    i2c_port_t i2c_slave_port = I2C_NUM_0;
    i2c_config_t conf_slave;
    conf_slave.sda_io_num           = I2C_SLAVE_SDA_IO;
    conf_slave.sda_pullup_en        = _config.i2cPullup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    conf_slave.scl_io_num           = I2C_SLAVE_SCL_IO;
    conf_slave.scl_pullup_en        = _config.i2cPullup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    conf_slave.mode                 = I2C_MODE_SLAVE;
    conf_slave.slave.addr_10bit_en  = 0;
    conf_slave.slave.slave_addr     = _config.i2cAddress;
    
    i2c_param_config(i2c_slave_port, &conf_slave);
    return i2c_driver_install(i2c_slave_port, conf_slave.mode, I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
}

/**
 * Default Arduino setup routine.
 * Sets up the whole environment.
 * 
 * @returns   Nothing.
 */
void setup() {

  pinMode(LED_PIN, OUTPUT);
  pinMode(SIGNAL1_PIN, OUTPUT);
  pinMode(SIGNAL2_PIN, OUTPUT);
  pinMode(SIGNAL3_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(SIGNAL1_PIN, LOW);
  digitalWrite(SIGNAL2_PIN, LOW);
  digitalWrite(SIGNAL3_PIN, LOW);

  SerialDuet.begin(baudrate, SERIAL_8N1, RXD0, TXD0);
  SerialBT.begin(_hostname);

  SPIFFS.begin(true);
  readConfig();         // read the JSON config file from SPIFFS
  delay(1000);

  setupWiFi();          // setup everything
  setupOTA();
  setupWebServer();
  setupMdns();

  // initialize ports
  SerialSmuff.begin(baudrate, SERIAL_8N1, RXD1, TXD1);
  SerialPanelDue.begin(baudrate, SERIAL_8N1, RXD2, TXD2);
  i2c_slave_init();
}

/**
 * Default Arduino processing loop.
 * 
 * @returns Nothing.
 */
void loop() {
  if(SerialDuet.available()) {
    if(!_config.i2cMode) {
      handleDuetSerial();
    }
  }
  if(SerialPanelDue.available()) {
    if(!_config.i2cMode) {
      handlePanelDueSerial();
    }
  }
  if(SerialSmuff.available()) {
    handleSmuffSerial();
  }
  if(SerialBT.available()) {
    handleBluetoothSerial();
  }
  if(_config.i2cMode) {
    int size = i2c_slave_read_buffer(I2C_NUM_0, i2cBuffer, sizeof(i2cBuffer), 1000 / portTICK_RATE_MS);
    if(size > 0) {
      handleI2C(size);
    }
  }
}


