// Your firmware version. Must be above SinricPro.h. Do not rename this.

// 0.1.1 Single switch
// 0.2.1 plus ota
// 0.3.1 multiple switch

#define FIRMWARE_VERSION "0.3.1"  

// Sketch -> Export Compiled Binary to export

#ifdef ENABLE_DEBUG
  #define DEBUG_ESP_PORT Serial
  #define NODEBUG_WEBSOCKETS
  #define NDEBUG
#endif

#include <Arduino.h>
#include "secrets.h"
#include <LittleFS.h>

#include <WiFiManager.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266OTAHelper.h>
#include <statusled.h>

#include "SemVer.h"
#include "SinricPro.h"
#include "SinricProSwitch.h"

const byte PIN_SWITCH_ID_1 = 5;  //D1
const byte PIN_SWITCH_ID_2 = 4;  //D2
const byte PIN_SWITCH_ID_3 = 14; //D5
const byte PIN_SWITCH_ID_4 = 12; //D6
const byte PIN_SWITCH_ID_5 = 13; //D7

//secrets.h
//#define APP_KEY           "APP_KEY" 
//#define APP_SECRET        "APP_SECRET"

//customParameter via WifiManager
char    SWITCH_ID_1[64] = ""; 
char    SWITCH_ID_2[64] = "";
char    SWITCH_ID_3[64] = ""; 
char    SWITCH_ID_4[64] = "";
char    SWITCH_ID_5[64] = "";

WiFiManager wifiManager;
WiFiManagerParameter custom_SWITCH_ID_1("SWITCH_ID_1", "SWITCH_ID_1", SWITCH_ID_1, 60);
WiFiManagerParameter custom_SWITCH_ID_2("SWITCH_ID_2", "SWITCH_ID_2", SWITCH_ID_2, 60);
WiFiManagerParameter custom_SWITCH_ID_3("SWITCH_ID_3", "SWITCH_ID_3", SWITCH_ID_3, 60);
WiFiManagerParameter custom_SWITCH_ID_4("SWITCH_ID_4", "SWITCH_ID_4", SWITCH_ID_4, 60);
WiFiManagerParameter custom_SWITCH_ID_5("SWITCH_ID_5", "SWITCH_ID_5", SWITCH_ID_5, 60);

StatusLedManager slm;

bool handleOTAUpdate(const String& url, int major, int minor, int patch, bool forceUpdate) {
  Version currentVersion  = Version(FIRMWARE_VERSION);
  Version newVersion      = Version(String(major) + "." + String(minor) + "." + String(patch));
  bool updateAvailable    = newVersion > currentVersion;

  Serial.print("URL: ");
  Serial.println(url.c_str());
  Serial.print("Current version: ");
  Serial.println(currentVersion.toString());
  Serial.print("New version: ");
  Serial.println(newVersion.toString());
  if (forceUpdate) Serial.println("Enforcing OTA update!");

  // Handle OTA update based on forceUpdate flag and update availability
  if (forceUpdate || updateAvailable) {
    if (updateAvailable) {
      Serial.println("Update available!");
    }

    String result = startOtaUpdate(url);
    if (!result.isEmpty()) {
      SinricPro.setResponseMessage(std::move(result));
      return false;
    } 
    return true;
  } else {
    String result = "Current version is up to date.";
    SinricPro.setResponseMessage(std::move(result));
    Serial.println(result);
    return false;
  }
}

bool onPowerState1(const String &deviceId, bool &state) {
 Serial.printf("\nDevice 1 turned %s", state?"on":"off");
 digitalWrite(PIN_SWITCH_ID_1, state ? HIGH:LOW);
 slm("ready").ledSetStill(state ? LOW:HIGH);
 return true; 
}

bool onPowerState2(const String &deviceId, bool &state) {
 Serial.printf("\nDevice 2 turned %s", state?"on":"off");
 digitalWrite(PIN_SWITCH_ID_2, state ? HIGH:LOW);
 slm("ready").ledSetStill(state ? LOW:HIGH);
 return true; 
}

bool onPowerState3(const String &deviceId, bool &state) {
 Serial.printf("\nDevice 3 turned %s", state?"on":"off");
 digitalWrite(PIN_SWITCH_ID_3, state ? HIGH:LOW);
 slm("ready").ledSetStill(state ? LOW:HIGH);
 return true; 
}

bool onPowerState4(const String &deviceId, bool &state) {
 Serial.printf("\nDevice 4 turned %s", state?"on":"off");
 digitalWrite(PIN_SWITCH_ID_4, state ? HIGH:LOW);
 slm("ready").ledSetStill(state ? LOW:HIGH);
 return true; 
}

bool onPowerState5(const String &deviceId, bool &state) {
 Serial.printf("\nDevice 5 turned %s", state?"on":"off");
 digitalWrite(PIN_SWITCH_ID_5, state ? HIGH:LOW);
 slm("ready").ledSetStill(state ? LOW:HIGH);
 return true; 
}

