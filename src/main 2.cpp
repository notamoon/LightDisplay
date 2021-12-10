#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>

#include "ArduinoJson.h"
#include "Adafruit_NeoPixel.h"
#include "LightRenderer.h"
#include "fonts.h"

#include "conf.h"
#include "data.h"

Adafruit_NeoPixel strip(conf::displayWidth * conf::displayHeight, conf::dataPin, NEO_GRB + NEO_KHZ800);

LightRenderer renderer(strip, conf::displayWidth, conf::displayHeight);

ESP8266WiFiMulti WiFiMulti;

WebSocketsClient client;

void updateRendererData(uint8_t* &);

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_CONNECTED) {
        //Serial.printf("Connected to url: %s\n", payload);
        client.sendTXT("init");
    }
    else if (type == WStype_TEXT) {
        updateRendererData(payload);
    }
    else if (type == WStype_FRAGMENT_TEXT_START) {
        //Serial.println("FRAGMENT START");
    }
    else if (type == WStype_DISCONNECTED) {
        //Serial.println("DISCONNECTED");
    }
}

void clearRenderer(int color) {
    renderer.setColor(color);
    renderer.fillArea(0, 0, conf::displayWidth, conf::displayHeight);

    renderer.update();
    renderer.render();
}

void setup() {
    clearRenderer(Color::RED);

    // Serial Monitor
    // ======================================================================
    clearRenderer(Color::CYAN);

    //Serial.begin(115200);
    //Serial.println("Began Serial output");
    // ======================================================================

    // Light Renderer Setup
    // ======================================================================
    clearRenderer(Color::YELLOW);

    renderer.setup();

    renderer.setInverted(conf::inverted);
    renderer.setBrightness(255);

    renderer.update();
    renderer.render();

    //Serial.println("Setup renderer");
    // ======================================================================


    // Internet Connection
    // ======================================================================
    WiFiMulti.addAP(conf::networkSSID, conf::networkPassword);

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        //Serial.print(".");

        delay(50);
    }
    //Serial.println();

    //Serial.println("Connected to internet.");

    //Serial.print("Local IP Address: ");
    //Serial.println(WiFi.localIP());
    // ======================================================================


    // Websocket Connection
    // ======================================================================
    clearRenderer(Color::BLUE);

    client.begin("mvhscsf.ml", 4567, "/api");
    client.onEvent(webSocketEvent);
    client.setReconnectInterval(5000);
    client.enableHeartbeat(15000, 3000, 2);

    //Serial.println("Connected to websocket server.");
    // ======================================================================

    clearRenderer(Color::LIME);
}

String type;

SimpleRendererData simpleRendererData;
TwoLineRendererData twoLineRendererData;
ImageRendererData imageRendererData;

unsigned long long heapCheckCooldown = 5000;
unsigned long long lastHeapCheck = 0;

void updateRendererData(uint8_t* &payload) {
    //Serial.println("Deserializing data...");

    StaticJsonBuffer<2000> buffer;

    JsonObject& document = buffer.parseObject(payload);

    if (!document.success()) {
        //Serial.print("Encountered error deserializing display data.");

        type = "simple";
        simpleRendererData = {"Deserialization Error", {255, 0, 0}, {255, 255, 0}, 2};
        return;
    }

    if (document["type"] == "simple") {
        twoLineRendererData = {};
        imageRendererData = {};

        type = "simple";

        simpleRendererData.text = document["data"]["message"].as<String>();

        for (int i=0; i<3; i++) {
            simpleRendererData.textColor[i] = document["data"]["textColor"][i];
            simpleRendererData.borderColor[i] = document["data"]["borderColor"][i];
        }

        simpleRendererData.speed = document["data"]["speed"];

        simpleRendererData.brightness = document["data"]["brightness"];

        //Serial.println("Deserialized simple display data.");
    }
    else if (document["type"] == "twoline") {
        simpleRendererData = {};
        imageRendererData = {};

        type = "twoline";

        twoLineRendererData.bottomText = document["data"]["bottomMessage"].as<String>();

        for (int i=0; i<3; i++) {
            twoLineRendererData.bottomTextColor[i] = document["data"]["bottomTextColor"][i];
            twoLineRendererData.topTextColor[i] = document["data"]["topTextColor"][i];
            twoLineRendererData.borderColor[i] = document["data"]["borderColor"][i];
        }

        twoLineRendererData.bottomSpeed = document["data"]["bottomSpeed"];

        twoLineRendererData.topText = document["data"]["topMessage"].as<String>();

        twoLineRendererData.topSpeed = document["data"]["topSpeed"];

        twoLineRendererData.brightness = document["data"]["brightness"];

        //Serial.println("Deserialized twoline display data.");
    }
    else if (document["type"] == "image") {
        simpleRendererData = {};
        twoLineRendererData = {};

        type = "image";

        imageRendererData.image = matrix(conf::displayWidth, conf::displayHeight);

        for (auto x: document["data"]["image"].as<JsonArray>()) {
            for (auto y: x.as<JsonArray>()) {
                imageRendererData.image[x][y] = y;
            }
        }

        imageRendererData.brightness = document["data"]["brightness"];

        //Serial.println("Deserialized image display data.");
    }

    buffer.clear();
}

