#include "sigrokworker.h"
#include <QDateTime>
#include <QDebug>
#include <QThread>
#include <type_traits>

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

    auto drivers = sr_driver_list(sr_ctx_);

    auto findDriver = [&](auto list) -> struct sr_dev_driver* {
        using ListT = decltype(list);
        struct sr_dev_driver *found = nullptr;
        if constexpr (std::is_same_v<ListT, GSList*>) {
            for (GSList *l = list; l; l = l->next) {
                auto *d = static_cast<struct sr_dev_driver*>(l->data);
                if (driverName_ == QString::fromUtf8(d->name)) { found = d; break; }
            }
            g_slist_free(list);
        } else {
            for (ListT d = list; *d; ++d) {
                if (driverName_ == QString::fromUtf8((*d)->name)) { found = *d; break; }
            }
            g_free(list);
        }
        return found;
    };

    struct sr_dev_driver *drv = findDriver(drivers);
    if (!drv) {
        emit logMsg("Driver not found: " + driverName_);
        sr_exit(ctx);
        sr_ctx_ = nullptr;
        emit finished(-2);
        return;
    }

    // Scan
    GSList *devs = nullptr;
#if 0
    if (sr_driver_scan(const_cast<struct sr_dev_driver*>(drv), &devs, nullptr) != SR_OK || !devs) {
        emit logMsg("No devices found for driver.");
        sr_exit(ctx);
        sr_ctx_ = nullptr;
        emit finished(-3);
        return;
    }
#else
    devs = sr_driver_scan(const_cast<struct sr_dev_driver*>(drv), nullptr);
    if (!devs) {
        emit logMsg("No devices found for driver.");
        sr_exit(ctx);
        sr_ctx_ = nullptr;
        emit finished(-3);
        return;
    }
#endif
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

#ifndef NO_SRD
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

    while (!stopFlag_.loadRelaxed() && sr_session_is_running(sr_sess_)) {
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

void SigrokWorker::stop() { stopFlag_.storeRelaxed(1); }

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
        self->stopFlag_.storeRelaxed(1);
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

#ifndef NO_SRD
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

#ifndef NO_SRD
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
    srd_can_inst_ = srd_inst_new(srd_sess_, "can", nullptr);
    if (!srd_can_inst_) {
        emit logMsg("srd_inst_new failed; CAN decode disabled.");
        canMode_ = false;
        return;
    }
    // Set decoder options.
    GHashTable *opts = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(opts, (gpointer)"nominal_bitrate", g_variant_new_uint64(canNominalBitrate_));
    g_hash_table_insert(opts, (gpointer)"sample_point", g_variant_new_double(canSamplePoint_));
    srd_inst_option_set(srd_can_inst_, opts);
    g_hash_table_destroy(opts);

#ifdef SRD_OUTPUT_LOGIC
    srd_session_probe_new(srd_sess_, SRD_OUTPUT_LOGIC, 0, "can_rx");
    GHashTable *chmap = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_insert(chmap, g_strdup("can_rx"), g_variant_new_int32(0));
    srd_inst_channel_set_all(srd_can_inst_, chmap);
    g_hash_table_destroy(chmap);
#endif

    srd_session_datafeed_callback_add(srd_sess_, &SigrokWorker::srdAnnCb, this);
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

    srd_session_send(srd_sess_, SRD_OUTPUT_LOGIC, 0,
                     reinterpret_cast<const uint8_t*>(packed.constData()), packed.size());
}

void SigrokWorker::srdAnnCb(const struct srd_decoder*, struct srd_proto_data *pdata, void *user) {
    Q_UNUSED(pdata);
    Q_UNUSED(user);
}
#endif
