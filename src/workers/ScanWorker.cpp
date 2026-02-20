#include <algorithm>
#include <vector>
#include <cstdint>
#include <fstream>
#include <sane/sane.h>

#include <QDebug>

#include "ScanWorker.h"
#include "../device_quirks/scanProfiles.h"

ScanWorker::ScanWorker(QObject *parent)
    : QObject(parent)
{
}

#define SCAN_DEBUG 1

#if SCAN_DEBUG
#define DBG qDebug()
#else
#define DBG if(false) qDebug()
#endif

static bool setOptionByName(SANE_Handle handle,
                            const char* name,
                            void* value)
{
    for (int i = 0;; ++i)
    {
        const SANE_Option_Descriptor* opt =
            sane_get_option_descriptor(handle, i);

        if (!opt)
            break;

        if (opt->name && strcmp(opt->name, name) == 0)
        {
            SANE_Status st =
                sane_control_option(handle,
                                    i,
                                    SANE_ACTION_SET_VALUE,
                                    value,
                                    nullptr);

            //qDebug() << "Setting option"
            //         << name
            //         << "status:"
            //         << sane_strstatus(st);

            return (st == SANE_STATUS_GOOD);
        }
    }

    qDebug() << "Option not found:" << name;
    return false;
}

ScannerCapabilities ScanWorker::readCapabilities()
{
    ScannerCapabilities caps;

    sane_init(nullptr,nullptr);

    SANE_Handle handle;
    if (sane_open(deviceName.toUtf8().constData(), &handle) != SANE_STATUS_GOOD)
        return caps;

    for (int i = 0;; ++i)
    {
        const SANE_Option_Descriptor* opt =
            sane_get_option_descriptor(handle, i);

        if (!opt)
            break;

        if (!opt->name)
            continue;

        if (strcmp(opt->name, "resolution") == 0)
        {
            if (opt->constraint_type == SANE_CONSTRAINT_RANGE)
            {
                const SANE_Range* r = opt->constraint.range;
                caps.minDpi = r->min;
                caps.maxDpi = r->max;

                for (int v = r->min; v <= r->max; v += r->quant)
                    caps.supportedResolutions.append(v);
            }
            else if (opt->constraint_type == SANE_CONSTRAINT_WORD_LIST)
            {
                int count = opt->constraint.word_list[0];
                for (int j = 1; j <= count; ++j)
                    caps.supportedResolutions.append(
                        opt->constraint.word_list[j]);

                if (!caps.supportedResolutions.isEmpty())
                {
                    caps.minDpi =
                        *std::min_element(caps.supportedResolutions.begin(),
                                          caps.supportedResolutions.end());

                    caps.maxDpi =
                        *std::max_element(caps.supportedResolutions.begin(),
                                          caps.supportedResolutions.end());
                }
            }
        }

        if (strcmp(opt->name, "depth") == 0)
        {
            if (opt->constraint_type == SANE_CONSTRAINT_WORD_LIST)
            {
                int count = opt->constraint.word_list[0];
                for (int j = 1; j <= count; ++j)
                {
                    int d = opt->constraint.word_list[j];
                    caps.supportedBitDepths.append(d);
                    if (d == 16)
                        caps.supports16Bit = true;
                }
            }
        }

        if (strcmp(opt->name, "source") == 0)
        {
            if (opt->constraint_type == SANE_CONSTRAINT_STRING_LIST)
            {
                const SANE_String_Const* list = opt->constraint.string_list;
                for (int j = 0; list[j] != nullptr; ++j)
                {
                    QString s = QString::fromUtf8(list[j]);
                    caps.supportedSources.append(s);

                    if (s.contains("Transparency", Qt::CaseInsensitive))
                        caps.supportsTransparency = true;
                }
            }
        }
    }

    sane_close(handle);
    sane_exit();

    //qDebug() << "Found max/min dpi" << caps.maxDpi << "/" << caps.minDpi;
    //qDebug() << "16bit / Transparency" << caps.supports16Bit << "/" << caps.supportsTransparency; 
    //qDebug() << "All supported" << caps.supportedBitDepths;
    //qDebug() << "Rest supported" << caps.supportedResolutions;
    //qDebug() << "Sources" << caps.supportedSources << "\n";

    return caps;
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

void ScanWorker::requestCapabilities()
{
    ScannerCapabilities caps = readCapabilities();
    emit capabilitiesReady(caps);
}

QImage ScanWorker::performScan(const ScanParameters &params)
{
    sane_init(nullptr,nullptr);

    //qDebug() << ;

    SANE_Handle handle;
    if (sane_open(deviceName.toUtf8().constData(), &handle) != SANE_STATUS_GOOD)
        return {};

    SANE_Int resolution = params.dpi;
    setOptionByName(handle, "resolution", &resolution);
    setOptionByName(handle, "source", (void*)"Transparency Adapter");
    const char* mode = params.color ? "Color" : "Gray";

    sane_control_option(handle, 2, SANE_ACTION_SET_VALUE,
                       (void*)mode, nullptr);

    sane_start(handle);

    SANE_Parameters sp;
    sane_get_parameters(handle, &sp);

    int width = sp.pixels_per_line;
    int height = sp.lines;
    int bytesPerSample = sp.depth / 8;
    int channels = (sp.format == SANE_FRAME_RGB) ? 3 :
                   (sp.format == SANE_FRAME_GRAY) ? 1 : 0;

    // Only allow 16-bit scans for now
    if (sp.depth != 16 || channels == 0)
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
        // emit progress(percent);
    }

    sane_close(handle);
    sane_exit();

    if (status != SANE_STATUS_EOF || offset != totalBytes)
        return {};

    if (sp.format == SANE_FRAME_RGB)
    {
        std::vector<uint16_t> rgba(width * height * 4);
        uint16_t* src = reinterpret_cast<uint16_t*>(data.data());

        int samplesPerRow = sp.bytes_per_line / 2;
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

    if (sp.format == SANE_FRAME_GRAY)
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

void ScanWorker::requestPreview(const ScanParameters &params)
{
    int eta = ScanProfiles::estimateSeconds(deviceName, params.dpi);
    emit scanStarted(eta); // estimate

    QImage img = performScan(params);

    emit scanFinished(img);
}

void ScanWorker::requestScan(const ScanParameters &params)
{
    int eta = ScanProfiles::estimateSeconds(deviceName, params.dpi);
    emit scanStarted(eta); // estimate only

    QImage img = performScan(params);
    
    emit scanFinished(img);
}
