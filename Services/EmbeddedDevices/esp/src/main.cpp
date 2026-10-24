#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "config_loader.h"

#define LED_PIN 8
#define NUM_LEDS 1

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
WebServer server(80);

String ssid = "";
String password = "";
int treePin = 9;
String SERVER_IP = "192.168.1.39";

int currentMode = 0; 
int targetMode = 0; 
bool isProcessing = false;
unsigned long lastClickTime = 0;
int clickState = 0;

unsigned long lastPingTime = 0;
unsigned long lastHeapCheckTime = 0;

void setLed(uint8_t r, uint8_t g, uint8_t b) {
    strip.setPixelColor(0, strip.Color(r, g, b));
    strip.show();
}

void logSystemStatus() {
    Serial.print("[HEAP] Free: ");
    Serial.print(ESP.getFreeHeap());
    Serial.print(" bytes | Wi-Fi RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
}

void pingServer() {
    if (WiFi.status() != WL_CONNECTED) return;
    HTTPClient http;
    String url = "http://" + SERVER_IP + ":8080/status";
    http.begin(url);
    http.setTimeout(2000);
    int code = http.GET();
    if (code > 0) {
        Serial.println("[PING] Server OK. Code: " + String(code));
    } else {
        Serial.println("[PING] Server unreachable. Error: " + String(code));
    }
    http.end();
}

void startModeChange(int target) {
    targetMode = target;
    isProcessing = true;
    lastClickTime = millis();
    clickState = 0;
}

void handleOn() {
    Serial.println("[HTTP] Request /tree/on from " + server.client().remoteIP().toString());
    startModeChange(1);
    server.send(200, "application/json", "{\"status\":\"on\"}");
    logSystemStatus();
}

void handleOff() {
    Serial.println("[HTTP] Request /tree/off from " + server.client().remoteIP().toString());
    startModeChange(0);
    server.send(200, "application/json", "{\"status\":\"off\"}");
    logSystemStatus();
}

void processTreeClicks() {
    if (!isProcessing) return;
    if (currentMode == targetMode) {
        isProcessing = false;
        Serial.println("[TREE] Target mode reached: " + String(currentMode));
        return;
    }
    unsigned long now = millis();
    if (clickState == 0) {
        digitalWrite(treePin, HIGH);
        lastClickTime = now;
        clickState = 1;
    } 
    else if (clickState == 1) {
        if (now - lastClickTime > 1000) {
            digitalWrite(treePin, LOW);
            currentMode = (currentMode + 1) % 3;
            Serial.println("[TREE] Switched to: " + String(currentMode));
            lastClickTime = now;
            clickState = 2;
        }
    } 
    else if (clickState == 2) {
        if (now - lastClickTime > 1000) {
            clickState = 0;
        }
    }
}

void parseLocalConfig() {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, CONFIG_JSON_RAW);
    if (!error) {
        ssid = doc["ssid"].as<String>();
        password = doc["password"].as<String>();
        treePin = doc["tree_pin"] | 9;
        SERVER_IP = doc["server_ip"] | "192.168.1.39";
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n--- JARVIS ESP BOOT ---");
    logSystemStatus();

    strip.begin();
    setLed(0, 0, 50);
    parseLocalConfig();

    pinMode(treePin, OUTPUT);
    digitalWrite(treePin, LOW);

    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to WiFi: " + ssid);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\n[WiFi] Connected! IP: " + WiFi.localIP().toString());
    WiFi.setSleep(false);
    setLed(0, 50, 0);

    server.on("/tree/on", HTTP_GET, handleOn);
    server.on("/tree/off", HTTP_GET, handleOff);
    server.begin();
    Serial.println("[HTTP] Server started on port 80.");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi] LOST CONNECTION! Rebooting in 5s...");
        setLed(50, 0, 0);
        delay(5000);
        ESP.restart();
    }

    unsigned long now = millis();

    if (now - lastPingTime > 10000) {
        lastPingTime = now;
        pingServer();
    }

    if (now - lastHeapCheckTime > 30000) {
        lastHeapCheckTime = now;
        logSystemStatus();
    }

    server.handleClient();
    processTreeClicks();
}