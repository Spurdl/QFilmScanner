#pragma once

#include <QString>
#include <QMap>

class DeviceQuirks
{
public:
    QString modelName;
    QString usbId;
    QString backend; 
    QString vendor;

    bool trueGray = false;        // real grayscale sensor
    bool rgbSensor = true;        // RGB Bayer / tri-linear CCD

    int calibrationStrip = 0;     // pixels to crop from left

    enum GrayMethod {
        GREEN_CHANNEL,
        RGB_WEIGHTED,
        SCANNER_NATIVE
    };

    QMap<int,int> dpiToSeconds;  

    GrayMethod grayMethod = GREEN_CHANNEL;
private:

};