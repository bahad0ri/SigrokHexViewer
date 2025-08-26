#include "sigrokworker.h"
#include <QDateTime>
#include <QDebug>
#include <QThread>

SigrokWorker::SigrokWorker(QObject *parent) : QObject(parent) {}
SigrokWorker::~SigrokWorker() { stop(); }

void SigrokWorker::setDeviceDriver(const QString &drv) { driverName_ = drv; }
void SigrokWorker::setSamplerate(uint64_t sr) { samplerate_ = sr; }
void SigrokWorker::setLimitSamples(uint64_t n) { limitSamples_ = n; }
void SigrokWorker::setChannels(const QStringList &chs) { enabledChs_ = chs; }
void SigrokWorker::setCanMode(bool e) { canMode_ = e; }
void SigrokWorker::setCanParams(const QString &rxCh, uint32_t nominal_bitrate, double sample_point) {
    canRx_ = rxCh; canNominalBitrate_ = nominal_bitrate; canSamplePoint_ = sample_point;
}

void SigrokWorker::start() {
    stopFlag_.storeRelaxed(0);
    emit logMsg("sr_init...");
    struct sr_context *ctx = nullptr;
    if (sr_init(&ctx) != SR_OK) {
        emit logMsg("sr_init failed.");
        emit finished(-1);
        return;
    }
    sr_ctx_ = ctx;

    // Driver list (libsigrok >=0.6 uses a NULL terminated array)
    const struct sr_dev_driver **drivers = sr_driver_list(sr_ctx_);
    const struct sr_dev_driver *drv = nullptr;
    if (drivers) {
        for (const struct sr_dev_driver **d = drivers; *d; ++d) {
            if (driverName_ == QString::fromUtf8((*d)->name)) { drv = *d; break; }
        }
    }
    g_free(drivers);
    if (!drv) {
        emit logMsg("Driver not found: " + driverName_);
        sr_exit(ctx);
        sr_ctx_ = nullptr;
        emit finished(-2);
        return;
    }

    // Scan
    GSList *devs = sr_driver_scan(const_cast<struct sr_dev_driver*>(drv), nullptr);
    if (!devs) {
        emit logMsg("No devices found for driver.");
        sr_exit(ctx);
        sr_ctx_ = nullptr;
        emit finished(-3);
        return;
    }
    sdi_ = static_cast<struct sr_dev_inst*>(devs->data);
    g_slist_free(devs);

    // Open
    if (sr_dev_open(sdi_) != SR_OK) {
        emit logMsg("sr_dev_open failed.");
        sr_exit(ctx);
        sr_ctx_ = nullptr;
        emit finished(-4);
        return;
    }

    // Samplerate
    GVariant *g_sr = g_variant_new_uint64(samplerate_);
    if (sr_config_set(sdi_, nullptr, SR_CONF_SAMPLERATE, g_sr) != SR_OK) {
        emit logMsg("Failed to set samplerate.");
    }

    // Enable channels
    activeLogicIdx_.clear();
    GSList *channels = sr_dev_inst_channels_get(sdi_);
    for (GSList *l = channels; l; l = l->next) {
        auto *ch = static_cast<struct sr_channel*>(l->data);
        const QString name = QString::fromUtf8(ch->name ? ch->name : "");
        bool enable = enabledChs_.contains(name);
        ch->enabled = enable ? TRUE : FALSE;
        if (enable) activeLogicIdx_.append(ch->index);
    }
    if (activeLogicIdx_.isEmpty()) {
        emit logMsg("No channels enabled, enabling CH0 fallback.");
        for (GSList *l = channels; l; l = l->next) {
            auto *ch = static_cast<struct sr_channel*>(l->data);
            if (QString::fromUtf8(ch->name ? ch->name : "") == "CH0") {
                ch->enabled = TRUE; activeLogicIdx_.append(ch->index); break;
            }
        }
    }
    g_slist_free(channels);

    // Session
    if (sr_session_new(sr_ctx_, &sr_sess_) != SR_OK) {
        emit logMsg("sr_session_new failed.");
        sr_dev_close(sdi_);
        sr_exit(ctx);
        emit finished(-5);
        return;
    }
    if (sr_session_dev_add(sr_sess_, sdi_) != SR_OK) {
        emit logMsg("sr_session_dev_add failed.");
        sr_session_destroy(sr_sess_);
        sr_dev_close(sdi_);
        sr_exit(ctx);
        emit finished(-6);
        return;
    }

    // Limit samples
    GVariant *g_nsamp = g_variant_new_uint64(limitSamples_);
    if (sr_config_set(sdi_, nullptr, SR_CONF_LIMIT_SAMPLES, g_nsamp) != SR_OK) {
        emit logMsg("Failed to set LIMIT_SAMPLES (it might still work).");
    }

    // Callback
    sr_session_datafeed_callback_add(sr_sess_, &SigrokWorker::datafeedCb, this);

#ifdef SRD_HEADER
    srdInitIfNeeded();
#endif

    // Start
    emit logMsg("sr_session_start...");
    if (sr_session_start(sr_sess_) != SR_OK) {
        emit logMsg("sr_session_start failed.");
        sr_session_destroy(sr_sess_);
        sr_dev_close(sdi_);
        sr_exit(ctx);
        emit finished(-7);
        return;
    }

    while (!stopFlag_.load() && sr_session_is_running(sr_sess_)) {
        QThread::msleep(10);
    }

    // Cleanup
    sr_session_stop(sr_sess_);
    sr_session_datafeed_callback_remove_all(sr_sess_);
    sr_session_dev_remove(sr_sess_, sdi_);
    sr_session_destroy(sr_sess_);
    sr_dev_close(sdi_);
    sr_exit(ctx);

    sr_sess_ = nullptr;
    sdi_ = nullptr;
    sr_ctx_ = nullptr;

    emit finished(0);
}

