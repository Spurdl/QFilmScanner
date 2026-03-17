#include "deviceQuirks.h"

#include <toml++/toml.h>
#include <QDir>
#include <QHash>
#include <QString>
#include <QFile>
#include "../helpers/helpers.h"
#include "deviceQuirks.h"

QHash<QString, DeviceQuirks> DeviceQuirkDatabase::quirksByFriendlyName;
QHash<QString, DeviceQuirks> DeviceQuirkDatabase::defaultByVendor;

bool DeviceQuirkDatabase::findByFriendlyName(const QString& name, DeviceQuirks& out) {
    DBG << "Searching for " << name;
    if (quirksByFriendlyName.contains(name)) {
        out = quirksByFriendlyName[name];
        return true;
    }
    return false;
}

void DeviceQuirkDatabase::loadProfileFile(const QString& path)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)){
        return;
    }

    QByteArray data = file.readAll();

    toml::table tbl = toml::parse(std::string(data.constData(), data.size()));

    QString manufacturer = QString::fromStdString(tbl["manufacturer"].value_or(""));
    QString vendorId = QString::fromStdString(tbl["vendorId"].value_or(""));

    //  default section 
    if (auto def = tbl["default"].as_table()) {
        DBG << "Default found!";
        DeviceQuirks q;

        q.manufacturer = manufacturer;
        q.modelName = QString::fromStdString(def->at("modelName").value_or("Generic"));

        q.trueGray = def->at("trueGray").value_or(false);
        q.rgbSensor = def->at("rgbSensor").value_or(true);
        q.calibrationStrip = def->at("calibrationStrip").value_or(0);

        QString gray = QString::fromStdString(def->at("grayMethod").value_or("rgb_weighted"));

        if (gray == "green_channel"){
            q.grayMethod = DeviceQuirks::GREEN_CHANNEL;
        } else if (gray == "scanner_native") {
            q.grayMethod = DeviceQuirks::SCANNER_NATIVE;
        } else {   
            q.grayMethod = DeviceQuirks::RGB_WEIGHTED;
        }
        if (auto dpiTable = def->get_as<toml::table>("dpiToSeconds")) {
            for (auto&& [k,v] : *dpiTable) {
                int dpi = std::stoi(std::string(k.str()));
                int sec = v.value_or(0);
                q.dpiToSeconds.insert(dpi, sec);
            }
        }
        defaultByVendor.insert(vendorId, q);
    }

    // ---- devices ----
    if (auto devArray = tbl["device"].as_array()) {
        for (auto&& node : *devArray) {
            auto dev = *node.as_table();

            DeviceQuirks q;

            q.manufacturer = manufacturer;
            q.modelName = QString::fromStdString(dev["modelName"].value_or(""));

            q.trueGray = dev["trueGray"].value_or(false);
            q.rgbSensor = dev["rgbSensor"].value_or(true);
            q.calibrationStrip = dev["calibrationStrip"].value_or(0);

            QString gray = QString::fromStdString(dev["grayMethod"].value_or("rgb_weighted"));

            if (gray == "green_channel"){
                q.grayMethod = DeviceQuirks::GREEN_CHANNEL;
            } else if (gray == "scanner_native") {
                q.grayMethod = DeviceQuirks::SCANNER_NATIVE;
            } else if (gray == "rgb_median") {
                q.grayMethod = DeviceQuirks::RGB_MEDIAN;
            } else {
                q.grayMethod = DeviceQuirks::RGB_WEIGHTED;
            }
            if (auto dpiTable = dev.get_as<toml::table>("dpiToSeconds")) {
                for (auto&& [k,v] : *dpiTable) {
                    int dpi = std::stoi(std::string(k.str()));
                    int sec = v.value_or(0);
                    q.dpiToSeconds.insert(dpi, sec);
                }
            }
            
            if (auto names = dev["names"].as_array()) {
                for (auto&& n : *names) {
                    QString friendly = QString::fromStdString(n.value_or(""));
                    q.friendlyName = friendly;
                    quirksByFriendlyName.insert(friendly, q);

                    DBG << "Model found: " << friendly;
                }
            }
        }
    }
    return;
}

void DeviceQuirkDatabase::loadProfiles()
{
    QDir dir(":/profiles");

    QStringList files = dir.entryList({"*.toml"});

    for (const QString& f : files)
    {   
        DBG << "Loaded profile " << f;
        loadProfileFile(dir.filePath(f));
    }
    return;
}