#pragma once

#include <QObject>
#include <QImage>
#include <QString>


class ScanWorker : public QObject
{
    Q_OBJECT
public:
    QString getDeviceName() const { return deviceName; }

public slots:
    void doPreview(bool color);
    void doScan(int frameIndex, bool color, QString folder);
    void setDeviceName(const QString &name) { deviceName = name; }
    void requestDeviceList();

signals:
    void imageReady(const QImage &img);
    void scanStarted(int estimatedSeconds);
    void progress(int percent);
    void scanFinished();
    void deviceListReady(QStringList devices);

private:    
    QString deviceName;
    QImage performScan(int dpi, bool color);
};