void SigrokWorker::stop() { stopFlag_.store(1); }

void SigrokWorker::datafeedCb(const struct sr_dev_inst*,
                              const struct sr_datafeed_packet *packet, void *cb_data)
{
    auto *self = static_cast<SigrokWorker*>(cb_data);
    if (!self || !packet) return;

    switch (packet->type) {
    case SR_DF_LOGIC: {
        auto *logic = static_cast<const struct sr_datafeed_logic*>(packet->payload);
        self->handleLogicPacket(logic);
        break;
    }
    case SR_DF_END:
        self->stopFlag_.store(1);
        break;
    default:
        break;
    }
}

void SigrokWorker::handleLogicPacket(const struct sr_datafeed_logic *logic) {
    if (!logic || !logic->data || logic->length == 0) return;

    const uint8_t *buf = static_cast<const uint8_t*>(logic->data);
    const size_t nbytes = logic->length;
    if (logic->unitsize != 1) {
        emit logMsg("Unexpected unitsize != 1; skipping.");
        return;
    }

#ifdef SRD_HEADER
    if (canMode_) {
        srdFeed(logic);
    }
#endif

    for (int chIdx : activeLogicIdx_) {
        QString line = QString("CH%1: ").arg(chIdx);
        uint8_t acc = 0;
        int bitpos = 0;
        for (size_t i = 0; i < nbytes; ++i) {
            const uint8_t v = buf[i];
            const uint8_t bit = (v >> chIdx) & 0x1;
            acc |= (bit & 0x1) << bitpos;
            bitpos++;
            if (bitpos == 8) {
                line += QString("%1 ").arg(acc, 2, 16, QLatin1Char('0')).toUpper();
                bitpos = 0;
                acc = 0;
            }
        }
        if (bitpos != 0) {
            line += QString("%1 ").arg(acc, 2, 16, QLatin1Char('0')).toUpper();
        }
        emit rawLineReady(line.trimmed());
    }
}

