#ifndef LIGHTDISPLAY_DATA_H
#define LIGHTDISPLAY_DATA_H

#include <string>
#include <vector>

using namespace std;


struct SimpleRendererData {
    string text;
    int textColor[3];
    int borderColor[3];
    int speed;

    int brightness;
};

struct TwoLineRendererData {
    string topText;
    int topTextColor[3];
    int topSpeed;

    string bottomText;
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
