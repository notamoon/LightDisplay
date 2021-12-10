#ifndef LIGHTDISPLAY_DATA_H
#define LIGHTDISPLAY_DATA_H

#include <Arduino.h>

using namespace std;


struct SimpleRendererData {
    String text;
    int textColor[3];
    int borderColor[3];
    int speed;

    int brightness;
};

struct TwoLineRendererData {
    String topText;
    int topTextColor[3];
    int topSpeed;

    String bottomText;
    int bottomTextColor[3];
    int bottomSpeed;

    int borderColor[3];

    int brightness;
};

struct ImageRendererData {
    matrix image;

    int brightness = 0;
};

#endif //LIGHTDISPLAY_DATA_H
