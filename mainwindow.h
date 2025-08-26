#pragma once
#include <QMainWindow>
#include <QTextEdit>
#include <QTableWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QListWidget>
#include "sigrokworker.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent=nullptr);
    ~MainWindow();

private slots:
    void onStart();
    void onStop();
    void onRawLine(const QString &line);
    void onCanFrame(const CanFrame &f);
    void onLog(const QString &msg);
    void onFinished(int code);
    void onModeChanged(int);

private:
    enum class Mode { RawHex, CanDecode };
    Mode mode() const;

private:
    QLineEdit *editDriver;
    QLineEdit *editSamplerate;
    QSpinBox  *spinSamples;
    QListWidget *listChannels;

    QComboBox *comboMode;
    QComboBox *comboCanRx;
    QLineEdit *editCanBitrate;
    QLineEdit *editSamplePoint;

    QPushButton *btnStart;
    QPushButton *btnStop;

    QTextEdit *txtRaw;
    QTextEdit *txtLog;
    QTableWidget *tblCan;

    SigrokWorker *worker = nullptr;
    QThread *workerThread = nullptr;
};
