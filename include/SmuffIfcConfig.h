
#ifndef CONFIG_H_
#define CONFIG_H_

#define CONFIG_FILE "/config.json"

#ifdef __cplusplus
extern "C" {
#endif

struct SmuffIfcConfig {
  bool isAP = true;
  char ssid[64];
  char pwd[64];
  bool btMirrorMode = false;
  bool i2cMode = false;
  bool i2cPullup = true;
  int  i2cAddress = 0x58;
};

void readConfig();
void writeConfig();

extern SmuffIfcConfig _config;
extern void __debug(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif