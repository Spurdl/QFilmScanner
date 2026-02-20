#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QThread>
#include <QProgressBar>
#include <QShortcut>
#include <QTimer>
#include <QComboBox>

#include "AspectRatioLabel.h"

#include "workers/ScanWorker.h"
#include "workers/ImageWorker.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
// UI
    AspectRatioLabel *imageLabel;

    QProgressBar *progressBar;
    QLabel *timeLabel;

    QPushButton *previewBtn;
    QPushButton *scanBtn;
    QPushButton *nextBtn;
    QPushButton *prevBtn;
    QPushButton *folderBtn;
    QPushButton *deviceBtn;

    QCheckBox *colorswitch;
    QCheckBox *bitswitch;
    QLabel *frameLabel;
    QLineEdit *deviceEdit;
    QLineEdit *folderEdit;

    QComboBox *previewDpiBox;
    QComboBox *scanDpiBox;

    QCheckBox *slideSwitch;
    QCheckBox *flipSwitch;
    QCheckBox *upsideSwitch;
    QCheckBox *rawSwitch;

    QList<QShortcut*> shortcuts;
// Internal state
    ScanParameters currentParams;
    ScannerCapabilities currentCaps;

    bool isBusy = false;
    bool deviceReady = false;
    int maxTime;
    int estimatedSeconds;
    int frameIndex = 0;
    float progressValue;
    
    int previewDpi = 0;
    int scanDpi = 0;

    QString outputFolder;

// Image itself
    QImage rawImage;

    QTimer *timer;

    QThread workerThread[2];
    ScanWorker *scanWorker;
    ImageWorker *editWorker;

private slots:
// buttons
    void onPreview();
    void onScan();
    void onNext();
    void onPrev();
    void onChooseFolder();
    void onChooseDevice();

// helpers
    void updateFrameLabel();
    void updateTimeLabel();
    void setProgressReadyStyle();
    void setProgressScanningStyle();
    void callPreviewUpdater();
    void setBusy(bool busy);

//signal handlers
    void onDeviceListFound(QStringList devices);
    void onCapabilitiesReady(const ScannerCapabilities &caps);
    void onScanStarted(int estimatedSeconds);
    void onScanFinished(const QImage &img);
    void onImageReady(const QImage &img);
};
