#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WebSocketsClient.h>

#include "Adafruit_NeoPixel.h"
#include "LightRenderer.h"
#include "fonts.h"

#include "conf.h"
#include "data.h"

Adafruit_NeoPixel strip(conf::displayWidth * conf::displayHeight, conf::dataPin, NEO_GRB + NEO_KHZ800);

LightRenderer renderer(strip, conf::displayWidth, conf::displayHeight);

ESP8266WiFiMulti internet;

WebSocketsClient client;

unsigned long timeStarted;

void renderSimple(int brightness, int borderColor, int textColor, String text, int speed);
void renderTwoLine(int brightness, int borderColor, int topTextColor, String topText, int topSpeed, int bottomTextColor, String bottomText, int bottomSpeed);
void renderImage(int brightness, uint32_t image[]);

String requestField(const String& field);
vector<int> requestImageFragment(int fragmentIndex, int fragmentSize);

void updateRendererData();
void clientUpdateReceived(WStype_t, uint8_t*, size_t);
void clearRenderer(int);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Serial Monitor
    // ======================================================================
    clearRenderer(Color::CYAN);

    Serial.begin(115200);
    Serial.println("Began Serial output");
    // ======================================================================

    for (int i = conf::bootDelay; i > 0; i--) {
        Serial.printf("Starting in... %d\n", i);
        delay(1000);
    }

    timeStarted = millis();

    // Light Renderer Setup
    // ======================================================================
    renderer.setup();

    renderer.setInverted(conf::inverted);
    renderer.setBrightness(255);

    renderer.update();
    renderer.render();

    Serial.println("Setup renderer");
    // ======================================================================


    // Internet Connection
    // ======================================================================
    internet.addAP(conf::networkSSID, conf::networkPassword);

    //WiFi.disconnect();
    while(internet.run() != WL_CONNECTED) {
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
    client.begin(conf::serverHost, conf::serverPort, conf::serverPath);
    client.onEvent(clientUpdateReceived);
    client.enableHeartbeat(15000, 3000, 2);
    client.setReconnectInterval(5000);

    Serial.println("Connected to websocket server.");
    // ======================================================================

    clearRenderer(Color::LIME);

    digitalWrite(2, HIGH);
}

String type;

SimpleRendererData simpleRendererData;
TwoLineRendererData twoLineRendererData;
ImageRendererData imageRendererData;

bool recreateImage = false;

unsigned long long heapCheckCooldown = 5000;
unsigned long long lastHeapCheck = 0;

void loop() {
    client.loop();

    // Print current size of heap every 5 seconds
    // ======================================================================
    if (heapCheckCooldown + lastHeapCheck < millis()) {
        lastHeapCheck = millis();

        Serial.print("Current Free Heap Size: ");
        Serial.println(EspClass::getFreeHeap());

        Serial.print("Time Running: ");
        Serial.println(millis() - timeStarted);
    }
    // ======================================================================


    // One line of text
    // ======================================================================
    if (type == "simple") {
        renderSimple(
                simpleRendererData.brightness,
                simpleRendererData.borderColor,
                simpleRendererData.textColor,
                simpleRendererData.text,
                simpleRendererData.speed
                );
    }
    // ======================================================================


    // Two lines of text
    // ======================================================================
    else if (type == "twoline") {
        renderTwoLine(
                twoLineRendererData.brightness,
                twoLineRendererData.borderColor,
                twoLineRendererData.topTextColor,
                twoLineRendererData.topText,
                twoLineRendererData.topSpeed,
                twoLineRendererData.bottomTextColor,
                twoLineRendererData.bottomText,
                twoLineRendererData.bottomSpeed
                );
    }
    // ======================================================================


    // Image
    // ======================================================================
    else if (type == "image") {
        if (recreateImage) {
            recreateImage = false;

            renderImage(imageRendererData.brightness, imageRendererData.image);
        }
    }
    // ======================================================================


    // No Data
    // ======================================================================
    else {
        renderer.setBrightness(255);

        renderer.clear();

        renderer.setColor(Color::YELLOW);

        renderer.drawArea(0, 0, 30, 20);

        renderer.setFont(Fonts::FONT2);

        renderer.setColor(Color::RED);

        String text = "No Data";
        renderer.scrollText(text, 2, 4, 2, 26, Direction::HORIZONTAL, 3);
    }
    // ======================================================================

    renderer.update();
    renderer.render();
}

String requestField(const String& field) {
    yield();

    Serial.println(String("   Requesting data field ") + field);

    WiFiClient internetClient;

    HTTPClient http;

    String response;

    if (http.begin(internetClient, String("http://") + conf::serverHost + ":" + conf::serverPort + conf::serverPath + "?field=" + field))
        if (http.GET() == HTTP_CODE_OK) response = http.getString();

    http.end();

    return response;
}

