#include <vector>
#include <algorithm>

#include <QDir>
#include <QTransform>

#include "ImageWorker.h"

QList<QString> supportedSaveMethods = {
    "Raw", "Edited", "Raw + Edited"
};

QList<QString> supportedSaveTypes = {
    "JPEG", "PNG", "TIFF", "WEBP", "BMP"
};

QImage autoLevel(const QImage &src, double lowPercent, double highPercent)
{
    if (src.isNull()) return src;

    QImage img = src;
    int width = img.width();
    int height = img.height();

    auto stretch = [](uint16_t val, uint16_t p0, uint16_t p100) -> uint16_t {
        if(val <= p0) return 0;
        if(val >= p100) return 65535;
        return (val - p0) * 65535 / (p100 - p0);
    };

    auto computePercentiles = [lowPercent, highPercent](const std::vector<uint16_t> &data) -> std::pair<uint16_t,uint16_t> {
        std::vector<uint16_t> copy = data;
        std::sort(copy.begin(), copy.end());
        size_t n = copy.size();
        return {copy[static_cast<size_t>(n*lowPercent)],
                copy[static_cast<size_t>(n*highPercent)]};
    };

    if (img.format() == QImage::Format_RGBX64)
    {
        std::vector<uint16_t> rVec(width*height);
        std::vector<uint16_t> gVec(width*height);
        std::vector<uint16_t> bVec(width*height);

        for(int y = 0; y < height; ++y)
        {
            auto row = reinterpret_cast<uint16_t*>(img.scanLine(y));
            for(int x = 0; x < width; ++x)
            {
                rVec[y*width + x] = row[x*4 + 0];
                gVec[y*width + x] = row[x*4 + 1];
                bVec[y*width + x] = row[x*4 + 2];
            }
        }

        auto [r0,r100] = computePercentiles(rVec);
        auto [g0,g100] = computePercentiles(gVec);
        auto [b0,b100] = computePercentiles(bVec);

        for(int y = 0; y < height; ++y)
        {
            auto row = reinterpret_cast<uint16_t*>(img.scanLine(y));
            for(int x = 0; x < width; ++x)
            {
                row[x*4 + 0] = stretch(rVec[y*width + x], r0, r100);
                row[x*4 + 1] = stretch(gVec[y*width + x], g0, g100);
                row[x*4 + 2] = stretch(bVec[y*width + x], b0, b100);
            }
        }
    }
    else if (img.format() == QImage::Format_Grayscale16)
    {
        std::vector<uint16_t> grayVec(width*height);

        for(int y = 0; y < height; ++y)
        {
            auto row = reinterpret_cast<uint16_t*>(img.scanLine(y));
            for(int x = 0; x < width; ++x)
                grayVec[y*width + x] = row[x];
        }

        auto [g0,g100] = computePercentiles(grayVec);

        for(int y = 0; y < height; ++y)
        {
            auto row = reinterpret_cast<uint16_t*>(img.scanLine(y));
            for(int x = 0; x < width; ++x)
                row[x] = stretch(grayVec[y*width + x], g0, g100);
        }
    }

    return img;
}

QImage invert(const QImage &src)
{
    if (src.isNull()) return src;

    QImage img = src;
    int width = img.width();
    int height = img.height();

    if (img.format() == QImage::Format_RGBX64)
    {
        for(int y = 0; y < height; ++y)
        {
            auto row = reinterpret_cast<uint16_t*>(img.scanLine(y));
            for(int x = 0; x < width; ++x)
            {
                row[x*4 + 0] = 65535 - row[x*4 + 0];
                row[x*4 + 1] = 65535 - row[x*4 + 1];
                row[x*4 + 2] = 65535 - row[x*4 + 2];
            }
        }
    }
    else if (img.format() == QImage::Format_Grayscale16)
    {
        for(int y = 0; y < height; ++y)
        {
            auto row = reinterpret_cast<uint16_t*>(img.scanLine(y));
            for(int x = 0; x < width; ++x)
                row[x] = 65535 - row[x];
        }
    }

    return img;
}

QImage upsideDown(const QImage &src){
    QImage img = src;
    img = img.flipped(Qt::Orientation::Vertical);
    return img;
}

QImage flip90degrees(const QImage &src){
    QImage img = src.transformed(QTransform().rotate(90));
    return img;
}

QImage mirror(const QImage &src, Qt::Orientation orientation){
    QImage img = src;
    img = img.flipped(orientation);
    return img;
}

void ImageWorker::processImage(const QImage &img, const ImageEditParams &params){
    if (img.isNull()) {
        emit imageProcessed(img);
        return;
    }

    QImage result = img;

    if (params.turn){
        result = flip90degrees(result);
    }

    if (params.mirror_v){
        result = mirror(result, Qt::Orientation::Vertical);
    }

    if (params.mirror_h){
        result = mirror(result, Qt::Orientation::Horizontal);
    }

    if (params.invert) {
        result = invert(result);
    }

    if (params.autolevel) {
        result = autoLevel(result, 0.01, 0.99);
    }

    emit imageProcessed(result);
}

QImage ImageWorker::processInternally(const QImage &img, const ImageEditParams &params){
    QImage result = img.copy();

    if (params.turn){
        result = flip90degrees(result);
    }

    if (params.mirror_v){
        result = mirror(result, Qt::Orientation::Vertical);
    }

    if (params.mirror_h){
        result = mirror(result, Qt::Orientation::Horizontal);
    }

    if (params.invert) {
        result = invert(result);
    }

    if (params.autolevel) {
        result = autoLevel(result, 0.01, 0.99);
    }

    return result;
}

bool ImageWorker::saveWithSuffix(const QImage &img, QString saveLocation, int frameIdx, const ImageEditParams &params, const QString &suffix){
    QString extension = params.saveType.toLower();
    QString frameStr = QString("%1").arg(frameIdx, 3, 10, QChar('0'));

    QDir dir(saveLocation);
    QString baseName = dir.filePath(frameStr);
    
    QImage result;
    if(suffix.compare("edit", Qt::CaseInsensitive) == 0){
        result = processInternally(img, params);
    }
    else{
        result = img.copy();
    }

    QString fileName = QString("%1_%2_frame.%3")
                            .arg(baseName)
                            .arg(suffix)
                            .arg(extension);

    QImageWriter writer(fileName, params.saveType.toUtf8());

    if (params.saveType.compare("JPEG", Qt::CaseInsensitive) == 0)
        writer.setQuality(95);

    if (!writer.write(result)) {
        qWarning() << "Failed to save:" << fileName
                << writer.errorString();
        return false;
    }
    return true;
};

void ImageWorker::saveImage(const QImage &image, QString saveLocation, int frameIdx, const ImageEditParams &params){
    if (image.isNull()){
        emit saveComplete(false);
        return;
    }

    bool success;
    if (params.saveMethod.compare("Raw", Qt::CaseInsensitive) == 0) {
        success = saveWithSuffix(image, saveLocation, frameIdx, params, "raw");
    }
    else if (params.saveMethod.compare("Edited", Qt::CaseInsensitive) == 0) {
        success = saveWithSuffix(image, saveLocation, frameIdx, params,"edit");
    }
    else if (params.saveMethod.compare("Raw + Edited", Qt::CaseInsensitive) == 0) {
        success = saveWithSuffix(image, saveLocation, frameIdx, params,"raw");
        success &= saveWithSuffix(image, saveLocation, frameIdx, params,"edit");
    }

    emit saveComplete(success);
    
    return;
}