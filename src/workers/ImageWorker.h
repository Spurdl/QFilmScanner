#pragma once

#include <QObject>
#include <QImage>
#include <QTransform>

struct ImageEditParams {
    bool turn           = false;
    bool mirror         = false;
    bool invert         = false;
    bool autolevel      = false;
};

class ImageWorker : public QObject
{
    Q_OBJECT
public:
    ImageWorker(QObject *parent = nullptr) : QObject(parent) {}

public slots:
    void processImage(const QImage &img, const ImageEditParams &params);

signals:
    void imageProcessed(const QImage &img);
};

