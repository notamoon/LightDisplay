#ifndef LIGHTDISPLAY_CONF_H
#define LIGHTDISPLAY_CONF_H

namespace conf {

    const char* networkSSID = "VUSD";
    const char* networkPassword = "";

    const char* serverHost = "123.56.7.89";
    const int   serverPort = 1234;

    const char* httpURL = "/";
    const char* dataURL = "/";

    const int displayWidth  = 30;
    const int displayHeight = 20;

    const int dataPin = 0;

    const bool inverted = false;
}
#endif //LIGHTDISPLAY_CONF_H
