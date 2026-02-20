#pragma once

#include <QString>
#include <QMap>

struct ScanSpeedProfile
{
    QString modelName;             // e.g. "Plustek 8200i"
    QMap<int,int> dpiToSeconds;    // dpi -> estimated seconds
};

class ScanProfiles
{
public:
    static int estimateSeconds(const QString& deviceName,
                               int dpi)
    {
        const auto& profiles = allProfiles();

        for (const ScanSpeedProfile& p : profiles)
        {
            if (deviceName.contains(p.modelName,
                                    Qt::CaseInsensitive))
            {
                if (p.dpiToSeconds.contains(dpi))
                    return p.dpiToSeconds[dpi];

                return closestEstimate(p.dpiToSeconds, dpi);
            }
        }

        return genericEstimate(dpi);
    }

private:
    static QList<ScanSpeedProfile> allProfiles()
    {
        return {
            {
                "Plustek",
                {
                    { 900,  20 },
                    { 1200, 25 },
                    { 2400, 40 },
                    { 4800, 70 },
                    { 7200, 115 }
                }
            },
            {
                "Epson",
                {
                    { 1200, 15 },
                    { 2400, 30 },
                    { 4800, 60 },
                    { 6400, 90 }
                }
            }
        };
    }

    static int closestEstimate(const QMap<int,int>& map,
                               int dpi)
    {
        int closest = map.firstKey();
        for (int key : map.keys())
        {
            if (abs(key - dpi) < abs(closest - dpi))
                closest = key;
        }
        return map[closest];
    }

    static int genericEstimate(int dpi)
    {
        return dpi / 50; // fallback formula :)
    }
};