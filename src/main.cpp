#include <Arduino.h>
#include "secrets.h"
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h>
#include <statusled.h>

#include "SinricPro.h"
#include "SinricProSwitch.h"

//secrets.h
//#define APP_KEY           "APP_KEY" 
//#define APP_SECRET        "APP_SECRET"
char    SWITCH_ID_1[64] = ""; //customParameter via WifiManager

WiFiManager wifiManager;
WiFiManagerParameter custom_SWITCH_ID_1("SWITCH_ID_1", "SWITCH_ID_1", SWITCH_ID_1, 60);

StatusLedManager slm;

bool onPowerState1(const String &deviceId, bool &state) {
 Serial.printf("\nDevice 1 turned %s", state?"on":"off");
 digitalWrite(4, state ? HIGH:LOW);
 digitalWrite(5, state ? HIGH:LOW);
 //digitalWrite(2, state ? LOW:HIGH);

 slm("ready").ledSetStill(state ? LOW:HIGH);

 return true; // request handled properly
}

// callback notifying us of the need to save config
void saveConfigCallback(){

  Serial.println("Saving new config");
  strcpy(SWITCH_ID_1, custom_SWITCH_ID_1.getValue());

  DynamicJsonDocument json(256);

  json["SWITCH_ID_1"] = SWITCH_ID_1;

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
  custom_SWITCH_ID_1.setValue(SWITCH_ID_1, 64); // set custom parameter value

  wifiManager.setSaveConfigCallback(saveConfigCallback); // set config save notify callback

  if (!wifiManager.autoConnect("25-8 Sinric")){
    Serial.println("Failed to connect and hit timeout");
  }
  else{
    Serial.println("Connected to WiFi.");
  }
}

// setup function for SinricPro
void setupSinricPro() {
  // add devices and callbacks to SinricPro
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  //pinMode(2, OUTPUT);
    
  SinricProSwitch& mySwitch1 = SinricPro[SWITCH_ID_1];
  mySwitch1.onPowerState(onPowerState1);  
  
  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
   
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// main setup function
void setup() {
  pinMode(D4, OUTPUT);
  slm.createStatusLed("ready", D4);
  slm("ready").ledSetBlink(0.25, 50);

  Serial.begin(115200); Serial.printf("\r\n\r\n");

  pinMode(D7, INPUT_PULLUP);

  if(!digitalRead(D7)){
    Serial.println("D7 Jumper detected, resetting");
    slm("ready").ledSetBlink(0.5, 50);
    wifiManager.erase();
  }
  
  setupWiFi();
  slm("ready").ledSetBlink(1, 50);

  setupSinricPro();
  slm("ready").ledSetBlink(2, 50);

  ArduinoOTA.begin();
}

void loop() {
  SinricPro.handle();
  ArduinoOTA.handle();
  slm.process(millis());

}
