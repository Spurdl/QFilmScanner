#pragma once

#include <QObject>
#include <QImage>
#include <QList>
#include <QTransform>
#include <QImageWriter>

struct ImageEditParams {
    bool turn           = false;
    bool mirror_v       = false;
    bool mirror_h       = false;
    bool invert         = false;
    bool autolevel      = false;
    QString saveType    = "TIF";
    QString saveMethod  = "RAW";
};

extern QList<QString> supportedSaveTypes;
extern QList<QString> supportedSaveMethods;

class ImageWorker : public QObject
{
    Q_OBJECT
public:
    ImageWorker(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void processImage(const QImage &img, const ImageEditParams &params);
    void saveImage(const QImage &image, QString saveLocation, int frameIdx, const ImageEditParams &params);

private: 
    QImage processInternally(const QImage &img, const ImageEditParams &params);
    bool saveWithSuffix(const QImage &img, QString saveLocation, int frameIdx, const ImageEditParams &param, const QString &suffix);

signals:
    void imageProcessed(const QImage &img);
    void saveComplete(bool success);
};