void loop() {
    client.loop();

    // Print current size of heap every 5 seconds
    // ======================================================================
    /*
    if (heapCheckCooldown + lastHeapCheck < millis()) {
        lastHeapCheck = millis();

        Serial.print("Current Free Heap Size: ");
        Serial.println(EspClass::getFreeHeap());
    }
    */
    // ======================================================================


    // One line of text
    // ======================================================================
    if (type == "simple") {
        renderer.setBrightness(simpleRendererData.brightness);

        renderer.clear();

        renderer.setColor(simpleRendererData.borderColor[0], simpleRendererData.borderColor[1], simpleRendererData.borderColor[2]);

        renderer.drawArea(0, 0, 30, 20);

        renderer.setFont(Fonts::FONT2);

        renderer.setColor(simpleRendererData.textColor[0], simpleRendererData.textColor[1], simpleRendererData.textColor[2]);
        renderer.scrollText(simpleRendererData.text, 2, 4, 2, 26, Direction::HORIZONTAL, simpleRendererData.speed);
    }
    // ======================================================================


    // Two lines of text
    // ======================================================================
    else if (type == "twoline") {
        renderer.setBrightness(twoLineRendererData.brightness);

        renderer.clear();

        renderer.setColor(twoLineRendererData.borderColor[0], twoLineRendererData.borderColor[1], twoLineRendererData.borderColor[2]);

        renderer.drawArea(0, 0, 30, 20);

        renderer.setFont(Fonts::FONT1);

        renderer.setColor(twoLineRendererData.bottomTextColor[0], twoLineRendererData.bottomTextColor[1], twoLineRendererData.bottomTextColor[2]);
        renderer.scrollText(twoLineRendererData.bottomText, 2, 3, 2, 26, Direction::HORIZONTAL, twoLineRendererData.bottomSpeed);

        renderer.setColor(twoLineRendererData.topTextColor[0], twoLineRendererData.topTextColor[1], twoLineRendererData.topTextColor[2]);
        renderer.scrollText(twoLineRendererData.topText, 2, 11, 2, 26, Direction::HORIZONTAL, twoLineRendererData.topSpeed);
    }
    // ======================================================================


    // Matrix of pixels
    // ======================================================================
    else if (type == "image") {
        renderer.setBrightness(imageRendererData.brightness);

        renderer.clear();

        for (int x = 0; x < conf::displayWidth; x++) {
            for (int y = 0; y < conf::displayHeight; y++) {
                renderer.setColor(imageRendererData.image[x][y]);
                renderer.drawPixel(x, y);
            }
        }
    }
    // ======================================================================


    // No data from server
    // ======================================================================
    else {
        renderer.setBrightness(255);

        renderer.clear();

        renderer.setColor(Color::YELLOW);

        renderer.drawArea(0, 0, 30, 20);

        renderer.setFont(Fonts::FONT2);

        renderer.setColor(Color::RED);
        renderer.scrollText("No Data", 2, 4, 2, 26, Direction::HORIZONTAL, 3);
    }
    // ======================================================================


    renderer.update();
    renderer.render();
}