vector<int> requestImageFragment(int fragmentIndex, int fragmentSize) {
    yield();

    Serial.println(String("   Requesting image fragment ") + fragmentIndex);

    WiFiClient internetClient;

    HTTPClient http;

    String response;

    if (http.begin(internetClient, String("http://") + conf::serverHost + ":" + conf::serverPort + conf::serverPath + "?image&fragment_index=" + fragmentIndex + "&fragment_size=" + fragmentSize))
        if (http.GET() == HTTP_CODE_OK) response = http.getString();

    int nextSeperator;

    vector<int> colors;

    while ((nextSeperator = response.indexOf(' ')) != -1) {
        colors.push_back(response.substring(0, nextSeperator).toInt());

        response = response.substring(nextSeperator + 1);
    }

    colors.push_back(response.toInt());

    return colors;
}

void updateRendererData() {
    Serial.println("Updating renderer data...");

    renderTwoLine(255, Color::BLUE, Color::YELLOW, "Read", 0, Color::YELLOW, "Data", 0);

    renderer.update();
    renderer.render();

    if (type == "simple")
    {
        simpleRendererData.text = requestField("message");

        simpleRendererData.textColor = requestField("textColor").toInt();
        simpleRendererData.borderColor = requestField("borderColor").toInt();
        simpleRendererData.speed = requestField("speed").toInt();
        simpleRendererData.brightness = requestField("brightness").toInt();

        Serial.println("Deserialized simple display data.");
    }

    if (type == "twoline")
    {
        twoLineRendererData.topText = requestField("topMessage");
        twoLineRendererData.topTextColor = requestField("topTextColor").toInt();
        twoLineRendererData.topSpeed = requestField("topSpeed").toInt();

        twoLineRendererData.bottomText = requestField("bottomMessage");
        twoLineRendererData.bottomTextColor = requestField("bottomTextColor").toInt();
        twoLineRendererData.bottomSpeed = requestField("bottomSpeed").toInt();

        twoLineRendererData.borderColor = requestField("borderColor").toInt();

        twoLineRendererData.brightness = requestField("brightness").toInt();

        Serial.println("Deserialized twoline display data.");
    }

    if (type == "image") {
        int c = 0;

        for (int i = 0; i < conf::fragmentCount; ++i) {
            for (auto color : requestImageFragment(i, conf::fragmentSize))
                imageRendererData.image[c++] = color;
        }

        imageRendererData.brightness = requestField("brightness").toInt();

        recreateImage = true;

        Serial.println("Deserialized image display data.");
    }
}

void clientUpdateReceived(WStype_t wsType, uint8_t * payload, size_t length) {
    yield();

    if (wsType == WStype_CONNECTED) {
        Serial.printf("Connected to url: %s\n", payload);
        client.sendTXT("init");
    }
    else if (wsType == WStype_TEXT) {
        Serial.printf("Received text: %s\n", payload);

        type.clear();
        type.concat(reinterpret_cast<char*>(payload));

        updateRendererData();
    }
}

void clearRenderer(int color) {
    renderer.setColor(color);
    renderer.fillArea(0, 0, conf::displayWidth, conf::displayHeight);

    renderer.update();
    renderer.render();
}

void renderSimple(int brightness, int borderColor, int textColor, String text, int speed) {
    yield();

    renderer.setBrightness(brightness);

    renderer.clear();

    renderer.setColor(borderColor);

    renderer.drawArea(0, 0, 30, 20);

    renderer.setFont(Fonts::FONT2);

    renderer.setColor(textColor);
    renderer.scrollText(text, 2, 4, 2, 26, Direction::HORIZONTAL, speed);
}

void renderTwoLine(int brightness, int borderColor, int topTextColor, String topText, int topSpeed, int bottomTextColor, String bottomText, int bottomSpeed) {
    yield();

    renderer.setBrightness(brightness);

    renderer.clear();

    renderer.setColor(borderColor);

    renderer.drawArea(0, 0, 30, 20);

    renderer.setFont(Fonts::FONT1);

    renderer.setColor(topTextColor);
    renderer.scrollText(topText, 2, 11, 2, 26, Direction::HORIZONTAL, topSpeed);

    renderer.setColor(bottomTextColor);
    renderer.scrollText(bottomText, 2, 3, 2, 26, Direction::HORIZONTAL, bottomSpeed);
}

void renderImage(int brightness, uint32_t image[]) {
    yield();

    renderer.setBrightness(brightness);

    renderer.clear();

    int c = 0;

    for (int col = 0; col < conf::displayWidth; ++col) {
        for (int row = 0; row < conf::displayHeight; ++row) {
            renderer.setColor(image[c++]);
            renderer.drawPixel(col, conf::displayHeight - 1 - row);
        }
    }
}