#include <FS.h> 					       // needs to be first
#include <ESP8266WiFi.h>			   //ESP8266 Core WiFi Library

// for wifimanager
#include <DNSServer.h>				  //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>		//Local WebServer used to serve the configuration portal
#include <WiFiManager.h>   
#include <ArduinoJson.h> 

// MQTT
#include <PubSubClient.h>

// ++++++++++++++++++++++++++++++++
// Settings
// ++++++++++++++++++++++++++++++++

#define TESTING false									// set to true to reset all data

// Wlan settings
#define configSSID "LEDController"		// name of the AP used to configure the settings
#define configPW "password"           // password for the AP used to configure the settings

// MQTT settings
char* mqtt_client_name =  "HeadphoneController";
char* mqtt_server =       "192.168.2.105";
char* mqtt_port =         "1883";
char* mqtt_user =         "";
char* mqtt_pass =         "";


// ++++++++++++++++++++++++++++++++
// Don't change stuff beyond this point
// ++++++++++++++++++++++++++++++++
WiFiClient WiFiClient;
PubSubClient mqttClient(WiFiClient);

//flag for saving data
bool shouldSaveConfig = false;

void setup() {
	Serial.begin(115200);
  Serial.println();
  Serial.println("Starting setup");

  //clean FS, for testing
  #if TESTING
    SPIFFS.format();
  #endif

  initFileSystem();

  // start the wifi manager
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setTimeout(180);
  
  // add custom parameter of the format id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 20);
  WiFiManagerParameter custom_mqtt_pass("pass", "mqtt pass", mqtt_pass, 20);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);

  //reset settings - for testing
  #if TESTING
    wifiManager.resetSettings();
  #endif
  
  //goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(configSSID, configPW)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters ...
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_user, custom_mqtt_user.getValue());
  strcpy(mqtt_pass, custom_mqtt_pass.getValue());

  // ... and save the custom parameters to FS
  if (shouldSaveConfig) {
    saveConfigToFile();
  }

  // connected to wifi now
  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  // convert portnumber from string
  uint16_t portNumber = strtol(mqtt_port, NULL, 0);

  mqttClient.setServer(mqtt_server, portNumber);
  mqttClient.setCallback(mqttCallback);

  Serial.println("===== Setup done =====");
}

void loop() {
  // make sure mqtt is kept running
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  // content...
}

void initFileSystem() {
	//read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
  	Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_user, json["mqtt_user"]);
          strcpy(mqtt_pass, json["mqtt_pass"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
}

void saveConfigToFile() {
	Serial.println("saving config");
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["mqtt_server"] = mqtt_server;
  json["mqtt_port"] = mqtt_port;
  json["mqtt_user"] = mqtt_user;
  json["mqtt_pass"] = mqtt_pass;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // message received
    Serial.print("Got message from topic ");
    Serial.print(topic);
    Serial.print(" and content ");
    //Serial.println(&payload)
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (mqttClient.connect(mqtt_client_name, mqtt_server, mqtt_port)) {
      Serial.println("MQTT connected");
      startListeningToMQTTMessages();
      mqttClient.publish("HeadphoneController/status", "Started");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }  
  }
}

// TODO read first part of topics from name variable
void startListeningToMQTTMessages() {
  mqttClient.subscribe("HeadphoneController/color");
  mqttClient.subscribe("HeadphoneController/mode");
}
