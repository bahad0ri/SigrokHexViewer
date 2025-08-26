#pragma once
#include <QObject>
#include <QAtomicInt>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QMutex>
#include <QPointer>

#include <glib.h>

/*
 * Libsigrokdecode is optional.  By default the build excludes any decoder
 * integration, but defining ENABLE_SRD when compiling enables the CAN
 * decoding paths and pulls in libsigrokdecode headers.
 */
#ifndef ENABLE_SRD
#define NO_SRD
#endif

extern "C" {
#include <libsigrok/libsigrok.h>
#ifndef NO_SRD
#include <libsigrokdecode/libsigrokdecode.h>
#endif
}

struct CanFrame {
    QString timeStr;
    QString id;
    QString ide;
    int dlc = 0;
    QString data;
};

class SigrokWorker : public QObject {
    Q_OBJECT
public:
    explicit SigrokWorker(QObject *parent=nullptr);
    ~SigrokWorker();

    void setDeviceDriver(const QString &drv);
    void setSamplerate(uint64_t sr_hz);
    void setLimitSamples(uint64_t n);
    void setChannels(const QStringList &chs);
    void setCanMode(bool enable);
    void setCanParams(const QString &rxCh, uint32_t nominal_bitrate, double sample_point = 80.0);

signals:
    void rawLineReady(const QString &line);
    void canFrameReady(const CanFrame &f);
    void logMsg(const QString &msg);
    void finished(int code);

public slots:
    void start();
    void stop();

private:
    static void datafeedCb(const struct sr_dev_inst *sdi,
                           const struct sr_datafeed_packet *packet, void *cb_data);
    void handleLogicPacket(const struct sr_datafeed_logic *logic);

#ifndef NO_SRD
    void srdInitIfNeeded();
    void srdFeed(const struct sr_datafeed_logic *logic);
    static void srdAnnCb(const struct srd_decoder *dec, struct srd_proto_data *pdata, void *user);
#endif

private:
    QString driverName_ = "kingst-la2016";
    uint64_t samplerate_ = 10'000'000;
    uint64_t limitSamples_ = 1'000'000;
    QStringList enabledChs_ = {"CH0"};

    bool canMode_ = false;
    QString canRx_ = "CH0";
    uint32_t canNominalBitrate_ = 500000;
    double canSamplePoint_ = 80.0;

    QAtomicInt stopFlag_ {0};

    struct sr_context *sr_ctx_ = nullptr;
    struct sr_session *sr_sess_ = nullptr;
    struct sr_dev_inst *sdi_ = nullptr;

#ifndef NO_SRD
    struct srd_session *srd_sess_ = nullptr;
    struct srd_decoder_inst *srd_can_inst_ = nullptr;
#endif

    QVector<int> activeLogicIdx_;
};
