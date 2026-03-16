#pragma once

#include <QString>
#include <QMap>

struct DeviceQuirks{
    QString modelName;
    QString usbId;

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
};

class DeviceFinder {
    Q_OBJECT
public slots:
    static bool findDeviceQuirksByUsbId(QString usbId, DeviceQuirks& quirks);
    static bool findDeviceQuirksByModel(QString modelName, DeviceQuirks& quirks);
public:
private:
};