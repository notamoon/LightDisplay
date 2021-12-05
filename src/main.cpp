#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WebSocketsClient.h>

#include "ArduinoJson.h"
#include "Adafruit_NeoPixel.h"
#include "LightRenderer.h"
#include "fonts.h"

#include "conf.h"

Adafruit_NeoPixel strip(conf::displayWidth * conf::displayHeight, conf::dataPin, NEO_GRB + NEO_KHZ800);

LightRenderer renderer(strip, conf::displayWidth, conf::displayHeight);

WebSocketsClient client;

void clientDataReceived(WStype_t, uint8_t*, size_t);

void setup() {
    // Light Renderer Setup
    // ======================================================================
    renderer.setup();

    renderer.setInverted(conf::inverted);
    renderer.setBrightness(255);

    renderer.update();
    renderer.render();
    // ======================================================================


    // Internet Connection
    // ======================================================================
    WiFi.mode(WIFI_STA);
    WiFi.begin(conf::networkSSID, conf::networkPassword);

    renderer.setFont(Fonts::FONT1);
    renderer.setColor(Color::RED);

    while (WiFi.status() != WL_CONNECTED) {
        renderer.scrollText("Connecting to internet...", 0, 0, 1, -1, Direction::HORIZONTAL, 1);

        renderer.update();
        renderer.render();

        delay(50);
    }
    // ======================================================================


    // Websocket Connection
    // ======================================================================
    client.begin(conf::serverHost, conf::serverPort, conf::dataURL);

    client.onEvent(clientDataReceived);
    client.setReconnectInterval(5000);
    client.enableHeartbeat(15000, 3000, 2);
    // ======================================================================
}

struct RendererData {
    int type;

    string line1;
    string line2;

    int color1;
    int color2;
    int color3;

    int brightness;
    int speed;

    matrix pixels;
};

RendererData rendererData = {0, "", "", Color::WHITE, Color::WHITE, Color::BLUE, 128, 1,matrix(30, 20) };

void loop() {
    // One line of text
    // ======================================================================
    if (rendererData.type == 0) {
        renderer.clear();

        renderer.setColor(rendererData.color3);

        renderer.drawArea(0, 0, 30, 20);

        renderer.setFont(Fonts::FONT2);

        renderer.setColor(rendererData.color1);
        renderer.scrollText(rendererData.line1, 2, 4, 2, 26, Direction::HORIZONTAL, rendererData.speed);
    }
    // ======================================================================


    // One line of text
    // ======================================================================
    if (rendererData.type == 1) {
        renderer.clear();

        renderer.setColor(rendererData.color3);

        renderer.drawArea(0, 0, 30, 20);

        renderer.setFont(Fonts::FONT1);

        renderer.setColor(rendererData.color1);
        renderer.scrollText(rendererData.line1, 2, 3, 2, 26, Direction::HORIZONTAL, rendererData.speed);

        renderer.setColor(rendererData.color2);
        renderer.scrollText(rendererData.line2, 2, 11, 2, 26, Direction::HORIZONTAL, rendererData.speed);
    }
    // ======================================================================


    // One line of text
    // ======================================================================
    if (rendererData.type == 2) {
        renderer.clear();

        for (int x = 0; x < conf::displayWidth; x++) {
            for (int y = 0; y < conf::displayHeight; y++) {
                renderer.setColor(rendererData.pixels[x][y]);
                renderer.drawPixel(x, y);
            }
        }
    }
    // ======================================================================


    renderer.update();
    renderer.render();
}

void updateRendererData(uint8_t* payload) {
    DynamicJsonDocument jsonData(400);

    DeserializationError error = deserializeJson(jsonData, payload);

    if (error) {
        rendererData = {0, "Error", "", Color::RED, 0, Color::WHITE, 255, 1};
        return;
    }

    //TODO
}

void clientDataReceived(WStype_t type, uint8_t* payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            rendererData = {0, "Disconnected from server", "", Color::RED, 0, Color::BLUE, 255, 1};
            break;
        case WStype_CONNECTED:
            rendererData = {0, "Connected to server", "", Color::LIME, 0, Color::BLUE, 255, 1};
            break;
        case WStype_TEXT:
            updateRendererData(payload);
            break;
    }
}