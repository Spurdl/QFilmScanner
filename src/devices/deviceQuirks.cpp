#include <toml++/toml.h>
#include "deviceQuirks.h"

DeviceQuirks loadProfile(QString path)
{
    DeviceQuirks q;

    auto tbl = toml::parse_file(path.toStdString());

    q.modelName = QString::fromStdString(
        tbl["modelName"].value_or("")
    );

    q.vendor = QString::fromStdString(
        tbl["vendor"].value_or("")
    );

    q.backend = QString::fromStdString(
        tbl["backend"].value_or("")
    );

    q.usbId = QString::fromStdString(
        tbl["usbId"].value_or("")
    );

    q.trueGray = tbl["trueGray"].value_or(false);
    q.rgbSensor = tbl["rgbSensor"].value_or(true);
    q.calibrationStrip = tbl["calibrationStrip"].value_or(0);

    std::string gm = tbl["grayMethod"].value_or("GREEN_CHANNEL");

    if (gm == "GREEN_CHANNEL")
        q.grayMethod = DeviceQuirks::GREEN_CHANNEL;

    if (gm == "RGB_WEIGHTED")
        q.grayMethod = DeviceQuirks::RGB_WEIGHTED;

    if (gm == "SCANNER_NATIVE")
        q.grayMethod = DeviceQuirks::SCANNER_NATIVE;

    if (auto dpi = tbl["dpiToSeconds"].as_table())
    {
        for (auto& [k,v] : *dpi)
        {
            int d = std::stoi(k.str());
            int t = v.value_or(0);
            q.dpiToSeconds[d] = t;
        }
    }

    return q;
}

const DeviceQuirks* findBestProfile(const QVector<DeviceQuirks>& db, QString backend, QString model, QString usbId) {
    int bestScore = 0;
    const DeviceQuirks* best = nullptr;

    for (const auto& p : db) {
        int score = 0;

        if (!p.backend.isEmpty() && backend.contains(p.backend, Qt::CaseInsensitive))
            score += 5;

        if (!p.modelName.isEmpty() && model.contains(p.modelName, Qt::CaseInsensitive))
            score += 10;

        if (!p.usbId.isEmpty() && usbId == p.usbId)
            score += 20;

        if (score > bestScore) {
            bestScore = score;
            best = &p;
        }
    }

    return best;
}