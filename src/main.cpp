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

    Serial.print("Connecting to ");
    Serial.println(conf::networkSSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(conf::networkSSID, conf::networkPassword);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");

    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    renderer.setup();

    renderer.setInverted(false);

    renderer.setBrightness((int)(.25 * 255));

    Serial.begin(115200);
}

unsigned long lastMessageRequest = 0;
unsigned long requestDelay = 5000;

unsigned int color = 0;
string message;

void loop() {
    if (lastMessageRequest + requestDelay < millis()) {
        Serial.println("Sending message request.");

        requestMessage();
        lastMessageRequest = millis();
    }

    renderer.clear();

    renderer.setColor(Color::CYAN);

    renderer.drawArea(0, 0, 30, 20);

    renderer.setFont(Fonts::FONT2);

    renderer.setColor(color);
    renderer.scrollText(message, 1, 4, 2, 28, Direction::HORIZONTAL, 2);

    // Required
    renderer.update();
    renderer.render();
}

void requestMessage() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Unable to connect to server!");
        return;
    }

    WiFiClient client;

    HTTPClient http;

    Serial.println("Sending HTTP Request");

    if (!http.begin(client, conf::serverURL)) {
        Serial.println("WiFi is not connected.");
        return;
    }

    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {

        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();

            DynamicJsonDocument document(400);

            DeserializationError error = deserializeJson(document, payload);

            if (error) {
                Serial.print("Failed to deserialize JSON, error code: ");
                Serial.println(error.f_str());
                return;
            }

            //{"id":1,"message":"test","color":[255,255,255],"scroll":true,"speed":75,"displayWidth":20,"displayHeight":30,"brightness":0.25}

            message = document["message"].as<const char*>();

            color = Adafruit_NeoPixel::Color(document["color"][0], document["color"][1], document["color"][2]);

            float d = document["brightness"].as<float>();
            renderer.setBrightness((int)(d * 255));
        }
    } else Serial.printf("HTTP request failed: %s\n", HTTPClient::errorToString(httpCode).c_str());

    http.end();
}