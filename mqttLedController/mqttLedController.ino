#include <FS.h>                           // needs to be first
#include <ESP8266WiFi.h>               //ESP8266 Core WiFi Library

// for wifimanager
#include <DNSServer.h>                  //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>        //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>
#include <ArduinoJson.h>

// MQTT
#include <PubSubClient.h>

// Neopixel
#include <Adafruit_NeoPixel.h>

// Animations
#include "Rainbow.h"

#ifdef __AVR__
#include <avr/power.h>
#endif

// ++++++++++++++++++++++++++++++++
// Settings
// ++++++++++++++++++++++++++++++++

#define TESTING false                                    // set to true to reset all data

// Wlan settings
#define configSSID  "LEDController"        // name of the AP used to configure the settings
#define configPW    "password"        // password for the AP used to configure the settings

// LED settings
#define NEO_PIN   2                   // NeoPixel DATA pin
#define NEO_PTYPE NEO_GRBW            // type of led
#define NUMPIXELS 33                  // Number of pixel in the strip
#define MAX_BRIGHTNESS  128           // max brightness (0-255)
int r = 0, g = 0, b = 0, w = 255;     // start led color (0-255)
int brightness = 100;                 // current brightness in percent 1-100
unsigned long patternInterval = 150;  // time between steps in the pattern, lower value means faster animations

// MQTT settings
char *mqtt_client_name = "HeadphoneController";  // used as name send to the mqtt client and as master topic
char *mqtt_server = "192.168.2.105";        // ip adress of the mqtt server
char *mqtt_port = "1883";                 // standard port is 1883
char *mqtt_user = "";                     // empty if no authentification is needed
char *mqtt_pass = "";                     // empty if no authentification is needed

// TODO split up code
// TODO set topics with clientname
// TODO set LWT

// ++++++++++++++++++++++++++++++++
// Don't change stuff beyond this point
// ++++++++++++++++++++++++++++++++
WiFiClient WiFiClient;
PubSubClient mqttClient(WiFiClient);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, NEO_PIN, NEO_PTYPE + NEO_KHZ800);
// animations
Rainbow rainbowAnimation = Rainbow(100, strip, 255);

char *colorTopic = "HeadphoneController/color";
char *modeTopic = "HeadphoneController/mode";
char *statusTopic = "HeadphoneController/status";
char *dimmerTopic = "HeadphoneController/dimmer";

unsigned long lastUpdate = 0;        // for millis() when last update occoured
int currentMode = 0;                  // display mode for led: 0 = set color, 1 = rainbwo, 2 ...

//flag for saving data
bool shouldSaveConfig = false;

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("Starting setup");

    // init leds
    strip.begin();
    strip.setBrightness(MAX_BRIGHTNESS); // set brightness
    strip.show(); // Initialize all pixels to 'off'

    blinkBlocking(); // blink 3 times to indicate startup
    blinkBlocking();
    blinkBlocking();

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

    blinkBlocking(); // setup done

    Serial.println("===== Setup done =====");
}

void loop() {
    // make sure mqtt is kept running
    if (!mqttClient.connected()) {
        reconnect();
    }

    // keep mqtt and wifi connection running
    mqttClient.loop();

    // keep led running
    //if (millis() - lastUpdate > patternInterval) {
    //    updatePattern(currentMode);
    //}
    rainbowAnimation.update();
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
                JsonObject &json = jsonBuffer.parseObject(buf.get());
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
    JsonObject &json = jsonBuffer.createObject();
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
void saveConfigCallback() {
    shouldSaveConfig = true;
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
    // message received
    Serial.print("Got message from topic ");
    Serial.print(topic);

    if (strcmp(topic, modeTopic) == 0) {
        Serial.println("Message is new mode");

        payload[length] = '\0';
        String newMode = String((char *) payload);

        currentMode = newMode.toInt();
        currentMode = currentMode % 6; // there are only 5 possible modes (0-4)
        if (currentMode < 0) {
            currentMode = 0;
        }
    }

    if (strcmp(topic, dimmerTopic) == 0) {
        Serial.print("Got new dimmer value ");
        payload[length] = '\0';
        String dimmerValue = String((char *) payload);
        Serial.println(dimmerValue);

        brightness = dimmerValue.toInt();
        brightness = brightness % 101;
        if (brightness < 0) {
            brightness = 0;
        }
    }

    if (strcmp(topic, colorTopic) == 0) {
        Serial.print("Incoming new color value ");
        payload[length] = '\0';
        String RGBWvalue = String((char *) payload);
        Serial.println(RGBWvalue);

        if (RGBWvalue.length() == 15) {
            // rrr,ggg,bbb,www
            // 0123456789....14

            if (RGBWvalue.charAt(3) == ',' && RGBWvalue.charAt(7) == ',' && RGBWvalue.charAt(11) == ',') {
                // split string at every "," and store in proper variable, value with error returns 0
                r = RGBWvalue.substring(0, 3).toInt();
                g = RGBWvalue.substring(4, 7).toInt();
                b = RGBWvalue.substring(8, 11).toInt();
                w = RGBWvalue.substring(12, 15).toInt();
                Serial.print("Got new color value ");
                Serial.print(r);
                Serial.print(" ");
                Serial.print(g);
                Serial.print(" ");
                Serial.print(b);
                Serial.print(" ");
                Serial.println(w);
            } else {
                Serial.println("Could not find delimiter in new color value");
            }
        } else {
            Serial.println("Got color value of unknown length");
        }
    }
}

void reconnect() {
    // Loop until we're reconnected
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");

        if (mqttClient.connect(mqtt_client_name, mqtt_server, mqtt_port)) {
            Serial.println("MQTT connected");
            startListeningToMQTTMessages();
            mqttClient.publish(statusTopic, "Started");
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
    mqttClient.subscribe(colorTopic);
    mqttClient.subscribe(modeTopic);
    mqttClient.subscribe(dimmerTopic);
}

// ++++++++++++++++++++++++++++++++
// LED control code
// ++++++++++++++++++++++++++++++++
void blinkBlocking() {
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 255));
    }
    strip.show();
    delay(500);
    wipe();
    strip.show();
    delay(500);
}

