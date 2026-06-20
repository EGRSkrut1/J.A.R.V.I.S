#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 8
#define NUM_LEDS 1

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    Serial.begin(115200);
    strip.begin();
    strip.show();
    Serial.println("LED test started!");
}

void loop() {
    // Зеленый
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    strip.show();
    Serial.println("GREEN");
    delay(1000);
}