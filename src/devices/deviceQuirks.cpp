#include <toml++/toml.h>
#include <QDir>
#include "deviceQuirks.h"

void DeviceQuirkDatabase::loadDirectory(const QString& path)
{
    QDir dir(path);
    for (auto file : dir.entryList({"*.toml"}, QDir::Files)) 
    {
        auto table = toml::parse_file(dir.filePath(file).toStdString());
        auto devices = table["device"].as_array();
        if (!devices) continue;

        for (auto&& dev : *devices)
        {
            DeviceQuirks q = parseDevice(*dev.as_table());

            if (!q.usbId.isEmpty())
                usbMap[q.usbId] = q;

            if (!q.modelName.isEmpty())
                modelMap[q.modelName] = q;
        }
    }
}