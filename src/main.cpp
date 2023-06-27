#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"

std::unordered_map<int, bool> pinMap;

#define D1_PIN 5 //Decke1
#define D2_PIN 4 //Decke2
#define D3_PIN 0 //Hängeschrank
#define D4_PIN 2 //Außenlicht
#define D5_PIN 14 //Heizstab1
#define D6_PIN 12 //Heizstab2
#define D7_PIN 13 //frei
#define D8_PIN 15 //frei

IPAddress ipv4(192, 168, 1, 180);
IPAddress standardGateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(1, 1, 1, 1);
IPAddress secondaryDNS(8, 8, 8, 8);

AsyncWebServer server(80);


void connectToWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin("Crafti-WLAN", "BS-IB96:-)");
    Serial.println("\nConnecting to WiFi ");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected!");
    } else {
        Serial.println("Connection timeout!");
    }
}

void initFS(){
    if (!LittleFS.begin()) {
        Serial.println("An error...");
    }
    Serial.println("LittleFS mounted successfully lol");
}


void handleGetAllDataRequest(AsyncWebServerRequest *request) {

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
    valueArray.add(pinMap[D1_PIN]);
    valueArray.add(pinMap[D2_PIN]);
    valueArray.add(pinMap[D3_PIN]);
    valueArray.add(pinMap[D4_PIN]);
    valueArray.add(pinMap[D5_PIN]);
    valueArray.add(pinMap[D6_PIN]);
    valueArray.add(pinMap[D7_PIN]);
    valueArray.add(pinMap[D8_PIN]);

    Serial.println(jsonDocument.memoryUsage());
    String jsonString;
    serializeJson(jsonDocument, jsonString);



    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonString);
    request->send(response);
}

void handleAllOff(AsyncWebServerRequest( *request)){

    for (auto it = pinMap.begin(); it != pinMap.end(); ++it) {
        int key = it->first;
        pinMap[key] = false;
        digitalWrite(key, LOW);
    }


    request->send(200, "text/plain", "ist geschehen");
}

void handleHTTPRequest(AsyncWebServerRequest *request) {

    StaticJsonDocument<50> jsonDocument;
    DeserializationError error = deserializeJson(jsonDocument, request->arg("data"));

    if (error) {
        Serial.print("parsing JSON failed: ");
        Serial.println(error.c_str());
        return;
    }

    int key = jsonDocument["key"];

    pinMap[key] = !pinMap[key];
    if (pinMap[key]) digitalWrite(key, HIGH);
    else digitalWrite(key, LOW);


    request->send(200, "text/plain", "Finished");
}

void setup() {
    pinMap[D1_PIN] = false;
    pinMap[D2_PIN] = false;
    pinMap[D3_PIN] = false;
    pinMap[D4_PIN] = false;
    pinMap[D5_PIN] = false;
    pinMap[D6_PIN] = false;
    pinMap[D7_PIN] = false;
    pinMap[D8_PIN] = false;


    // setup globals
    pinMode(D1_PIN, OUTPUT);
    pinMode(D2_PIN, OUTPUT);
    pinMode(D3_PIN, OUTPUT);
    pinMode(D4_PIN, OUTPUT);
    pinMode(D5_PIN, OUTPUT);
    pinMode(D6_PIN, OUTPUT);
    pinMode(D7_PIN, OUTPUT);
    pinMode(D8_PIN, OUTPUT);

    digitalWrite(D1_PIN, LOW);
    digitalWrite(D2_PIN, LOW);
    digitalWrite(D3_PIN, LOW);
    digitalWrite(D4_PIN, LOW);
    digitalWrite(D5_PIN, LOW);
    digitalWrite(D6_PIN, LOW);
    digitalWrite(D7_PIN, LOW);
    digitalWrite(D8_PIN, LOW);

    if (!WiFi.config(ipv4, standardGateway, subnet, primaryDNS, secondaryDNS)) Serial.println("ging nicht");


    Serial.begin(115200); // start serial
    initFS();



    connectToWiFi();




    //server static html
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "index.html", "text/html");
    });

    //Serve static files (css, js)
    server.serveStatic("/", LittleFS, "/");


    //handle all incoming httprequests on /data
    server.on("/data", handleHTTPRequest);

    server.on("/getAllData", handleGetAllDataRequest);

    server.on("/allOff", handleAllOff);

    server.begin();
    Serial.println("Webserver started!");


    Serial.println(WiFi.localIP());
}


void loop() {
    delay(1000);
    if (WiFi.status() != WL_CONNECTED) connectToWiFi(); // if not connected to wifi, reconnect
}