#include <algorithm>
#include <fstream>
#include <cmath>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QInputDialog>
#include <QImage>
#include <QFrame>
#include <QDir>
#include <QMessageBox>
#include <QStatusBar>
#include <QKeySequence>
#include <QGuiApplication>
#include <QStyleHints>

#include "MainWindow.h"
#include "workers/ScanWorker.h"
#include "workers/ImageWorker.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QString("QFilmScanner  /  QFiSc"));
    auto *central = new QWidget;
    setCentralWidget(central);

    bool dark = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;

    qRegisterMetaType<ScanParameters>("ScanParameters");
    qRegisterMetaType<ScannerCapabilities>("ScannerCapabilities");

    statusBar()->showMessage("Ready");

    imageLabel = new AspectRatioLabel();
    imageLabel->setMinimumSize(450, 300);
    imageLabel->setAlignment(Qt::AlignCenter);

    previewBtn = new QPushButton("Preview");
    scanBtn = new QPushButton("Scan");
    nextBtn = new QPushButton(">");
    prevBtn = new QPushButton("<");
    deviceBtn = new QPushButton("Choose Device");
    folderBtn = new QPushButton("Choose Folder");
    saveAgain = new QPushButton("Save Again");
    saveAgain->setEnabled(false);
    colorswitch = new QCheckBox("Color");
    bitswitch = new QCheckBox("16 Bits");
    slideSwitch = new QCheckBox("R");
    flipSwitch = new QCheckBox("↻");
    mirrorSwitch = new QCheckBox("⇄");
    upsideSwitch = new QCheckBox("⇅");
    rawSwitch = new QCheckBox("⎘");
    bitswitch->setChecked(true);
    bitswitch->setEnabled(false);
    frameLabel = new QLabel("Frame 000");
    frameLabel->setAlignment(Qt::AlignCenter);

    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine);  
    line->setFrameShadow(QFrame::Raised);
    line->setLineWidth(4);
    line->setFixedHeight(4); 

    QFrame *line1 = new QFrame;
    line1->setFrameShape(QFrame::HLine);  
    line1->setFrameShadow(QFrame::Raised);
    line1->setLineWidth(4);
    line1->setFixedHeight(4); 
    
    QFrame *line2 = new QFrame;
    line2->setFrameShape(QFrame::HLine);  
    line2->setFrameShadow(QFrame::Raised);
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

    previewDpiBox = new QComboBox;
    scanDpiBox = new QComboBox;

    previewDpiBox->setEnabled(false);
    scanDpiBox->setEnabled(false);

    saveType = new QComboBox;
    saveMethod = new QComboBox;
    saveType->addItems(supportedSaveTypes);
    saveMethod->addItems(supportedSaveMethods);
    saveType->setCurrentIndex(2);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTimeLabel);

    // TOP PANEL
    auto *topLayout = new QHBoxLayout;
    topLayout->setDirection(QBoxLayout::Direction::RightToLeft);
    topLayout->addWidget(frameLabel, 12);
    topLayout->addWidget(nextBtn, 4);
    topLayout->addWidget(prevBtn, 4);
    topLayout->addWidget(timeLabel, 30);
    topLayout->addWidget(progressBar, 50);
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

    // DPI SELECTION
    auto *togetherToo = new QHBoxLayout;
    auto *UpDown = new QVBoxLayout;
    auto *UpDownToo = new QVBoxLayout;
    UpDown->addWidget(new QLabel("Preview DPI"));
    UpDown->addWidget(previewDpiBox);
    UpDownToo->addWidget(new QLabel("Scan DPI"));
    UpDownToo->addWidget(scanDpiBox);
    
    togetherToo->addLayout(UpDown);
    togetherToo->addLayout(UpDownToo);

    // SAVE SELECTION
    auto *saveLayout = new QGridLayout;
    saveLayout->addWidget(new QLabel("Save method"),0,0);
    saveLayout->addWidget(new QLabel("File type")  ,0,1);
    saveLayout->addWidget(saveMethod,1,0);
    saveLayout->addWidget(saveType,1,1);

    leftLayout->addLayout(together);
    leftLayout->addSpacing(10);
    leftLayout->addLayout(togetherToo);
    leftLayout->addStretch();
    leftLayout->addLayout(saveLayout);
    leftLayout->addWidget(previewBtn);
    leftLayout->addWidget(scanBtn);
    leftLayout->addWidget(saveAgain);
    leftLayout->addSpacing(15);

    // IMAGE EDIT LAYOUT
    auto *imageLayout = new QHBoxLayout;
    auto *friends = new QVBoxLayout;
    imageLayout->addWidget(slideSwitch);
    imageLayout->addSpacing(40);
    imageLayout->addWidget(flipSwitch);
    imageLayout->addSpacing(40);
    imageLayout->addWidget(upsideSwitch);
    imageLayout->addSpacing(40);
    imageLayout->addWidget(mirrorSwitch);
    imageLayout->addSpacing(40);
    imageLayout->addWidget(rawSwitch);
    imageLayout->setAlignment(Qt::AlignCenter);
    friends->addWidget(imageLabel);
    friends->addLayout(imageLayout);    

    // HORIZONTAL LAYOUT
    auto *horizontalLayout = new QHBoxLayout;
    horizontalLayout->setContentsMargins(0,0,0,0);
    horizontalLayout->addWidget(leftWidget);
    horizontalLayout->addLayout(friends, 1);

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

    // FOOTER
    auto *footerLayout = new QHBoxLayout;
    footerLayout->setContentsMargins(5, 2, 5, 2);
    footerLayout->addStretch();
    
    QLabel *githubLabel = new QLabel("<a href=\"https://github.com/Spurdl/QFilmScanner\" "
                                     "style=\"color: gray; text-decoration: none; font-size:9pt;\">Github ↗</a>");
    githubLabel->setTextFormat(Qt::RichText);
    githubLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    githubLabel->setOpenExternalLinks(true);
    githubLabel->setFixedHeight(16); 

    footerLayout->addWidget(githubLabel);

    QLabel *versionLabel = new QLabel("v1.0.0");
    versionLabel->setStyleSheet("color: gray; font-size: 9pt;");
    footerLayout->addSpacing(40);
    footerLayout->addWidget(versionLabel);

    QWidget *footerWidget = new QWidget;
    footerWidget->setLayout(footerLayout);
    footerWidget->setStyleSheet("background-color: #fafafa;");
    footerWidget->setFixedHeight(18); 

    masterLayout->addWidget(footerWidget);

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
    scanWorker = new ScanWorker;
    editWorker = new ImageWorker;
    scanWorker->moveToThread(&workerThread[0]);
    editWorker->moveToThread(&workerThread[1]);

    connect(&workerThread[0], &QThread::finished,
            scanWorker, &QObject::deleteLater);

    connect(&workerThread[1], &QThread::finished,
        editWorker, &QObject::deleteLater);

    connect(editWorker, &ImageWorker::imageProcessed,
        this, &MainWindow::onImageReady);

    connect(scanWorker, &ScanWorker::deviceListReady,
            this, &MainWindow::onDeviceListFound);

    connect(editWorker, &ImageWorker::saveComplete,
            this, &MainWindow::onSaveComplete);

    connect(saveAgain, &QPushButton::clicked,
            this, &MainWindow::onSaveRequest);

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

    connect(scanWorker, &ScanWorker::capabilitiesReady,
        this, &MainWindow::onCapabilitiesReady);

    connect(scanWorker, &ScanWorker::scanStarted,
            this, &MainWindow::onScanStarted);

    connect(scanWorker, &ScanWorker::scanFinished,
            this, &MainWindow::onScanFinished);

    connect(scanWorker, &ScanWorker::previewFinished,
        this, &MainWindow::onPreviewFinished);

    connect(previewDpiBox, &QComboBox::currentIndexChanged,
            this, [this](int index)
    {
        previewDpi =
            previewDpiBox->itemData(index).toInt();
    });

    connect(scanDpiBox, &QComboBox::currentIndexChanged,
            this, [this](int index)
    {
        scanDpi =
            scanDpiBox->itemData(index).toInt();
    });

    connect(flipSwitch, &QCheckBox::checkStateChanged, 
            this, &MainWindow::callPreviewUpdater);
    connect(slideSwitch, &QCheckBox::checkStateChanged, 
            this, &MainWindow::callPreviewUpdater);
    connect(upsideSwitch, &QCheckBox::checkStateChanged, 
            this, &MainWindow::callPreviewUpdater);
    connect(mirrorSwitch, &QCheckBox::checkStateChanged, 
            this, &MainWindow::callPreviewUpdater);
    connect(rawSwitch, &QCheckBox::checkStateChanged, 
            this, &MainWindow::callPreviewUpdater);


    updateLabels();
    workerThread[0].start();
    workerThread[1].start();
}

