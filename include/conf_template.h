#ifndef LIGHTDISPLAY_CONF_H
#define LIGHTDISPLAY_CONF_H

namespace conf {

    const int bootDelay = 10;

    const char* networkSSID = "";
    const char* networkPassword = "";

    const char* serverHost = "12.34.56.78";
    const char* serverPath = "/";
    const int   serverPort = 1234;

    const int displayWidth = 30, displayHeight = 20;

    const int dataPin = 14;

    const int fragmentSize  = 30;
    const int fragmentCount = 20;

    const bool inverted = false;
}
#endif //LIGHTDISPLAY_CONF_H
