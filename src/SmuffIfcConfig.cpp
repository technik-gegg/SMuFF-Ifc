#include "SmuffIfcConfig.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>

SmuffIfcConfig _config;

/**
 * Method to read the interface settings from the JSON configuration file
 * using AdruinoJson library.
 * 
 * @returns   Nothing. Populates the _config structure though.
 */
void readConfig() {
    StaticJsonBuffer<512> jsonBuffer;
    
    File cfg = SPIFFS.open(CONFIG_FILE, "r");
    if (!cfg){
      __debug("Config file not found!\n");
    } 
    else {
      JsonObject& root = jsonBuffer.parseObject(cfg);
      if (!root.success()) {
        __debug("Config file possibly corrupted!\n");
      } 
      else {
        __debug("Configuration loaded\n");
        _config.isAP = root["AP-Mode"];
        _config.btMirrorMode = root["BT-Mirror-Mode"];
        _config.i2cMode = root["I2C-Mode"]["Enabled"];
        _config.i2cPullup = root["I2C-Mode"]["Pullup"];
        _config.i2cAddress = root["I2C-Mode"]["Address"];
        strlcpy(_config.ssid, root["STA-Mode"]["SSID"], sizeof(_config.ssid));
        strlcpy(_config.pwd, root["STA-Mode"]["Pwd"], sizeof(_config.pwd));
      }
      cfg.close();
    }
}

/**
 * Method to write the interface settings to the JSON configuration file
 * using AdruinoJson library.
 * 
 * @returns   Nothing.
 */  
void writeConfig() {
    StaticJsonBuffer<512> jsonBuffer;

    JsonObject& root = jsonBuffer.createObject();
    root["AP-Mode"] = _config.isAP;
    root["BT-Mirror-Mode"] = _config.btMirrorMode;
    
    JsonObject& i2cMode = root.createNestedObject("I2C-Mode");
    i2cMode["Enabled"] = _config.i2cMode;
    i2cMode["Pullup"] = _config.i2cPullup;
    i2cMode["Address"] = _config.i2cAddress;

    JsonObject& staMode = root.createNestedObject("STA-Mode");
    staMode["SSID"] = _config.ssid;
    staMode["Pwd"] = _config.pwd;
    File cfg = SPIFFS.open(CONFIG_FILE, "w");
    root.printTo(cfg);
    cfg.close();  
}


