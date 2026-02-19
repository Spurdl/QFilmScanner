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
#include "AspectRatioLabel.h"
#include "ScanWorker.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
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

    int frameIndex = 0;

    QTimer *timer;
    float progressValue;
    int maxTime;
    int estimatedSeconds;

    bool colorMode = true;
    QString outputFolder;

    QThread workerThread;
    ScanWorker *worker;

    bool isBusy = false;
    QList<QShortcut*> shortcuts;

private slots:
    void updateFrameLabel();
    void updateTimeLabel();
    void setBusy(bool busy);
    void onPreview();
    void onScan();
    void onNext();
    void onPrev();
    void onChooseFolder();
    void onImageReady(const QImage &img);
    void onChooseDevice();
    void onDeviceListFound(QStringList devices);
    void onScanStarted(int estimatedSeconds);
    void onProgress(int percent);
    void onScanFinished();
};
