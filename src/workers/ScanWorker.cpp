#include <algorithm>
#include <vector>
#include <cstdint>
#include <fstream>
#include <sane/sane.h>

#include <QtEndian>
#include "ScanWorker.h"

#define WEIGHT_R 0.299
#define WEIGHT_G 0.587
#define WEIGHT_B 0.114 

ScanWorker::ScanWorker(QObject *parent)
    : QObject(parent) {}

// Helpers

static uint16_t convertGray(uint16_t r, uint16_t g, uint16_t b, const DeviceQuirks* p)
{
    switch (p->grayMethod){
        case DeviceQuirks::GREEN_CHANNEL:
            return g;
        case DeviceQuirks::RGB_WEIGHTED:
            return (uint16_t)(WEIGHT_R * r + WEIGHT_G * g + WEIGHT_B * b);
        default:
            return g;
    };
}

static bool setOptionByName(SANE_Handle handle, const char* name, void* value){
    for (int i = 0;; ++i) {
        const SANE_Option_Descriptor* opt = sane_get_option_descriptor(handle, i);
        if (!opt) {
            break;
        }
        if (opt->name && strcmp(opt->name, name) == 0) {
            SANE_Status st = sane_control_option(handle, i, SANE_ACTION_SET_VALUE, value, nullptr);
            return (st == SANE_STATUS_GOOD);
        }
    }
    DBG << "Option not found:" << name;
    return false;
}

// Device calls

ScannerCapabilities ScanWorker::readCapabilities()
{
    ScannerCapabilities caps;
    sane_init(nullptr,nullptr);

    SANE_Handle handle;
    if (sane_open(deviceName.toUtf8().constData(), &handle) != SANE_STATUS_GOOD)
        return caps;

    for (int i = 0;; ++i)
    {
        const SANE_Option_Descriptor* opt = sane_get_option_descriptor(handle, i);

        if (!opt) {
            break;}
        if (!opt->name) {
            continue;}

        if (strcmp(opt->name, "resolution") == 0) {
            if (opt->constraint_type == SANE_CONSTRAINT_RANGE) {
                const SANE_Range* r = opt->constraint.range;
                caps.minDpi = r->min;
                caps.maxDpi = r->max;

                for (int v = r->min; v <= r->max; v += r->quant) {
                    caps.supportedResolutions.append(v);
                }
            }
            else if (opt->constraint_type == SANE_CONSTRAINT_WORD_LIST) {
                int count = opt->constraint.word_list[0];
                for (int j = 1; j <= count; ++j){
                    caps.supportedResolutions.append(opt->constraint.word_list[j]);
                }

                if (!caps.supportedResolutions.isEmpty()) {
                    caps.minDpi = *std::min_element(caps.supportedResolutions.begin(),
                                          caps.supportedResolutions.end());

                    caps.maxDpi = *std::max_element(caps.supportedResolutions.begin(),
                                          caps.supportedResolutions.end());
                }
            }
        }

        if (strcmp(opt->name, "depth") == 0) {
            if (opt->constraint_type == SANE_CONSTRAINT_WORD_LIST) {
                int count = opt->constraint.word_list[0];
                for (int j = 1; j <= count; ++j) {
                    int d = opt->constraint.word_list[j];
                    caps.supportedBitDepths.append(d);
                    if(d == 8){
                        caps.bitDepth = static_cast<ScannerCapabilities::BitDepths>(caps.bitDepth | ScannerCapabilities::ONLY_8_BIT);
                    }
                    else if(d == 16){
                        caps.bitDepth = static_cast<ScannerCapabilities::BitDepths>(caps.bitDepth | ScannerCapabilities::ONLY_16_BIT);
                    }
                }
            }
        }

        if (strcmp(opt->name, "source") == 0) {
            if (opt->constraint_type == SANE_CONSTRAINT_STRING_LIST) {
                const SANE_String_Const* list = opt->constraint.string_list;
                for (int j = 0; list[j] != nullptr; ++j) {
                    QString s = QString::fromUtf8(list[j]);
                    caps.supportedSources.append(s);

                    if (s.contains("Transparen", Qt::CaseInsensitive) || s.contains("TPU", Qt::CaseInsensitive)){   // Transparen - cy or -t Unit etc
                        caps.filmSource = s;
                        caps.supportsTransparency = true;
                    }
                    else{
                        caps.stdSource = s;
                    }
                }
            }
        }
    }

    sane_close(handle);
    sane_exit();



    return caps;
}

