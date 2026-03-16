#pragma once

#include <QObject>
#include <QImage>
#include <QString>
#include "../helpers/helpers.h"

#include "../device_quirks/scanProfiles.h"

struct ScannerCapabilities
{
    QList<int> supportedResolutions;
    int minDpi = 0;
    int maxDpi = 0;

    QList<int> supportedBitDepths;
    QStringList supportedSources;
    QString transparencySource;

    bool supportsTransparency = false;
    bool supports8Bit = false;
    bool supports16Bit = false;

    QString stdSource = "Flatbed";
    QString filmSource = "Transparency Adapter";

    const ScannerQuirks* quirks = nullptr;
};

struct ScanParameters
{
    bool color = true;
    int dpi = 0;
    int bitDepth = 16;
    int frameIndex = 0;

    QString outputFolder;
    const ScannerCapabilities* cap = nullptr;
};

Q_DECLARE_METATYPE(ScanParameters)
Q_DECLARE_METATYPE(ScannerCapabilities)

class ScanWorker : public QObject
{
    Q_OBJECT
public:
    explicit ScanWorker(QObject *parent = nullptr);
    QString getDeviceName() const { return deviceName; }
    void setDeviceName(const QString &name) { deviceName = name; };

public slots:
    void requestPreview(const ScanParameters &params);
    void requestScan(const ScanParameters &params);
    void requestDeviceList();
    void requestCapabilities();
    
signals:
    void scanStarted(int estimatedSeconds);
    void scanFinished(const QImage &img);
    void previewFinished(const QImage &img);
    void deviceListReady(QStringList devices);
    void capabilitiesReady(const ScannerCapabilities &caps);

private:    
    QString deviceName;
    QImage performScan(const ScanParameters &params);
    ScannerCapabilities readCapabilities();
};
