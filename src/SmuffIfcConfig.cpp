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
    //StaticJsonBuffer<512> jsonBuffer;
    StaticJsonDocument<512> jsonDoc;
    
    File cfg = SPIFFS.open(CONFIG_FILE, "r");
    if (!cfg){
      __debug("Config file not found!\n");
    } 
    else {
      //JsonObject& root = jsonBuffer.parseObject(cfg);
      auto error = deserializeJson(jsonDoc, cfg);
      //if (!root.success()) {
      if (error) {
        __debug("Config file possibly corrupted!\n");
      } 
      else {
        __debug("Configuration loaded\n");
        _config.isAP = jsonDoc["AP-Mode"];
        _config.btMirrorMode = jsonDoc["BT-Mirror-Mode"];
        _config.i2cMode = jsonDoc["I2C-Mode"]["Enabled"];
        _config.i2cPullup = jsonDoc["I2C-Mode"]["Pullup"];
        _config.i2cAddress = jsonDoc["I2C-Mode"]["Address"];
        strlcpy(_config.ssid, jsonDoc["STA-Mode"]["SSID"], sizeof(_config.ssid));
        strlcpy(_config.pwd, jsonDoc["STA-Mode"]["Pwd"], sizeof(_config.pwd));
        _config.useNextion = jsonDoc["UseNextion"];
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
    //StaticJsonBuffer<512> jsonBuffer;
    StaticJsonDocument<512> jsonDoc;
    JsonObject jsonObj = jsonDoc.to<JsonObject>();

    //JsonObject& root = jsonBuffer.createObject();
    jsonDoc["AP-Mode"] = _config.isAP;
    jsonDoc["BT-Mirror-Mode"] = _config.btMirrorMode;
    jsonDoc["UseNextion"] = _config.useNextion;

    JsonObject i2cMode = jsonObj.createNestedObject("I2C-Mode");
    i2cMode["Enabled"] = _config.i2cMode;
    i2cMode["Pullup"] = _config.i2cPullup;
    i2cMode["Address"] = _config.i2cAddress;

    JsonObject staMode = jsonObj.createNestedObject("STA-Mode");
    staMode["SSID"] = _config.ssid;
    staMode["Pwd"] = _config.pwd;
    File cfg = SPIFFS.open(CONFIG_FILE, "w");
    serializeJsonPretty(jsonDoc, cfg);
    //root.printTo(cfg);
    cfg.close();  
}


