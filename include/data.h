#ifndef LIGHTDISPLAY_DATA_H
#define LIGHTDISPLAY_DATA_H

#include <Arduino.h>

using namespace std;


struct SimpleRendererData {
    String text;
    int textColor;
    int borderColor;
    int speed;

    int brightness;
};

struct TwoLineRendererData {
    String topText;
    int topTextColor;
    int topSpeed;

    String bottomText;
    int bottomTextColor;
    int bottomSpeed;

    int borderColor;

    int brightness;
};

struct ImageRendererData {
    uint32_t image[600];

    int brightness = 0;
};

#endif //LIGHTDISPLAY_DATA_H
