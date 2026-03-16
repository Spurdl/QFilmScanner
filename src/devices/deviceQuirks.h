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

class DeviceQuirkDatabase {
    Q_OBJECT
public slots:
    static bool findDeviceQuirksByUsbId(QString usbId, DeviceQuirks& quirks);
    static bool findDeviceQuirksByModel(QString modelName, DeviceQuirks& quirks);
public:
    void loadDirectory(const QString& path);
    const DeviceQuirks* findByUsbId(const QString& usbId) const {
        auto it = usbMap.find(usbId);
        return (it != usbMap.end()) ? &it.value() : nullptr;
    }
    const DeviceQuirks* findByModel(const QString& model) const {
        auto it = modelMap.find(model);
        return (it != modelMap.end()) ? &it.value() : nullptr;
    }
private:
    QMap<QString, DeviceQuirks> usbMap;
    QMap<QString, DeviceQuirks> modelMap;
};