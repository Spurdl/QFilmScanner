#include "MainWindow.h"
#include "ScanWorker.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QInputDialog>
#include <QImage>
#include <QFrame>
#include <QDir>
#include <QMessageBox>
#include <QStatusBar>
#include <QKeySequence>
#include <algorithm>
#include <fstream>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QString("QFilmScanner  QFiSc 1.0.0"));
    auto *central = new QWidget;
    setCentralWidget(central);

    statusBar()->showMessage("Ready");

    imageLabel = new AspectRatioLabel();
    imageLabel->setMinimumSize(450, 300);
    imageLabel->setStyleSheet("background: black;");
    imageLabel->setAlignment(Qt::AlignCenter);

    previewBtn = new QPushButton("Preview");
    scanBtn = new QPushButton("Scan");
    nextBtn = new QPushButton(">");
    prevBtn = new QPushButton("<");
    deviceBtn = new QPushButton("Choose Device");
    folderBtn = new QPushButton("Choose Folder");
    colorswitch = new QCheckBox("Color");
    bitswitch = new QCheckBox("16 Bits");
    bitswitch->setChecked(true);
    bitswitch->setEnabled(false);
    frameLabel = new QLabel("Frame 000");
    frameLabel->setAlignment(Qt::AlignCenter);
   /* frameLabel->setStyleSheet(
        "color: white;"
        "background-color: #303030;"
        "font-weight: bold;"
        "font-size: 16px;"
        "padding: 6px;"
        "border-radius: 4px;"
    );*/

    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine);  
    line->setFrameShadow(QFrame::Raised); // Sunken / Raised / Plain
    line->setLineWidth(4);
    line->setFixedHeight(4); 

    QFrame *line1 = new QFrame;
    line1->setFrameShape(QFrame::HLine);  
    line1->setFrameShadow(QFrame::Raised); // Sunken / Raised / Plain
    line1->setLineWidth(4);
    line1->setFixedHeight(4); 
    
    QFrame *line2 = new QFrame;
    line2->setFrameShape(QFrame::HLine);  
    line2->setFrameShadow(QFrame::Raised); // Sunken / Raised / Plain
    line2->setLineWidth(4);
    line2->setFixedHeight(4); 

    progressBar = new QProgressBar;
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setVisible(true);

    timeLabel = new QLabel;
    timeLabel->setVisible(true);

    folderEdit = new QLineEdit;
    folderEdit->setReadOnly(true);
    deviceEdit = new QLineEdit;
    deviceEdit->setReadOnly(true);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTimeLabel);

    // TOP PANEL
    auto *topLayout = new QHBoxLayout;
    topLayout->setDirection(QBoxLayout::Direction::RightToLeft);
    topLayout->addWidget(frameLabel);
    topLayout->addWidget(nextBtn);
    topLayout->addWidget(prevBtn);
    topLayout->addWidget(timeLabel, 12);
    topLayout->addWidget(progressBar, 28);
    //topLayout->addWidget();

    // LEFT PANEL
    auto *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(deviceBtn);
    leftLayout->addWidget(deviceEdit);
    leftLayout->addWidget(folderBtn);
    leftLayout->addWidget(folderEdit);
    leftLayout->addSpacing(20);

    QWidget *leftWidget = new QWidget;
    leftWidget->setFixedWidth(260);
    leftWidget->setLayout(leftLayout);
    leftWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);  

    auto *together = new QHBoxLayout;
    together->addWidget(colorswitch);
    together->addWidget(bitswitch);

    leftLayout->addLayout(together);
    leftLayout->addSpacing(20);
    leftLayout->addWidget(previewBtn);
    leftLayout->addWidget(scanBtn);
    leftLayout->addStretch();

    // HORIZONTAL LAYOUT
    auto *horizontalLayout = new QHBoxLayout;
    horizontalLayout->setContentsMargins(0,0,0,0);
    horizontalLayout->addWidget(leftWidget);
    horizontalLayout->addWidget(imageLabel, 1);

    // VERTICAL LAYOUT
    auto *masterLayout = new QVBoxLayout;
    masterLayout->addWidget(line);
    masterLayout->addSpacing(2);
    masterLayout->addLayout(topLayout);
    masterLayout->addSpacing(2);
    masterLayout->addWidget(line1);
    masterLayout->addSpacing(2);
    masterLayout->addLayout(horizontalLayout);
    masterLayout->addSpacing(2);
    masterLayout->addWidget(line2);

    central->setLayout(masterLayout);

    // SHORTCUTS
    auto *leftShortcut = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(leftShortcut, &QShortcut::activated,
            this, &MainWindow::onPrev);
    shortcuts.append(leftShortcut);

    auto *rightShortcut = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(rightShortcut, &QShortcut::activated,
            this, &MainWindow::onNext);
    shortcuts.append(rightShortcut);

    auto *enterShortcut = new QShortcut(QKeySequence(Qt::Key_Return), this);
    connect(enterShortcut, &QShortcut::activated,
            this, &MainWindow::onScan);
    shortcuts.append(enterShortcut);

    auto *enterShortcut2 = new QShortcut(QKeySequence(Qt::Key_Enter), this);

    shortcuts.append(enterShortcut);

    // Worker thread
    worker = new ScanWorker;
    worker->moveToThread(&workerThread);

    connect(&workerThread, &QThread::finished,
            worker, &QObject::deleteLater);

    connect(worker, &ScanWorker::imageReady,
            this, &MainWindow::onImageReady);

    connect(worker, &ScanWorker::deviceListReady,
            this, &MainWindow::onDeviceListFound);

    connect(previewBtn, &QPushButton::clicked,
            this, &MainWindow::onPreview);

    connect(scanBtn, &QPushButton::clicked,
            this, &MainWindow::onScan);

    connect(nextBtn, &QPushButton::clicked,
            this, &MainWindow::onNext);

    connect(prevBtn, &QPushButton::clicked,
            this, &MainWindow::onPrev);

    connect(folderBtn, &QPushButton::clicked,
            this, &MainWindow::onChooseFolder);
    connect(deviceBtn, &QPushButton::clicked,
            this, &MainWindow::onChooseDevice);

    connect(enterShortcut2, &QShortcut::activated,
            this, &MainWindow::onScan);
    

    connect(worker, &ScanWorker::scanStarted,
            this, &MainWindow::onScanStarted);

    connect(worker, &ScanWorker::progress,
            this, &MainWindow::onProgress);

    connect(worker, &ScanWorker::scanFinished,
            this, &MainWindow::onScanFinished);

    updateFrameLabel();
    workerThread.start();
}