void updatePattern(int pat) { // call the pattern currently being created
    
}

void rainbow() { // modified from Adafruit example to make it a state machine
    static uint16_t j = 0;
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    j++;
    if (j >= 256) j = 0;
    lastUpdate = millis(); // time for next change to the display
}

void threeRingColorFade(uint32_t color) {
    static uint16_t j = 0;

    if (j >= 0 && j < 85) {
        colorRing(color, 1, 3);
        colorRing(strip.Color(0, 0, 0), 2, 3);
        colorRing(strip.Color(0, 0, 0), 3, 3);
    }
    if (j >= 85 && j < 170) {
        colorRing(strip.Color(0, 0, 0), 1, 3);
        colorRing(color, 2, 3);
        colorRing(strip.Color(0, 0, 0), 3, 3);
    }
    if (j >= 170 && j < 256) {
        colorRing(strip.Color(0, 0, 0), 1, 3);
        colorRing(strip.Color(0, 0, 0), 2, 3);
        colorRing(color, 3, 3);
    }

    strip.show();
    j++;
    if (j >= 256) j = 0;

    lastUpdate = millis(); // time for next change to the display
}

void rainbowCycle() { // modified from Adafruit example to make it a state machine
    static uint16_t j = 0;
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    j++;
    if (j >= 256 * 5) j = 0;
    lastUpdate = millis(); // time for next change to the display
}

void theaterChaseRainbow() { // modified from Adafruit example to make it a state machine
    static int j = 0, q = 0;
    static boolean on = true;
    if (on) {
        for (int i = 0; i < strip.numPixels(); i = i + 3) {
            strip.setPixelColor(i + q, Wheel((i + j) % 255));    //turn every third pixel on
        }
    } else {
        for (int i = 0; i < strip.numPixels(); i = i + 3) {
            strip.setPixelColor(i + q, 0);        //turn every third pixel off
        }
    }
    on = !on; // toggel pixelse on or off for next time
    strip.show(); // display
    q++; // update the q variable
    if (q >= 3) { // if it overflows reset it and update the J variable
        q = 0;
        j++;
        if (j >= 256) j = 0;
    }
    lastUpdate = millis(); // time for next change to the display
}

void colorWipe(uint32_t c) { // modified from Adafruit example to make it a state machine
    static int i = 0;
    strip.setPixelColor(i, c);
    strip.show();
    i++;
    if (i >= strip.numPixels()) {
        i = 0;
        wipe(); // blank out strip
    }
    lastUpdate = millis(); // time for next change to the display
}

void staticColor(uint32_t c) { // modified from Adafruit example to make it a state machine
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
    }
    strip.show();
    lastUpdate = millis(); // time for next change to the display
}

void wipe() { // clear all LEDs
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
}

uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
        int wheelR = 255 - WheelPos * 3, wheelG = 0, wheelB = WheelPos * 3;
        return getScaledColor(wheelR, wheelG, wheelB, 0);
    }
    if (WheelPos < 170) {
        WheelPos -= 85;
        int wheelR = 0, wheelG = WheelPos * 3, wheelB = 255 - WheelPos * 3;
        return getScaledColor(wheelR, wheelG, wheelB, 0);
    }
    WheelPos -= 170;
    int wheelR = WheelPos * 3, wheelG = 255 - WheelPos * 3, wheelB = 0;
    return getScaledColor(wheelR, wheelG, wheelB, 0);
}

void colorRing(uint32_t color, int ring, int numberOfRings) {
    int ringSize = strip.numPixels() / numberOfRings;
    int startPixel = (ring - 1) * ringSize;
    for (int i = startPixel; i < startPixel + ringSize - 1; i++) {
        strip.setPixelColor(i, color);
    }
}

uint32_t getScaledColor(int newR, int newG, int newB, int newW) {
    return strip.Color(
            newR / 100 * brightness,
            newG / 100 * brightness,
            newB / 100 * brightness,
            newW / 100 * brightness
    );
}
