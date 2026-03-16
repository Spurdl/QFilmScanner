#pragma once

#include <QDebug>

#define SCAN_DEBUG 1

#if SCAN_DEBUG
#define DBG qDebug()
#else
#define DBG if(false) qDebug()
#endif