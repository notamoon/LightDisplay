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
    // LIGHT RENDERER SETUP ========================
    renderer.setup();

    renderer.setInverted(conf::inverted);
    renderer.setBrightness(255);

    renderer.update();
    renderer.render();
    // =============================================

    // INTERNET CONNECTION =========================
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
    // =============================================

    // WEBSOCKET CLIENT ============================
    client.begin(conf::serverHost, conf::serverPort, conf::dataURL);

    client.onEvent(clientDataReceived);
    client.setReconnectInterval(5000);
    client.enableHeartbeat(15000, 3000, 2);
    // =============================================
}

void loop() {
    renderer.update();
    renderer.render();
}

void clientDataReceived(WStype_t type, uint8_t* payload, size_t length) {
//// webSocket.sendTXT("message here");
//// webSocket.sendBIN(payload, length);
    switch(type) {
        case WStype_DISCONNECTED:
            break;
        case WStype_CONNECTED:
            break;
        case WStype_TEXT:
            break;
        case WStype_BIN:
            break;
        case WStype_PING:
            break;
        case WStype_PONG:
            break;
    }
}