#pragma once

#include <QString>
#include <QMap>

struct DeviceQuirks{
    QString modelName;
    QString manufacturer;
    QString friendlyName;

    bool trueGray = false;        // real grayscale sensor
    bool rgbSensor = true;        // RGB Bayer / tri-linear CCD

    int calibrationStrip = 0;     // pixels to crop from left

    enum GrayMethod {
        GREEN_CHANNEL,
        RGB_WEIGHTED,
        RGB_MEDIAN,
        SCANNER_NATIVE
    };

    QMap<int,int> dpiToSeconds;  

    GrayMethod grayMethod = GREEN_CHANNEL;
};

class DeviceQuirkDatabase{
public:
    static void loadProfiles();
    static bool findByFriendlyName(const QString& name, DeviceQuirks& out);

private:
    static void loadProfileFile(const QString& path);

    static QHash<QString, DeviceQuirks> quirksByFriendlyName;
    static QHash<QString, DeviceQuirks> defaultByVendor;
};