void MainWindow::updateTimeLabel()
{
    if (progressValue < 100)
    {
        progressValue += (100.0 / (maxTime+1)); 
        if (progressValue > 100) progressValue = 100.0;
        
    }

    if (estimatedSeconds > 0)
    {
        estimatedSeconds--;
    }

    int minutes = estimatedSeconds / 60;
    int seconds = estimatedSeconds % 60;

    int integer = std::floor(progressValue);
    progressBar->setValue(integer);

    timeLabel->setText(
        QString("Remaining: %1:%2")
        .arg(minutes)
        .arg(seconds, 2, 10, QChar('0'))
    );
}

void MainWindow::setBusy(bool busy)
{
    isBusy = busy;

    // Disable buttons
    previewBtn->setEnabled(!busy);
    scanBtn->setEnabled(!busy);
    nextBtn->setEnabled(!busy);
    prevBtn->setEnabled(!busy);
    folderBtn->setEnabled(!busy);
    colorswitch->setEnabled(!busy);

    // Disable shortcuts
    for (auto *sc : shortcuts)
        sc->setEnabled(!busy);

    if (busy)
        statusBar()->showMessage("Scanning...");
    else
        statusBar()->showMessage("Ready");
}

void MainWindow::onScanStarted(int estSec)
{
    setBusy(true);

    maxTime = estSec;
    progressBar->setValue(0);
    //progressBar->setVisible(true);
    //timeLabel->setVisible(true);

    estimatedSeconds = estSec;
    progressValue = 0;

    timer->start(1000);

    updateTimeLabel();
}

void MainWindow::onProgress(int percent)
{
    //progressBar->setValue(percent);
}

