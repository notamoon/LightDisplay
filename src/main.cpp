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
    Serial.begin(115200);

    renderer.setup();

    renderer.setInverted(conf::inverted);
    renderer.setBrightness(255);

    renderer.setFont(Fonts::FONT1);

    renderer.setColor(Color::YELLOW);
    renderer.scrollText("Setup", 0, 0, 1, -1, Direction::HORIZONTAL, 2);

    renderer.update();
    renderer.render();

    WiFi.mode(WIFI_STA);
    WiFi.begin(conf::networkSSID, conf::networkPassword);

    while (WiFi.status() != WL_CONNECTED) delay(500);

    Serial.println("Started");
}

unsigned long lastMessageRequest = 0;
unsigned long requestDelay = 10000;

string type = "";
int brightness;

struct simple {
    string message;
    int textColor[3];
    int borderColor[3];
    int speed;
};
simple simpleData;

struct twoline {
    string topMessage;
    int topTextColor[3];
    int topSpeed;

    string bottomMessage;
    int bottomTextColor[3];
    int bottomSpeed;

    int borderColor[3];
};
twoline twolineData;

struct image {
    int image[30][20] = {};
};
twoline image;

bool documentError = false;

int freeHeap = ESP.getFreeHeap();

void loop() {
    //int loopStart = millis();

    int currFreeHeap = ESP.getFreeHeap();

    if (freeHeap != currFreeHeap) {
        freeHeap = currFreeHeap;
        //Serial.println(freeHeap);
    }

    if (lastMessageRequest + requestDelay < millis()) {
        requestMessage();
        lastMessageRequest = millis();
    }

    renderer.clear();

    if (documentError) {
        renderer.setBrightness(255);

        if (renderer.clock() / 10 % 2 == 1) renderer.setColor(Color::YELLOW);
        else renderer.setColor(0);

        renderer.drawArea(0, 0, conf::displayWidth, conf::displayHeight);

        renderer.setFont(Fonts::FONT2);

        renderer.setColor(Color::RED);

        renderer.scrollText("Error", 1, 4, 2, 28, Direction::HORIZONTAL, 3);
    }
    else {
        renderer.setBrightness(brightness);

        if (type == "simple") {
            renderer.setColor(simpleData.borderColor[0], simpleData.borderColor[1], simpleData.borderColor[2]);

            renderer.drawArea(0, 0, conf::displayWidth, conf::displayHeight);

            renderer.setFont(Fonts::FONT2);

            renderer.setColor(simpleData.textColor[0], simpleData.textColor[1], simpleData.textColor[2]);

            renderer.scrollText(simpleData.message, 1, 4, 2, 28, Direction::HORIZONTAL, simpleData.speed);
        } else if (type == "twoline") {
            /*
            renderer.setBrightness(document["data"]["brightness"].as<int>());

            renderer.setColor(document["data"]["borderColor"][0].as<int>(), document["data"]["borderColor"][1].as<int>(),
                              document["data"]["borderColor"][2].as<int>());

            renderer.drawArea(0, 0, conf::displayWidth, conf::displayHeight);

            renderer.setFont(Fonts::FONT1);

            renderer.setColor(document["data"]["topTextColor"][0].as<int>(), document["data"]["topTextColor"][1].as<int>(),
                              document["data"]["topTextColor"][2].as<int>());

            renderer.scrollText(document["data"]["topMessage"].as<string>(), 1, 2, 2, 28, Direction::HORIZONTAL, document["data"]["topSpeed"].as<int>());

            renderer.setColor(document["data"]["bottomTextColor"][0].as<int>(),
                              document["data"]["bottomTextColor"][1].as<int>(),
                              document["data"]["bottomTextColor"][2].as<int>());

            int bottomSpeed = document["data"]["bottomScroll"].as<bool>() ? document["data"]["bottomSpeed"].as<int>() : 0;

            renderer.scrollText(document["data"]["bottomMessage"].as<string>(), 1, 2, 9, 28, Direction::HORIZONTAL,
                                bottomSpeed);
                                */
        } else if (type == "image") {
            /*
            renderer.setBrightness(document["data"]["brightness"].as<int>());

            for (auto x: document["data"]["image"].as<JsonArray>()) {
                for (auto y: x.as<JsonArray>()) {
                    renderer.setColor(y[0].as<int>(), y[1].as<int>(), y[2].as<int>());
                }
            }
             */
        } else {
            renderer.setFont(Fonts::FONT1);

            renderer.setColor(Color::RED);
            renderer.scrollText("Unknown Input Type", 0, 0, 1, -1, Direction::HORIZONTAL, 2);
        }
    }

    // Required
    renderer.update();
    renderer.render();

    Serial.println(ESP.getFreeHeap());

    //Serial.println(millis() - loopStart);
}

void requestMessage() {
    if (WiFi.status() != WL_CONNECTED) return;

    yield();

    Serial.println("Requesting message");

    WiFiClient client;

    HTTPClient http;

    if (!http.begin(client, "http://mvhscsf.ml:4567/api")) return;

    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {

        if (httpCode == HTTP_CODE_OK) {

            StaticJsonDocument<800> document;

            DeserializationError error = deserializeJson(document, http.getString());

            if (error) {
                documentError = true;
                document.clear();
                return;
            }

            type = document["type"].as<const char*>();
            brightness = document["data"]["brightness"].as<int>();

            if (type == "simple") {
                simpleData.message = document["data"]["message"].as<const char*>();;

                simpleData.textColor[0] = document["data"]["textColor"][0].as<int>();
                simpleData.textColor[1] = document["data"]["textColor"][1].as<int>();
                simpleData.textColor[2] = document["data"]["textColor"][2].as<int>();

                simpleData.borderColor[0] = document["data"]["borderColor"][0].as<int>();
                simpleData.borderColor[1] = document["data"]["borderColor"][1].as<int>();
                simpleData.borderColor[2] = document["data"]["borderColor"][2].as<int>();

                simpleData.speed = document["data"]["speed"].as<int>();
            }

            document.clear();

        }
    }

    http.end();
}