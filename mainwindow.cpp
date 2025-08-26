#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QDateTime>
#include <QThread>

static QStringList allCh() {
    QStringList r;
    for (int i=0;i<16;++i) r << QString("CH%1").arg(i);
    return r;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    editDriver = new QLineEdit("kingst-la2016");
    editSamplerate = new QLineEdit("10000000"); // 10 MHz
    spinSamples = new QSpinBox();
    spinSamples->setRange(1, 1000000000);
    spinSamples->setValue(1000000);

    comboCanRx = new QComboBox();
    comboCanRx->addItems(allCh());
    comboCanRx->setCurrentText("CH0");

    editCanBitrate = new QLineEdit("500000");
    editSamplePoint = new QLineEdit("80.0");

    btnStart = new QPushButton("Start");
    btnStop  = new QPushButton("Stop");
    btnStop->setEnabled(false);

    txtLog = new QTextEdit(); txtLog->setReadOnly(true);

    tblCan = new QTableWidget(0, 5);
    tblCan->setHorizontalHeaderLabels({"Time","ID","IDE","DLC","Data"});
    tblCan->horizontalHeader()->setStretchLastSection(true);
    tblCan->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tblCan->setSelectionBehavior(QAbstractItemView::SelectRows);

    auto *gbDev = new QGroupBox("Device");
    auto *devLay = new QGridLayout();
    devLay->addWidget(new QLabel("Driver:"),0,0);
    devLay->addWidget(editDriver,0,1);
    devLay->addWidget(new QLabel("Samplerate (Hz):"),1,0);
    devLay->addWidget(editSamplerate,1,1);
    devLay->addWidget(new QLabel("Samples:"),2,0);
    devLay->addWidget(spinSamples,2,1);
    gbDev->setLayout(devLay);

    auto *gbCap = new QGroupBox("Capture");
    auto *capLay = new QGridLayout();
    capLay->addWidget(new QLabel("CAN RX:"),0,0);
    capLay->addWidget(comboCanRx,0,1);
    capLay->addWidget(new QLabel("Nominal bitrate:"),1,0);
    capLay->addWidget(editCanBitrate,1,1);
    capLay->addWidget(new QLabel("Sample point (%):"),2,0);
    capLay->addWidget(editSamplePoint,2,1);
    gbCap->setLayout(capLay);

    auto *gbCtrl = new QGroupBox("Controls");
    auto *ctrlLay = new QHBoxLayout();
    ctrlLay->addWidget(btnStart);
    ctrlLay->addWidget(btnStop);
    ctrlLay->addStretch();
    gbCtrl->setLayout(ctrlLay);

    auto *left = new QVBoxLayout();
    left->addWidget(gbDev);
    left->addWidget(gbCap);
    left->addWidget(gbCtrl);
    left->addStretch();

    auto *gbCan = new QGroupBox("CAN Frames");
    auto *canLay = new QVBoxLayout();
    canLay->addWidget(tblCan);
    gbCan->setLayout(canLay);

    auto *gbLog = new QGroupBox("Logs");
    auto *logLay = new QVBoxLayout();
    logLay->addWidget(txtLog);
    gbLog->setLayout(logLay);

    auto *right = new QVBoxLayout();
    right->addWidget(gbCan, 3);
    right->addWidget(gbLog, 1);

    auto *central = new QWidget();
    auto *root = new QHBoxLayout(central);
    root->addLayout(left, 3);
    root->addLayout(right, 6);
    setCentralWidget(central);
    setWindowTitle("Sigrok Direct (Kingst)");

    connect(btnStart, &QPushButton::clicked, this, &MainWindow::onStart);
    connect(btnStop,  &QPushButton::clicked, this, &MainWindow::onStop);
}

MainWindow::~MainWindow() {
    onStop();
}

void MainWindow::onStart() {
    btnStart->setEnabled(false);
    btnStop->setEnabled(true);
    txtLog->clear();
    tblCan->setRowCount(0);

    if (workerThread) {
        onStop();
    }

    worker = new SigrokWorker();
    worker->setDeviceDriver(editDriver->text().trimmed());
    worker->setSamplerate(editSamplerate->text().trimmed().toULongLong());
    worker->setLimitSamples(spinSamples->value());
    worker->setChannels(QStringList() << comboCanRx->currentText());
    worker->setCanParams(editCanBitrate->text().trimmed().toUInt(),
                         editSamplePoint->text().trimmed().toDouble());

    workerThread = new QThread(this);
    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::started, worker, &SigrokWorker::start);
    connect(worker, &SigrokWorker::finished, this, &MainWindow::onFinished);
    connect(worker, &SigrokWorker::canFrameReady, this, &MainWindow::onCanFrame);
    connect(worker, &SigrokWorker::logMsg, this, &MainWindow::onLog);

    connect(worker, &SigrokWorker::finished, workerThread, &QThread::quit);
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    workerThread->start();
}

void MainWindow::onStop() {
    btnStart->setEnabled(true);
    btnStop->setEnabled(false);
    if (worker) worker->stop();
    if (workerThread) {
        workerThread->quit();
        workerThread->wait(500);
        workerThread = nullptr;
        worker = nullptr;
    }
}

void MainWindow::onCanFrame(const CanFrame &f) {
    int row = tblCan->rowCount();
    tblCan->insertRow(row);
    tblCan->setItem(row,0, new QTableWidgetItem(f.timeStr));
    tblCan->setItem(row,1, new QTableWidgetItem(f.id));
    tblCan->setItem(row,2, new QTableWidgetItem(f.ide));
    tblCan->setItem(row,3, new QTableWidgetItem(QString::number(f.dlc)));
    tblCan->setItem(row,4, new QTableWidgetItem(f.data));
}

void MainWindow::onLog(const QString &msg) {
    txtLog->append(msg);
}

void MainWindow::onFinished(int code) {
    txtLog->append(QString("\n[Finished] code=%1").arg(code));
    btnStart->setEnabled(true);
    btnStop->setEnabled(false);
}
