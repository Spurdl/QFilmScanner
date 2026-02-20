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
                    { 900,  32 },
                    { 1200, 45 },
                    { 2400, 55 },
                    { 4800, 85 },
                    { 7200, 120 }
                }
            },
            {
                "Epson",
                {
                    { 1200, 37 },
                    { 2400, 50 },
                    { 4800, 95 },
                    { 6400, 120 }
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