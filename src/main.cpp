#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WebSocketsClient.h>
#include <Hash.h>

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

    switch(type) {
        case WStype_CONNECTED:
            Serial.printf("Connected to url: %s\n", payload);
            client.sendTXT("init");
            break;

        case WStype_TEXT: {
            updateRendererData(payload);

            break;
        }
        default:
            break;
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

    delay(5000);

    // Serial Monitor
    // ======================================================================
    clearRenderer(Color::CYAN);

    Serial.begin(115200);
    Serial.println("Began Serial output");
    // ======================================================================

    // Light Renderer Setup
    // ======================================================================
    clearRenderer(Color::YELLOW);

    renderer.setup();

    renderer.setInverted(conf::inverted);
    renderer.setBrightness(255);

    renderer.update();
    renderer.render();

    Serial.println("Setup renderer");
    // ======================================================================


    // Internet Connection
    // ======================================================================
    clearRenderer(Color::WHITE);

    //WiFi.mode(WIFI_STA);
    //WiFi.begin(conf::networkSSID, conf::networkPassword);

    WiFiMulti.addAP("VUSD-Wireless2", "N@v15ta!");

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");

        delay(50);
    }
    Serial.println();

    Serial.println("Connected to internet.");

    Serial.print("Local IP Address: ");
    Serial.println(WiFi.localIP());
    // ======================================================================


    // Websocket Connection
    // ======================================================================
    clearRenderer(Color::BLUE);

    client.begin("mvhscsf.ml", 4567, "/api");
    client.onEvent(webSocketEvent);
    client.setReconnectInterval(5000);

    Serial.println("Connected to websocket server.");
    // ======================================================================

    clearRenderer(Color::LIME);
}

string type;

SimpleRendererData simpleRendererData;
TwoLineRendererData twoLineRendererData;
ImageRendererData imageRendererData;

unsigned long long heapCheckCooldown = 5000;
unsigned long long lastHeapCheck = 0;

void updateRendererData(uint8_t* &payload) {
    DynamicJsonDocument document(1200);

    DeserializationError error = deserializeJson(document, payload);

    if (error) {
        Serial.print("Encountered error deserializing display data: ");
        Serial.println(error.f_str());

        type = "simple";
        simpleRendererData = {error.c_str(), {255, 0, 0}, {255, 255, 0}, 2};
        return;
    }

    if (document["type"].as<string>() == "simple") {
        type = "simple";

        simpleRendererData.text = document["data"]["message"].as<string>();

        simpleRendererData.textColor[0] = document["data"]["textColor"][0].as<int>();
        simpleRendererData.textColor[1] = document["data"]["textColor"][1].as<int>();
        simpleRendererData.textColor[2] = document["data"]["textColor"][2].as<int>();

        simpleRendererData.borderColor[0] = document["data"]["borderColor"][0].as<int>();
        simpleRendererData.borderColor[1] = document["data"]["borderColor"][1].as<int>();
        simpleRendererData.borderColor[2] = document["data"]["borderColor"][2].as<int>();

        simpleRendererData.speed = document["data"]["speed"].as<int>();

        simpleRendererData.brightness = document["data"]["brightness"].as<int>();

        Serial.println("Deserialized simple display data.");
    }
    else if (document["type"].as<string>() == "twoline") {
        type = "twoline";

        twoLineRendererData.bottomText = document["data"]["bottomMessage"].as<string>();

        twoLineRendererData.bottomTextColor[0] = document["data"]["bottomTextColor"][0].as<int>();
        twoLineRendererData.bottomTextColor[1] = document["data"]["bottomTextColor"][1].as<int>();
        twoLineRendererData.bottomTextColor[2] = document["data"]["bottomTextColor"][2].as<int>();

        twoLineRendererData.bottomSpeed = document["data"]["bottomSpeed"].as<int>();

        twoLineRendererData.topText = document["data"]["topMessage"].as<string>();

        twoLineRendererData.topTextColor[0] = document["data"]["topTextColor"][0].as<int>();
        twoLineRendererData.topTextColor[1] = document["data"]["topTextColor"][1].as<int>();
        twoLineRendererData.topTextColor[2] = document["data"]["topTextColor"][2].as<int>();

        twoLineRendererData.topSpeed = document["data"]["topSpeed"].as<int>();

        twoLineRendererData.borderColor[0] = document["data"]["borderColor"][0].as<int>();
        twoLineRendererData.borderColor[1] = document["data"]["borderColor"][1].as<int>();
        twoLineRendererData.borderColor[2] = document["data"]["borderColor"][2].as<int>();

        twoLineRendererData.brightness = document["data"]["brightness"].as<int>();

        Serial.println("Deserialized twoline display data.");
    }
    else if (document["type"].as<string>() == "image") {
        type = "image";

        imageRendererData.image = matrix(conf::displayWidth, conf::displayHeight);

        for (auto x: document["data"]["image"].as<JsonArray>()) {
            for (auto y: x.as<JsonArray>()) {
                imageRendererData.image[x][y] = y;
            }
        }

        imageRendererData.brightness = document["data"]["brightness"].as<int>();

        Serial.println("Deserialized image display data.");
    }

    document.clear();
}

void loop() {
    client.loop();
    // ======================================================================
    /*
    if (!client.isConnected()) {
        client.connect(conf::serverHost, conf::serverPath, conf::serverPort);
        client.send("init");
    }
     */
    // ======================================================================


    // ======================================================================
    // Check for message from server and update
    //string payload;

    //if (client.getMessage(payload)) updateRendererData(payload);

    // Print current size of heap every 5 seconds
    // ======================================================================
    if (heapCheckCooldown + lastHeapCheck < millis()) {
        lastHeapCheck = millis();

        Serial.print("Current Free Heap Size: ");
        Serial.println(EspClass::getFreeHeap());
    }
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