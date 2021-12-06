#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "ArduinoJson.h"
#include "Adafruit_NeoPixel.h"
#include "LightRenderer.h"
#include "fonts.h"

#include "conf.h"

Adafruit_NeoPixel strip(conf::displayWidth * conf::displayHeight, conf::dataPin, NEO_GRB + NEO_KHZ800);

LightRenderer renderer(strip, conf::displayWidth, conf::displayHeight);

void requestMessage();

void setup() {
    renderer.setup();

    renderer.setInverted(false);
    renderer.setBrightness(255);

    renderer.setFont(Fonts::FONT1);

    renderer.setColor(Color::YELLOW);
    renderer.scrollText("Setup", 0, 0, 1, -1, Direction::HORIZONTAL, 2);

    renderer.update();
    renderer.render();

    WiFi.mode(WIFI_STA);
    WiFi.begin(conf::networkSSID, conf::networkPassword);

    while (WiFi.status() != WL_CONNECTED) delay(500);
}

unsigned long lastMessageRequest = 0;
unsigned long requestDelay = 5000;

unsigned int color = 0;
string message;
string special;

void loop() {
    if (lastMessageRequest + requestDelay < millis()) {
        requestMessage();
        lastMessageRequest = millis();
    }

    renderer.clear();

    if (special == "random") {
        for (int x = 0; x < conf::displayWidth; x++) {
            for (int y = 0; y < conf::displayHeight; y++) {
                renderer.setColor(random(0, 256), random(0, 256), random(0, 256));
                renderer.drawPixel(x, y);
            }
        }
    }
    else if (special == "horizontal_rainbow") {
        double value = renderer.clock() * 8;
        double increment = 360.0 / conf::displayWidth;

        for (int x = 0; x < conf::displayWidth; x++) {
            for (int y = 0; y < conf::displayHeight; y++) {
                Color::rgb color = Color::hsv2rgb(Color::hsv{fmod(value, 360.0), 1.0, 1.0});

                renderer.setColor(color.r * 255, color.g * 255, color.b * 255);
                renderer.drawPixel(x, y);
            }

            value += increment;
        }
    }
    else if (special == "vertical_rainbow") {
        double value = renderer.clock() * 8;
        double increment = 360.0 / conf::displayHeight;

        for (int y = 0; y < conf::displayHeight; y++) {
            for (int x = 0; x < conf::displayWidth; x++) {
                Color::rgb color = Color::hsv2rgb(Color::hsv{fmod(value, 360.0), 1.0, 1.0});

                renderer.setColor(color.r * 255, color.g * 255, color.b * 255);
                renderer.drawPixel(x, y);
            }

            value += increment;
        }
    }
    else if (special == "rainbow") {
        double value = renderer.clock() * 4;

        for (int y = 0; y < conf::displayHeight; y++) {
            for (int x = 0; x < conf::displayWidth; x++) {
                Color::rgb color = Color::hsv2rgb(Color::hsv{fmod(value, 360.0), 1.0, 1.0});

                renderer.setColor(color.r * 255, color.g * 255, color.b * 255);
                renderer.drawPixel(x, y);
            }
        }

        renderer.setFont(Fonts::FONT2);

        renderer.setColor(0);
        renderer.scrollText(message, 0, 4, 2, -1, Direction::HORIZONTAL, 2);
    }
    else {
        renderer.setColor(Color::CYAN);

        renderer.drawArea(0, 0, 30, 20);

        renderer.setFont(Fonts::FONT2);

        renderer.setColor(color);
        renderer.scrollText(message, 1, 4, 2, 28, Direction::HORIZONTAL, 2);
    }

    // Required
    renderer.update();
    renderer.render();
}

void requestMessage() {
    if (WiFi.status() != WL_CONNECTED) return;

    WiFiClient client;

    HTTPClient http;

    if (!http.begin(client, "http://10.73.227.190:4567/api")) return;

    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {

        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();

            DynamicJsonDocument document(400);

            DeserializationError error = deserializeJson(document, payload);

            if (error) return;

            message = document["message"].as<const char*>();

            color = Adafruit_NeoPixel::Color(document["color"][0], document["color"][1], document["color"][2]);

            special = document["special"].as<const char*>();

            float d = document["brightness"].as<float>();
            renderer.setBrightness((int)(d * 255));
        }
    }

    http.end();
}