// callback notifying us of the need to save config
void saveConfigCallback(){

  Serial.println("Saving new config");
  strcpy(SWITCH_ID_1, custom_SWITCH_ID_1.getValue());
  strcpy(SWITCH_ID_2, custom_SWITCH_ID_2.getValue());
  strcpy(SWITCH_ID_3, custom_SWITCH_ID_3.getValue());
  strcpy(SWITCH_ID_4, custom_SWITCH_ID_4.getValue());
  strcpy(SWITCH_ID_5, custom_SWITCH_ID_5.getValue());

  DynamicJsonDocument json(256);

  json["SWITCH_ID_1"] = SWITCH_ID_1;
  json["SWITCH_ID_2"] = SWITCH_ID_2;
  json["SWITCH_ID_3"] = SWITCH_ID_3;
  json["SWITCH_ID_4"] = SWITCH_ID_4;
  json["SWITCH_ID_5"] = SWITCH_ID_5;

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile)
  {
    Serial.println("Failed to open config file for writing");
  }

  serializeJson(json, Serial);
  serializeJson(json, configFile);
  configFile.close();
}

void readParamsFromFS(){
  if (LittleFS.begin()){
    if (LittleFS.exists("/config.json")){
      // file exists, reading and loading
      Serial.println("Reading config file");

      File configFile = LittleFS.open("/config.json", "r");
      if (configFile){
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

        DynamicJsonDocument json(256);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        Serial.println();

        if (!deserializeError){
          if (json.containsKey("SWITCH_ID_1"))
            strcpy(SWITCH_ID_1, json["SWITCH_ID_1"]);
          if (json.containsKey("SWITCH_ID_2"))
            strcpy(SWITCH_ID_2, json["SWITCH_ID_2"]);
          if (json.containsKey("SWITCH_ID_3"))
            strcpy(SWITCH_ID_3, json["SWITCH_ID_3"]);
          if (json.containsKey("SWITCH_ID_4"))
            strcpy(SWITCH_ID_4, json["SWITCH_ID_4"]);
          if (json.containsKey("SWITCH_ID_5"))
            strcpy(SWITCH_ID_5, json["SWITCH_ID_5"]);
        }
        else{
          Serial.println("Failed to load json config");
        }

        configFile.close();
      }
    }

    else{
      Serial.println("Failed to mount FS");
    }
  }
}


// setup function for WiFi connection
void setupWiFi() {
  WiFi.begin();

  readParamsFromFS(); // get parameters from file system
  wifiManager.setClass("invert");          // enable "dark mode" for the config portal
  wifiManager.setConfigPortalTimeout(120); // auto close configportal after n seconds
  wifiManager.setAPClientCheck(true);      // avoid timeout if client connected to softap

  wifiManager.addParameter(&custom_SWITCH_ID_1);      // set custom parameter for IO key
  wifiManager.addParameter(&custom_SWITCH_ID_2);      // set custom parameter for IO key
  wifiManager.addParameter(&custom_SWITCH_ID_3);      // set custom parameter for IO key
  wifiManager.addParameter(&custom_SWITCH_ID_4);      // set custom parameter for IO key
  wifiManager.addParameter(&custom_SWITCH_ID_5);      // set custom parameter for IO key
  custom_SWITCH_ID_1.setValue(SWITCH_ID_1, 64); // set custom parameter value
  custom_SWITCH_ID_2.setValue(SWITCH_ID_2, 64); // set custom parameter value
  custom_SWITCH_ID_3.setValue(SWITCH_ID_3, 64); // set custom parameter value
  custom_SWITCH_ID_4.setValue(SWITCH_ID_4, 64); // set custom parameter value
  custom_SWITCH_ID_5.setValue(SWITCH_ID_5, 64); // set custom parameter value

  wifiManager.setSaveConfigCallback(saveConfigCallback); // set config save notify callback

  if (!wifiManager.autoConnect("Sinric")){
    Serial.println("Failed to connect and hit timeout");
  }
  else{
    Serial.println("Connected to WiFi.");
  }
}

// setup function for SinricPro
void setupSinricPro() {
  // add devices and callbacks to SinricPro
  pinMode(PIN_SWITCH_ID_1, OUTPUT);
  pinMode(PIN_SWITCH_ID_2, OUTPUT);
  pinMode(PIN_SWITCH_ID_3, OUTPUT);
  pinMode(PIN_SWITCH_ID_4, OUTPUT);
  pinMode(PIN_SWITCH_ID_5, OUTPUT);
    
  SinricProSwitch& mySwitch1 = SinricPro[SWITCH_ID_1];
  SinricProSwitch& mySwitch2 = SinricPro[SWITCH_ID_2];
  SinricProSwitch& mySwitch3 = SinricPro[SWITCH_ID_3];
  SinricProSwitch& mySwitch4 = SinricPro[SWITCH_ID_4];
  SinricProSwitch& mySwitch5 = SinricPro[SWITCH_ID_5];

  mySwitch1.onPowerState(onPowerState1);  
  mySwitch2.onPowerState(onPowerState2);  
  mySwitch3.onPowerState(onPowerState3);  
  mySwitch4.onPowerState(onPowerState4);  
  mySwitch5.onPowerState(onPowerState5);  
  
  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.onOTAUpdate(handleOTAUpdate);  
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// main setup function
void setup() {

  //Status LED
  pinMode(D4, OUTPUT);
  Serial.begin(115200); Serial.printf("\r\n\r\n");

  //RESET jumper
  pinMode(D7, INPUT_PULLUP);

  if(!digitalRead(D7)){
    Serial.println("D7 Jumper detected, resetting");
    wifiManager.erase();
  }
  
  setupWiFi(); 
  setupSinricPro();

  slm.createStatusLed("ready", D4);
  slm("ready").ledSetBlink(2, 50);
}

void loop() {
  SinricPro.handle();
  slm.process(millis());
}