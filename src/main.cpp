#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
using namespace std;


#define D1_PIN 5 //Decke1
#define D2_PIN 4 //Decke2
#define D3_PIN 0 //Hängeschrank
#define D4_PIN 2 //Außenlicht
#define D5_PIN 14 //Heizstab1
#define D6_PIN 12 //Heizstab2
#define D7_PIN 13 //frei
#define D8_PIN 15 //frei

unordered_map<int, bool> pinStateMap;

AsyncWebServer server(80);


void setupPins(){
    pinMode(D1_PIN, OUTPUT);
    pinMode(D2_PIN, OUTPUT);
    pinMode(D3_PIN, OUTPUT);
    pinMode(D4_PIN, OUTPUT);
    pinMode(D5_PIN, OUTPUT);
    pinMode(D6_PIN, OUTPUT);
    pinMode(D7_PIN, OUTPUT);
    pinMode(D8_PIN, OUTPUT);

    pinStateMap[D1_PIN] = false;
    pinStateMap[D2_PIN] = false;
    pinStateMap[D3_PIN] = false;
    pinStateMap[D4_PIN] = false;
    pinStateMap[D5_PIN] = false;
    pinStateMap[D6_PIN] = false;
    pinStateMap[D7_PIN] = false;
    pinStateMap[D8_PIN] = false;
}


void setupIPConfig(){
    IPAddress ipv4(192, 168, 1, 180);
    IPAddress standardGateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress primaryDNS(1, 1, 1, 1);
    IPAddress secondaryDNS(8, 8, 8, 8);

    if (!WiFi.config(ipv4, standardGateway, subnet, primaryDNS, secondaryDNS)) {
        Serial.println("[WARN] IP Adresse konnte nicht konfiguriert werden.");
        return;
    }

    Serial.println("[SETUP] IP Adresse erfolgreich konfiguriert.");
}


void setupFilesystem(){
    if (!LittleFS.begin()) {
        Serial.println("[WARN] Filesystem konnte nicht erstellt werden.");
        return;
    }

    Serial.println("[SETUP] Filesystem erfolgreich erstellt.");
}


void connectToWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin("Crafti-WLAN", "BS-IB96:-)");
    Serial.println("[SETUP] Verbindung zum WLAN wird aufgebaut.");

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WARN] Verbindung zum WLAN fehlgeschlagen.");
        return;
    }

    Serial.println("[SETUP]  Verbindung zum WLAN hergestellt.");
}


void handleHttpDataRequest(AsyncWebServerRequest *request) {

    StaticJsonDocument<50> jsonDocument;
    DeserializationError error = deserializeJson(jsonDocument, request->arg("data"));

    if (error) {
        Serial.println("[WARN] JSON PARSING FAILED.");
        Serial.println(String("[WARN] ERROR: ").concat(error.c_str()));
        return;
    }

    int key = jsonDocument["key"];
    pinStateMap[key] = !pinStateMap[key];

    if (pinStateMap[key]) digitalWrite(key, HIGH);
    else digitalWrite(key, LOW);

    request->send(200, "text/plain", "[DEBUG] Request 'UpdatePinState' handled successfully.");
}


void handleHttpAllDataRequest(AsyncWebServerRequest *request) {
    DynamicJsonDocument jsonDocument(300);

    JsonArray keyArray = jsonDocument.createNestedArray("keys");
    keyArray.add(D1_PIN);
    keyArray.add(D2_PIN);
    keyArray.add(D3_PIN);
    keyArray.add(D4_PIN);
    keyArray.add(D5_PIN);
    keyArray.add(D6_PIN);
    keyArray.add(D7_PIN);
    keyArray.add(D8_PIN);

    JsonArray valueArray = jsonDocument.createNestedArray("values");
    valueArray.add(pinStateMap[D1_PIN]);
    valueArray.add(pinStateMap[D2_PIN]);
    valueArray.add(pinStateMap[D3_PIN]);
    valueArray.add(pinStateMap[D4_PIN]);
    valueArray.add(pinStateMap[D5_PIN]);
    valueArray.add(pinStateMap[D6_PIN]);
    valueArray.add(pinStateMap[D7_PIN]);
    valueArray.add(pinStateMap[D8_PIN]);

    Serial.println(jsonDocument.memoryUsage()); //TODO remove
    String jsonString;
    serializeJson(jsonDocument, jsonString);

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonString);
    request->send(response);
}


void handleHttpAllOffRequest(AsyncWebServerRequest(*request)){

    for (auto it = pinStateMap.begin(); it != pinStateMap.end(); ++it) {
        int key = it->first;
        pinStateMap[key] = false;
        digitalWrite(key, LOW);
    }

    request->send(200, "text/plain", "[DEBUG] Request 'AllOff' handled successfully.");
}


void setupWebservers(){
    //Server main Website (html)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "index.html", "text/html");
    });

    //Serve static files (css, js)
    server.serveStatic("/", LittleFS, "/");

    //subpages for http requests
    server.on("/data", handleHttpDataRequest);
    server.on("/getAllData", handleHttpAllDataRequest);
    server.on("/allOff", handleHttpAllOffRequest);

    server.begin();
    Serial.println("[SETUP] Webserver gestartet.");
}


void setup() {
    Serial.begin(115200);
    setupPins();
    setupIPConfig();
    setupFilesystem();
    connectToWiFi();
    setupWebservers();
}


void loop() {
    delay(10000);
    if (WiFi.status() != WL_CONNECTED) connectToWiFi();
}