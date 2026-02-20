#pragma once

#include <QObject>
#include <QImage>
#include <QString>

struct ScannerCapabilities
{
    QList<int> supportedResolutions;
    int minDpi = 0;
    int maxDpi = 0;

    QList<int> supportedBitDepths;
    QStringList supportedSources;

    bool supportsTransparency = false;
    bool supports16Bit = false;
};

struct ScanParameters
{
    int dpi = 0;
    bool color = true;
    int bitDepth = 16;
    QString source = "Transparency Adapter";
    QString outputFolder;
    int frameIndex = 0;
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
    void deviceListReady(QStringList devices);
    void capabilitiesReady(const ScannerCapabilities &caps);

private:    
    QString deviceName;
    QImage performScan(const ScanParameters &params);
    ScannerCapabilities readCapabilities();
};