#ifdef SRD_HEADER
void SigrokWorker::srdInitIfNeeded() {
    if (!canMode_) return;
    if (srd_init(nullptr) != SRD_OK) {
        emit logMsg("srd_init failed; CAN decode disabled.");
        canMode_ = false;
        return;
    }
    if (srd_session_new(&srd_sess_) != SRD_OK) {
        emit logMsg("srd_session_new failed; CAN decode disabled.");
        canMode_ = false;
        return;
    }
    srd_decoder_load_all();
    srd_can_dec_ = srd_decoder_by_id("can");
    if (!srd_can_dec_) {
        emit logMsg("CAN decoder not found in libsigrokdecode.");
        canMode_ = false;
        return;
    }
    if (srd_inst_new(&srd_can_inst_, srd_can_dec_, srd_sess_) != SRD_OK) {
        emit logMsg("srd_inst_new failed; CAN decode disabled.");
        canMode_ = false;
        return;
    }
    srd_inst_option_set(srd_can_inst_, "nominal_bitrate", canNominalBitrate_);
    srd_inst_option_set(srd_can_inst_, "sample_point", canSamplePoint_);

    srd_probe_new(srd_sess_, SRD_PROBE_LOGIC, 0, "can_rx");
    srd_inst_channel_set(srd_can_inst_, "can_rx", 0);

    srd_session_data_callback_set(srd_sess_, &SigrokWorker::srdAnnCb, this);
    srd_session_samplerate_set(srd_sess_, samplerate_);
}

void SigrokWorker::srdFeed(const struct sr_datafeed_logic *logic) {
    if (!srd_sess_ || !logic || !logic->data || logic->length == 0) return;

    int rxIndex = activeLogicIdx_.isEmpty() ? 0 : activeLogicIdx_.first();

    const uint8_t *buf = static_cast<const uint8_t*>(logic->data);
    const size_t nbytes = logic->length;

    QByteArray packed;
    packed.reserve(static_cast<int>((nbytes * 8 + 7) / 8));

    uint8_t acc = 0; int bitpos = 0;
    for (size_t i = 0; i < nbytes; ++i) {
        const uint8_t v = buf[i];
        const uint8_t bit = (v >> rxIndex) & 0x1;
        acc |= (bit & 1) << bitpos;
        bitpos++;
        if (bitpos == 8) {
            packed.append(char(acc));
            acc = 0; bitpos = 0;
        }
    }
    if (bitpos) packed.append(char(acc));

    srd_session_send(srd_sess_, SRD_PROTO_DATA_LOGIC, 0,
                     reinterpret_cast<const uint8_t*>(packed.constData()), packed.size());
}

void SigrokWorker::srdAnnCb(const struct srd_decoder*, struct srd_proto_data *pdata, void *user) {
    auto *self = static_cast<SigrokWorker*>(user);
    if (!self || !pdata || pdata->ann == nullptr) return;

    QString txt = QString::fromUtf8(reinterpret_cast<const char*>(pdata->ann->text));

    static const QRegExp rx1("id=0x([0-9A-Fa-f]+)\\s+ext=([01]|true|false)\\s+dlc=([0-8])\\s+data=([0-9A-Fa-f]{2}(?:\\s+[0-9A-Fa-f]{2}){0,7})");
    static const QRegExp rx2("id[:=]\\s*0x?([0-9A-Fa-f]+)\\s+dlc[:=]\\s*([0-8]).*?\\b([0-9A-Fa-f]{2}(?:\\s+[0-9A-Fa-f]{2}){0,7})");

    CanFrame f;
    f.timeStr = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");

    if (rx1.indexIn(txt) >= 0) {
        f.id = "0x" + rx1.cap(1).toUpper();
        const QString ext = rx1.cap(2).toLower();
        f.ide = (ext == "1" || ext == "true") ? "ext" : "std";
        f.dlc = rx1.cap(3).toInt();
        f.data = rx1.cap(4).simplified().toUpper();
        self->canFrameReady(f);
        return;
    }
    if (rx2.indexIn(txt) >= 0) {
        f.id = "0x" + rx2.cap(1).toUpper();
        f.ide = "std";
        f.dlc = rx2.cap(2).toInt();
        f.data = rx2.cap(3).simplified().toUpper();
        self->canFrameReady(f);
        return;
    }
}
#endif