void MainWindow::onScanFinished()
{
    timer->stop();
    progressValue=100.0;
    estimatedSeconds=0;
    updateTimeLabel();
    //progressBar->setVisible(false);
    //timeLabel->setVisible(false);

    setBusy(false);
}

void MainWindow::updateFrameLabel()
{
    frameLabel->setText(
        QString("Frame %1").arg(frameIndex, 3, 10, QChar('0'))
    );
}

MainWindow::~MainWindow()
{
    workerThread.quit();
    workerThread.wait();
}

void MainWindow::onChooseFolder()
{
    folderBtn->setText("Choosing");
    QString dir = QFileDialog::getExistingDirectory(
        this, "Select Output Folder");

    if (!dir.isEmpty())
    {
        outputFolder = dir;
        folderEdit->setText(dir);
        folderBtn->setText("Choose Folder");
        folderBtn->setStyleSheet("");
    }
    else{
        outputFolder = "";
        folderEdit->setText("");
        folderBtn->setText("Choose Folder");
        folderBtn->setStyleSheet("");
    }
}

void MainWindow::onPreview()
{
    if (worker->getDeviceName().isEmpty()){
        deviceBtn->setStyleSheet(
            "border: 2px solid red;"
            "background-color: #330000;"
            "color: white;"
            );
        statusBar()->showMessage("No scanner selected.", 5000);
        return;
    }
    bool colorstate = colorswitch->checkState();
    QMetaObject::invokeMethod(worker, "doPreview",
        Q_ARG(bool, colorstate));
    return;
}

void MainWindow::onDeviceListFound(QStringList devices){
    deviceBtn->setEnabled(true);

    if (devices.isEmpty())
        return;

    // Extract display labels only
    QStringList displayNames;
    for (const QString &d : devices)
        displayNames << d.section('|',1,1);
    bool ok;
    QString selectedLabel = QInputDialog::getItem(
        this,
        "Select Scanner",
        "Device:",
        displayNames,
        0,
        false,
        &ok);

    if (ok && !selectedLabel.isEmpty())
    {
        int index = displayNames.indexOf(selectedLabel);
        QString deviceId = devices[index].section('|',0,0);

        worker->setDeviceName(deviceId);
        deviceEdit->setText(selectedLabel);
        deviceBtn->setText("Choose Device");
        deviceBtn->setStyleSheet("");
    }
    else{
        worker->setDeviceName("");
        deviceEdit->setText("");
        deviceBtn->setText("Choose Device");
        deviceBtn->setStyleSheet("");
    }
    return;
}

void MainWindow::onScan()
{
    bool fail = false;
    if (worker->getDeviceName().isEmpty()){
        deviceBtn->setStyleSheet(
            "border: 2px solid red;"
            "background-color: #330000;"
            "color: white;"
            );
        statusBar()->showMessage("No scanner selected.", 5000);
        fail = true;
    }
    if (outputFolder.isEmpty()){
        folderBtn->setStyleSheet(
            "border: 2px solid red;"
            "background-color: #330000;"
            "color: white;"
            );
        statusBar()->showMessage("Error: Please choose an output folder first.", 5000);
        fail = true;
    }
    if(fail){return;}
    bool colorstate = colorswitch->checkState();
    QMetaObject::invokeMethod(worker, "doScan",
        Q_ARG(int, frameIndex),
        Q_ARG(bool, colorstate),
        Q_ARG(QString, outputFolder));
}

void MainWindow::onNext()
{
    frameIndex++;
    updateFrameLabel();
}

void MainWindow::onPrev()
{
    if (frameIndex > 0)
        frameIndex--;
    updateFrameLabel();
}

void MainWindow::onChooseDevice()
{
    deviceBtn->setEnabled(false);   // prevent double click
    worker->setDeviceName("");
    QMetaObject::invokeMethod(worker, "requestDeviceList");
    deviceBtn->setText("Searching...");
    deviceEdit->setText("");
    folderBtn->setStyleSheet("");
}

void MainWindow::onImageReady(const QImage &img)
{
    imageLabel->setPixmap(QPixmap::fromImage(img)
        .scaled(imageLabel->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation));
}
