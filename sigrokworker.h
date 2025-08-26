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

#include <glib.h>
extern "C" {
#include <libsigrok/libsigrok.h>
#include <libsigrokdecode/libsigrokdecode.h>
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
    void setCanParams(uint32_t nominal_bitrate, double sample_point = 80.0);

signals:
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

#ifdef SRD_HEADER
    void srdInitIfNeeded();
    void srdFeed(const struct sr_datafeed_logic *logic);
    static void srdAnnCb(const struct srd_decoder *dec, struct srd_proto_data *pdata, void *user);
#endif

private:
    QString driverName_ = "kingst-la2016";
    uint64_t samplerate_ = 10'000'000;
    uint64_t limitSamples_ = 1'000'000;
    QStringList enabledChs_ = {"CH0"};
    uint32_t canNominalBitrate_ = 500000;
    double canSamplePoint_ = 80.0;

    QAtomicInt stopFlag_ {0};

    struct sr_context *sr_ctx_ = nullptr;
    struct sr_session *sr_sess_ = nullptr;
    struct sr_dev_inst *sdi_ = nullptr;

#ifdef SRD_HEADER
    struct srd_session *srd_sess_ = nullptr;
    const struct srd_decoder *srd_can_dec_ = nullptr;
    struct srd_decoder_inst *srd_can_inst_ = nullptr;
#endif

    QVector<int> activeLogicIdx_;
};