MainWindow::~MainWindow()
{
    workerThread[0].quit();
    workerThread[0].wait();
    workerThread[1].quit();
    workerThread[1].wait();
}

// ------ HELPERS

void MainWindow::setProgressScanningStyle()
{
    progressBar->setStyleSheet(
        "QProgressBar {"
        "  border: 1px solid #444;"
        "  border-radius: 3px;"
        "  text-align: center;"
        "}"
        "QProgressBar::chunk {"
        "  background-color: #2a82da;"   // blue
        "}"
    );
}

void MainWindow::setProgressReadyStyle()
{
    progressBar->setStyleSheet(
        "QProgressBar {"
        "  border: 1px solid #444;"
        "  border-radius: 3px;"
        "  text-align: center;"
        "}"
        "QProgressBar::chunk {"
        "  background-color: #28a745;"   // green
        "}"
    );
}

void MainWindow::setBusy(bool busy)
{
    isBusy = busy;

    for (auto *sc : shortcuts)
        sc->setEnabled(!busy);

    if (busy)
        statusBar()->showMessage("Scanning...");
    else
        statusBar()->showMessage("Ready");
    updateLabels();
}

void MainWindow::updateLabels()
{
    previewBtn->setEnabled(!isBusy);
    scanBtn->setEnabled(!isBusy);
    nextBtn->setEnabled(!isBusy);
    prevBtn->setEnabled(!isBusy);
    folderBtn->setEnabled(!isBusy);
    colorswitch->setEnabled(!isBusy);

    if(!rawImage.isNull()){
        saveAgain->setEnabled(true);
    } else {
        saveAgain->setEnabled(false);
    }

    frameLabel->setText(
        QString("Frame %1").arg(frameIndex, 3, 10, QChar('0'))
    );
}