QImage ScanWorker::performScan(const ScanParameters &params)
{
    sane_init(nullptr,nullptr);

    SANE_Handle handle;

    if (sane_open(deviceName.toUtf8().constData(), &handle) != SANE_STATUS_GOOD){
        DBG << "Error opening device";
        return {};
    }

    // Params
    const int bytecount = params.bitDepth / 8;
    SANE_Int resolution = params.dpi;
    QByteArray src = params.source.toUtf8();
    setOptionByName(handle, "resolution", &resolution);
    setOptionByName(handle, "source", src.data());
    const char* gray;
    if(params.cap->quirks->grayMethod != DeviceQuirks::GrayMethod::SCANNER_NATIVE){
        gray = "Color";
    } else{
        gray = "Gray";
    }
    const char* mode = params.color ? "Color" : gray;

    sane_control_option(handle, 2, SANE_ACTION_SET_VALUE,
                       (void*)mode, nullptr);

    // Scanning
    sane_start(handle);

    SANE_Parameters sp;
    sane_get_parameters(handle, &sp);

    int width = sp.pixels_per_line;
    int height = sp.lines;
    int bytesPerSample = sp.depth / 8;
    int channels = (sp.format == SANE_FRAME_RGB) ? 3 : 1; 

    size_t totalBytes = width * height * channels * bytesPerSample;
    std::vector<uint8_t> data(totalBytes);

    SANE_Status status;
    SANE_Int len;
    size_t offset = 0;

    // SCAN !!!
    while ((status = sane_read(handle, data.data() + offset, data.size() - offset, &len)) == SANE_STATUS_GOOD) {
        offset += len;
        int percent = (offset * 100) / totalBytes;
    }

    sane_close(handle);
    sane_exit();

    if (status != SANE_STATUS_EOF || offset != totalBytes) {
        DBG << "NO IMAGE OR ERROR DURING SCANNING";
        return {};
    }

    // Image finalization

    QImage img;

    if (sp.format == SANE_FRAME_RGB){
        DBG << "FOUND RGB IMAGE";
        std::vector<uint16_t> rgba(width * height * 4);
        uint16_t* src = reinterpret_cast<uint16_t*>(data.data());

        int samplesPerRow = sp.bytes_per_line / bytecount;

        if (params.color) 
        {
            std::vector<uint16_t> rgba(width * height * 4);
            for (int y = 0; y < height; ++y) {
                uint16_t* row = src + y * samplesPerRow;
                for (int x = 0; x < width; ++x) {
                    rgba[(y*width + x)*4 + 0] = row[x*3 + 0];
                    rgba[(y*width + x)*4 + 1] = row[x*3 + 1];
                    rgba[(y*width + x)*4 + 2] = row[x*3 + 2];
                    rgba[(y*width + x)*4 + 3] = 65535;
                }
            }
            img = QImage(reinterpret_cast<const uchar*>(rgba.data()), width, height, QImage::Format_RGBX64);
        }

        else 
        {
            img = QImage(width, height, QImage::Format_Grayscale16);
            for (int y = 0; y < height; ++y) {
                uint16_t* srcRow = src + y * samplesPerRow;
                uint16_t* dstRow = reinterpret_cast<uint16_t*>(img.scanLine(y));
                for (int x = 0; x < width; ++x) {
                    dstRow[x] = convertGray(
                        srcRow[x*3 + 0],
                        srcRow[x*3 + 1],
                        srcRow[x*3 + 2],
                        params.cap->quirks
                    );
                }
            }
        }

    } else if (sp.format == SANE_FRAME_GRAY) {
        DBG << "FOUND GRAYSCALE IMAGE";

        std::vector<uint16_t> gray(width * height);
        uint16_t* src = reinterpret_cast<uint16_t*>(data.data());

        int samplesPerRow = sp.bytes_per_line / bytecount;
        img = QImage(width, height, QImage::Format_Grayscale16);
        for (int y = 0; y < height; ++y) {
            uint16_t* srcRow = src + y * samplesPerRow;
            uint16_t* dstRow = reinterpret_cast<uint16_t*>(img.scanLine(y));
            for (int x = 0; x < width; ++x) {
                dstRow[x] = convertGray(
                    srcRow[x*3 + 0],
                    srcRow[x*3 + 1],
                    srcRow[x*3 + 2],
                    params.cap->quirks
                );
            }
        }
    } else{
        return {};
    }
    const int calibration_stripe = params.cap->quirks->calibrationStrip;; // hardware stripe width
    if(calibration_stripe != 0){
        if (width > calibration_stripe) {
            QImage cropped = img.copy(calibration_stripe, 0, width - calibration_stripe, height);
            img = cropped;
            width -= calibration_stripe;
        }
    }
    return img.copy();
}

// Public Slots

void ScanWorker::requestPreview(const ScanParameters &params)
{
    //int eta = ScanProfiles::estimateSeconds(deviceName, params.dpi);
    emit scanStarted(20); // estimate

    QImage img = performScan(params);

    emit previewFinished(img);
}

void ScanWorker::requestScan(const ScanParameters &params)
{
    //int eta = ScanProfiles::estimateSeconds(deviceName, params.dpi);
    emit scanStarted(75); // estimate only

    QImage img = performScan(params);
    
    emit scanFinished(img);
}

void ScanWorker::requestCapabilities() {
    ScannerCapabilities caps = readCapabilities();
    emit capabilitiesReady(caps);
}

void ScanWorker::requestDeviceList() {
    sane_init(nullptr,nullptr);

    const SANE_Device **device_list = nullptr;
    sane_get_devices(&device_list,SANE_FALSE);

    QStringList devices;

    if(device_list) {
        for(int i=0; device_list[i]; ++i) {
            QString id = device_list[i]->name;
            QString label;

            if (device_list[i]->vendor && device_list[i]->model){
                label = QString("%1 %2")
                            .arg(device_list[i]->vendor)
                            .arg(device_list[i]->model);
            } else {
                label = id;  // fallback if no nice name
            }
            devices << QString("%1|%2").arg(id).arg(label);
        }
    }
    sane_exit();

    emit deviceListReady(devices);
}