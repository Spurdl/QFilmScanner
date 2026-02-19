#include "ScanWorker.h"
#include <sane/sane.h>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <fstream>
#include <QDebug>

static void setOption(SANE_Handle handle, const char* name, void* value)
{
    for (int i=0;;++i)
    {
        const SANE_Option_Descriptor* opt = sane_get_option_descriptor(handle,i);
        if (!opt) break;
        if (opt->name && strcmp(opt->name,name)==0)
        {
            sane_control_option(handle,i,SANE_ACTION_SET_VALUE,value,nullptr);
            break;
        }
    }
}

void ScanWorker::requestDeviceList()
{
    sane_init(nullptr,nullptr);

    const SANE_Device **device_list = nullptr;
    sane_get_devices(&device_list,SANE_FALSE);

    QStringList devices;

    if(device_list)
    {
        for(int i=0; device_list[i]; ++i)
        {
            QString id = device_list[i]->name;

            QString label;

            if (device_list[i]->vendor && device_list[i]->model)
                label = QString("%1 %2")
                            .arg(device_list[i]->vendor)
                            .arg(device_list[i]->model);
            else
                label = id;  // fallback if no nice name

            devices << QString("%1|%2").arg(id).arg(label);
        }
    }

    sane_exit();

    emit deviceListReady(devices);
}

QImage invertPreview(const QImage &src, bool flipVertical=false)
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
                rVec[y*width + x] = 65535 - row[x*4 + 0]; 
                gVec[y*width + x] = 65535 - row[x*4 + 1];
                bVec[y*width + x] = 65535 - row[x*4 + 2];
            }
        }

        auto computePercentiles = [](const std::vector<uint16_t> &data) -> std::pair<uint16_t,uint16_t> {
            std::vector<uint16_t> copy = data;
            std::sort(copy.begin(), copy.end());
            size_t n = copy.size();
            return {copy[static_cast<size_t>(n*0.005)],
                    copy[static_cast<size_t>(n*0.995)]};
        };

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
                grayVec[y*width + x] = 65535 - row[x];
        }

        auto computePercentiles = [](const std::vector<uint16_t> &data) -> std::pair<uint16_t,uint16_t> {
            std::vector<uint16_t> copy = data;
            std::sort(copy.begin(), copy.end());
            size_t n = copy.size();
            return {copy[static_cast<size_t>(n*0.005)],
                    copy[static_cast<size_t>(n*0.995)]};
        };

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

QImage ScanWorker::performScan(int dpi, bool color)
{
    sane_init(nullptr,nullptr);

    SANE_Handle handle;
    if (sane_open(deviceName.toUtf8().constData(), &handle) != SANE_STATUS_GOOD)
        return {};

    SANE_Int resolution = dpi;
    setOption(handle, "resolution", &resolution);
    setOption(handle, "source", (void*)"Transparency Adapter");
    color ? setOption(handle, "mode", (void*)"Color")
          : setOption(handle, "mode", (void*)"Gray");

    sane_start(handle);

    SANE_Parameters params;
    sane_get_parameters(handle, &params);

    int width = params.pixels_per_line;
    int height = params.lines;
    int bytesPerSample = params.depth / 8;
    int channels = (params.format == SANE_FRAME_RGB) ? 3 :
                   (params.format == SANE_FRAME_GRAY) ? 1 : 0;

    // Only allow 16-bit scans for now
    if (params.depth != 16 || channels == 0)
    {
        sane_close(handle);
        sane_exit();
        return {};
    }

    

    size_t totalBytes = width * height * channels * bytesPerSample;
    std::vector<uint8_t> data(totalBytes);

    SANE_Status status;
    SANE_Int len;
    size_t offset = 0;

    while ((status = sane_read(handle,
                               data.data() + offset,
                               data.size() - offset,
                               &len)) == SANE_STATUS_GOOD)
    {
        offset += len;
        int percent = (offset * 100) / totalBytes;
        emit progress(percent);
    }

    sane_close(handle);
    sane_exit();

    if (status != SANE_STATUS_EOF || offset != totalBytes)
        return {};

    if (params.format == SANE_FRAME_RGB)
    {
        std::vector<uint16_t> rgba(width * height * 4);
        uint16_t* src = reinterpret_cast<uint16_t*>(data.data());

        int samplesPerRow = params.bytes_per_line / 2;
        int channels = 3;

        for (int y = 0; y < height; ++y)
        {
            uint16_t* row = src + y * samplesPerRow;
            for (int x = 0; x < width; ++x)
            {
                rgba[(y*width + x)*4 + 0] = row[x*channels + 0];
                rgba[(y*width + x)*4 + 1] = row[x*channels + 1];
                rgba[(y*width + x)*4 + 2] = row[x*channels + 2];
                rgba[(y*width + x)*4 + 3] = 65535; // full alpha
            }
        }

        QImage img(reinterpret_cast<const uchar*>(rgba.data()),
                width,
                height,
                QImage::Format_RGBX64);

        const int STRIPE_WIDTH = 60; // hardware stripe width
        if (width > STRIPE_WIDTH)
        {
            QImage cropped = img.copy(STRIPE_WIDTH, 0, width - STRIPE_WIDTH, height);
            img = cropped;
            width -= STRIPE_WIDTH;
        }

        return img.copy();
    }

    if (params.format == SANE_FRAME_GRAY)
    {
        QImage img(reinterpret_cast<const uchar*>(data.data()),
                   width,
                   height,
                   QImage::Format_Grayscale16);

        const int STRIPE_WIDTH = 60; // hardware stripe width, idk whats wrong with my plustek
        if (width > STRIPE_WIDTH)
        {
            QImage cropped = img.copy(STRIPE_WIDTH, 0, width - STRIPE_WIDTH, height);
            img = cropped;
            width -= STRIPE_WIDTH;
        }


        return img.copy();
    }

    return {};
}

void ScanWorker::doPreview(bool color)
{
    emit scanStarted(31);
    QImage img = performScan(900, color);
    img = invertPreview(img, false);
    emit scanFinished();
    emit imageReady(img);
}

void ScanWorker::doScan(int frameIndex, bool color, QString folder)
{
    emit scanStarted(115);
    QImage img = performScan(7200, color);

    folder = folder.trimmed();
    QString filename =
        QString("%1/scan_%2.png")
        .arg(folder)
        .arg(frameIndex, 3, 10, QChar('0'));
    filename = filename.trimmed();
    qDebug() << filename << " ";
    
    bool success = img.save(filename);

    qDebug() << success;
    //img = invertPreview(img, false);

    emit scanFinished();
    //emit imageReady(img);
}