void MainWindow::updateTimeLabel()
{
    if (progressValue < 100){
        progressValue += (100.0 / (maxTime+1)); 
        if (progressValue > 100) progressValue = 100.0;
    }

    if (estimatedSeconds > 0){
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

// ----/ HELPERS
// ---- BUTTONS

void MainWindow::onSaveRequest(){

    ImageEditParams params;

    params.turn = flipSwitch->checkState();
    params.invert = !slideSwitch->checkState();
    params.mirror_v = upsideSwitch->checkState();
    params.mirror_h = mirrorSwitch->checkState();
    params.autolevel = !rawSwitch->checkState();
    params.saveMethod = saveMethod->currentText();
    params.saveType = saveType->currentText();

    QMetaObject::invokeMethod(editWorker,
                    "saveImage",
                    Q_ARG(QImage, rawImage),
                    Q_ARG(QString, outputFolder),
                    Q_ARG(int, frameIndex),
                    Q_ARG(ImageEditParams, params));

    statusBar()->showMessage(
        QString("Saving image..."),
        5000);
    
    return;
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

void MainWindow::onScan()
{
    if (scanWorker->getDeviceName().isEmpty()){
        deviceBtn->setStyleSheet(
            "border: 2px solid red;"
            "background-color: #330000;"
            "color: white;"
            );
        statusBar()->showMessage("No scanner selected.", 5000);
        return;
    }
    if (outputFolder.isEmpty()){
        folderBtn->setStyleSheet(
            "border: 2px solid red;"
            "background-color: #330000;"
            "color: white;"
            );
        statusBar()->showMessage("Error: Please choose an output folder first.", 5000);
        return;
    }

    currentParams.color = colorswitch->isChecked();
    currentParams.dpi = scanDpi;
    currentParams.outputFolder = outputFolder;
    currentParams.frameIndex = frameIndex;

    QMetaObject::invokeMethod(scanWorker,
                              "requestScan",
                              Q_ARG(ScanParameters,
                                    currentParams));
}

void MainWindow::onNext()
{
    frameIndex++;
    imageLabel->clearPreview();
    rawImage = QImage();
    updateLabels();
}

void MainWindow::onPrev()
{
    if (frameIndex > 0)
        frameIndex--;
    imageLabel->clearPreview();
    rawImage = QImage();
    updateLabels();
}

void MainWindow::onChooseDevice()
{
    deviceBtn->setEnabled(false);   // prevent double click
    scanWorker->setDeviceName("");
    QMetaObject::invokeMethod(scanWorker, "requestDeviceList");
    deviceBtn->setText("Searching...");
    deviceEdit->setText("");
    folderBtn->setStyleSheet("");
}

void MainWindow::onPreview()
{
    if (scanWorker->getDeviceName().isEmpty())
        return;

    if( currentCaps.maxDpi == 0)
        return;
    
    currentParams.color = colorswitch->isChecked();
    currentParams.dpi = previewDpi;  

    QMetaObject::invokeMethod(scanWorker,
                              "requestPreview",
                              Q_ARG(ScanParameters,
                                    currentParams));
}

// ----/ BUTTONS

// ---- SIGNALS 

void MainWindow::onCapabilitiesReady(const ScannerCapabilities &caps)
{
    previewDpiBox->setPlaceholderText("");
    scanDpiBox->setPlaceholderText("");

    currentCaps = caps;

    if (caps.supportedResolutions.isEmpty())
    {
        QMessageBox::warning(this,
                             "Error",
                             "Device has no usable resolution.");
        return;
    }


    currentParams.dpi = caps.minDpi;
    currentParams.bitDepth =
        caps.supports16Bit ? 16 : 8;

    currentParams.source =
        caps.supportsTransparency ?
        "Transparency Adapter" :
        caps.supportedSources.value(0);

    previewDpiBox->clear();
    scanDpiBox->clear();

    for (int dpi : caps.supportedResolutions)
    {
        previewDpiBox->addItem(QString::number(dpi), dpi);
        scanDpiBox->addItem(QString::number(dpi), dpi);
    }

    previewDpiBox->setEnabled(true);
    scanDpiBox->setEnabled(true);


    int minIndex =
        previewDpiBox->findData(caps.minDpi);
    previewDpiBox->setCurrentIndex(minIndex);

    int maxIndex =
        scanDpiBox->findData(caps.maxDpi);
    scanDpiBox->setCurrentIndex(maxIndex);

    statusBar()->showMessage(
        QString("Device ready. DPI range: %1 - %2")
            .arg(caps.minDpi)
            .arg(caps.maxDpi),
        5000);
}

void MainWindow::onImageReady(const QImage &img)
{
    imageLabel->setPixmap(
        QPixmap::fromImage(img)
            .scaled(imageLabel->size(),
                    Qt::KeepAspectRatio,
                    Qt::SmoothTransformation));
}

void MainWindow::onScanStarted(int estSec)
{
    setBusy(true);

    maxTime = estSec;
    progressBar->setValue(0);

    estimatedSeconds = estSec;
    progressValue = 0;

    timer->start(1000);

    setProgressScanningStyle();
    updateTimeLabel();
}

void MainWindow::callPreviewUpdater(){
    if(rawImage.isNull()){
        return;
    }

    ImageEditParams params;

    params.turn = flipSwitch->checkState();
    params.invert = !slideSwitch->checkState();
    params.mirror_v = upsideSwitch->checkState();
    params.mirror_h = mirrorSwitch->checkState();
    params.autolevel = !rawSwitch->checkState();

    QMetaObject::invokeMethod(editWorker,
                          "processImage",
                          Q_ARG(QImage, rawImage),
                          Q_ARG(ImageEditParams, params));
    return;
}
/*
    QString filename =
        QString("%1/scan_%2.png")
            .arg(params.outputFolder)
            .arg(params.frameIndex, 3, 10, QChar('0'));*/
void MainWindow::onScanFinished(const QImage &img){
    rawImage = img;
    timer->stop();
    progressValue=100.0;
    estimatedSeconds=0;
    setProgressReadyStyle();
    updateTimeLabel();
    setBusy(false);

    callPreviewUpdater();

    onSaveRequest();
}

void MainWindow::onPreviewFinished(const QImage &img){
    rawImage = img;
    timer->stop();
    progressValue=100.0;
    estimatedSeconds=0;
    setProgressReadyStyle();
    updateTimeLabel();
    setBusy(false);

    callPreviewUpdater();

}

void MainWindow::onDeviceListFound(QStringList devices)
{
    deviceBtn->setEnabled(true);
    deviceBtn->setText("Choose Device");

    if (devices.isEmpty())
        return;

    QStringList labels;
    for (const QString &d : devices)
        labels << d.section('|',1,1);

    bool ok;
    QString selected =
        QInputDialog::getItem(this,
                              "Select Scanner",
                              "Device:",
                              labels,
                              0,
                              false,
                              &ok);

    if (!ok || selected.isEmpty())
        return;

    int index = labels.indexOf(selected);
    QString deviceId = devices[index].section('|',0,0);

    scanWorker->setDeviceName(deviceId);
    deviceEdit->setText(selected);

    scanDpiBox->setPlaceholderText("Polling…");
    previewDpiBox->setPlaceholderText("Polling…");
    QMetaObject::invokeMethod(scanWorker, "requestCapabilities");
}

void MainWindow::onSaveComplete(bool success){
    if(success){
        statusBar()->showMessage(
            QString("Saving successful!"),
            5000);
    }
    else{
        statusBar()->showMessage(
            QString("Saving failed!"),
            5000);
    }
    return;
}

// ----/ SIGNALS 