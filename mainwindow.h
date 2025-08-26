#pragma once
#include <QMainWindow>
#include <QTextEdit>
#include <QTableWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QThread>
#include "sigrokworker.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow();

private slots:
    void onStart();
    void onStop();
    void onCanFrame(const CanFrame &f);
    void onLog(const QString &msg);
    void onFinished(int code);

private:
    QLineEdit *editDriver;
    QLineEdit *editSamplerate;
    QSpinBox  *spinSamples;
    QComboBox *comboCanRx;
    QLineEdit *editCanBitrate;
    QLineEdit *editSamplePoint;

    QPushButton *btnStart;
    QPushButton *btnStop;

    QTextEdit *txtLog;
    QTableWidget *tblCan;

    SigrokWorker *worker = nullptr;
    QThread *workerThread = nullptr;
};
