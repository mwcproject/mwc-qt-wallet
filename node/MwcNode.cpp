// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "MwcNode.h"
#include <QDebug>
#include "../util/Process.h"
#include <QDir>
#include <QThread>
#include "../core/appcontext.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../core/Notification.h"
#include "../core/WndManager.h"
#include "../util/Log.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QCoreApplication>
#include <math.h>
#include <QtConcurrent>

#include "TorProcess.h"
#include "util/Files.h"
#include "util/message_mapper.h"

namespace node {

/*int8_t const * blockChainCallback(void * mwc_node, int8_t const * event) {
    Q_UNUSED(mwc_node);
    // Logging now, will emit the signal to update the wallet state for Accepted block and may be for a new transaciton.
    logger::logInfo(logger::MWC_NODE, QString((const char *)event));
    return nullptr;
}*/

//////////////////////////////////////////////////////////////////////////////////////////////
///

static QPair<int, QString> startNode(MwcNode * node, QFuture<QString> * torStarter, QAtomicInt * stoppingFlag) {

    // Kill the thread itself when finished
    Q_ASSERT(node);
    Q_ASSERT(stoppingFlag);

    if ( !config::isColdWallet() ) {
        Q_ASSERT(torStarter);
        // waiting for tor core to start first
        QString torStartResult = torStarter->result();
        if (!torStartResult.isEmpty()) {
            return QPair<int, QString>(-1,"Failed to start embedded Tor");
        }
    }

    if (stoppingFlag->loadAcquire()) {
        return QPair<int, QString>(-1,"Start is canceled");
    }

    mwc_api::ApiResponse<int> create_context_res = node::create_context(node->getCurrentNetwork());
    if (create_context_res.hasError()) {
        return QPair<int, QString>(-1, "Unable to create a context. " + util::mapMessage(create_context_res.error));
    }

    int context_id = create_context_res.response;

    QString dataPath = node->getNodeDataPath();

    if (stoppingFlag->loadAcquire()) {
        return QPair<int, QString>(context_id,"Start is canceled");
    }

    mwc_api::ApiResponse<bool> res = create_server(context_id, dataPath, nullptr,
                        nullptr, nullptr);
    if (res.hasError()) {
        // Let's try to clean up the node data
        logger::logInfo( logger::MWC_NODE, "Cleaning the data because failed to start server: " + res.error);
        QDir dir(dataPath);
        dir.removeRecursively();
        QDir().mkpath(dataPath);

        res = create_server(context_id, dataPath, nullptr,
                        nullptr, nullptr);

        if (res.hasError()) {
            return QPair<int, QString>(context_id, "Unable to create the mwc-node. " + util::mapMessage(res.error));
        }
    }

    res = init_call_api(context_id);
    if (res.hasError()) {
        return QPair<int, QString>(context_id, "MWC node failed process init_call_api. " + util::mapMessage(res.error));
    }

    if (stoppingFlag->loadAcquire()) {
        return QPair<int, QString>(context_id,"Start is canceled");
    }

    if ( !config::isColdWallet() ) {
        // starting all other services if everybody is online

        Q_ASSERT( tor::getTorStatus().first );

        // Start p2p discovery job
        res = start_discover_peers(context_id);
        if (res.hasError()) {
            return QPair<int, QString>(context_id, "MWC node unable to start peers discovery. " + util::mapMessage(res.error));
        }

        if (stoppingFlag->loadAcquire()) {
            return QPair<int, QString>(context_id,"Start is canceled");
        }

        // Start node sync job
        res = start_sync_monitoring(context_id);
        if (res.hasError()) {
            return QPair<int, QString>(context_id, "MWC node unable to start syncing. " + util::mapMessage(res.error));
        }

        if (stoppingFlag->loadAcquire()) {
            return QPair<int, QString>(context_id,"Start is canceled");
        }

        // Start peers listening job
        res = start_listen_peers(context_id);
        if (res.hasError()) {
            return QPair<int, QString>(context_id, "MWC node unable to start listening. " + util::mapMessage(res.error));
        }

        if (stoppingFlag->loadAcquire()) {
            return QPair<int, QString>(context_id,"Start is canceled");
        }

        // Start dandelion job
        res = start_dandelion(context_id);
        if (res.hasError()) {
            return QPair<int, QString>(context_id, "MWC node unable to start dandelion. " + util::mapMessage(res.error));
        }
    }

    return QPair<int, QString>(context_id, "");
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

MwcNode::MwcNode(core::AppContext * _appContext, QFuture<QString> * _torStarter) :
        QObject(),
        appContext(_appContext),
        torStarter(_torStarter),
        stoppingFlag(0)
{
}

MwcNode::~MwcNode() {
    if (isRunning()) {
        stop();
    }
}

void MwcNode::start(const QString & dataPath, const QString & network ) {
    if (dataPath==nodeNetwork && nodeNetwork == network) {
        qDebug() << "MwcNode::starting is skipped, node is already running";
        return;
    }

    stop();

    Q_ASSERT(!startingNode.isValid());

    Q_ASSERT( stoppingFlag.loadAcquire() == 0 );

    qDebug() << "MwcNode::starting for network " + network;
    nodeNetwork = network;
    nodeDataPath = dataPath;

    startingNode = QtConcurrent::run([this]() -> void {
        QPair<int, QString> result = startNode(this, this->torStarter, &this->stoppingFlag );
        this->submitStartNodeResult(result.second, result.first, this->stoppingFlag.loadAcquire()!=0 );
        return;
    });
    Q_ASSERT(startingNode.isValid());

    Q_ASSERT(nodeContextId<0);
}


void MwcNode::stop() {
    qDebug() << "MwcNode::stop ...";

    waitCancelStart();

    if (nodeContextId<0)
        return;

    logger::logInfo( logger::MWC_NODE, "Stopping mwc-node..." );

    int contextId = nodeContextId;
    nodeContextId = -1;
    nodeDataPath = "";
    nodeNetwork = "";
    pibdSyncPhase = false;

    QFuture<QString> nodeStoping = QtConcurrent::run([contextId]()-> QString {
        QThread::currentThread()->setObjectName("node::release_context");
        auto res = node::release_context(contextId);
        QThread::currentThread()->setObjectName("QtThreadPool");
        return util::mapMessage(res.error);
    });

    int waitCnt = 0;
    while (!nodeStoping.isFinished()) {
        // waiting for minute
        waitCnt++;
        QThread::msleep(100);
        QCoreApplication::processEvents();
        if (waitCnt>60*10) // waiting for a minute
            break;
    }

    if (nodeStoping.isFinished()) {
        QString stopError = nodeStoping.result();
        if (stopError.isEmpty()) {
            logger::logInfo( logger::MWC_NODE, "Mwc-node is stopped" );
            notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::INFO, "Embedded mwc-node is stopped." );
        }
        else {
            logger::logError(logger::MWC_NODE, "Mwc-node is stopped with error " + stopError );
            notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::CRITICAL, "Embedded mwc-node stopped with error: " + stopError );

        }
    }
    else {
        logger::logError( logger::MWC_NODE, "Mwc-node wasn't stopped, abandoning it" );
        notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::CRITICAL, "Unable safely stop embedded mwc-node, abandoning the context." );
    }
}

static double calcStageProgress(int minVal, int cur, int maxVal) {
    if (minVal>=maxVal)
        return 0.0;
    if (cur<=minVal)
        return 0.0;
    if (cur>=maxVal)
        return 1.0;

    return double(cur - minVal) / double(maxVal - minVal);
}

static double calcProgress(double base, double range, double localProgress) {
    double res = base + range*localProgress;
    Q_ASSERT(res>=0.0 && res<1.000001);
    return res;
}

// return progress in the range [0-1.0]
static QString calcProgressStr( bool pibdSync, SYNC_STATE syncState, int minVal, int cur, int maxVal ) {
    int total_time = 20*60+4 - (04*60+19); // 945
    int headers_time = 14*60+50-(04*60+19); // 631
    int pibd_time = 17*60+26 - (14*60+50); // 156
    int validation_headers =  18*60 - (17*60+26) - 10; // 34 - 10
    int validation_kernels_pos = 10; // 10
    int validation_rangefroofs = 18*60+10 - 18*60; // 10
    int validation_kernwls = 18*60+40 - (18*60+10); // 30
    int download_blocks_time = 20*60+4 - (18*60+40); // 84

    const double getHeadersShare = double(headers_time) / double(total_time);
    const double pibdShare = double(pibd_time) / double(total_time);
    const double validationHeadersShare = double(validation_headers) / double(total_time);
    const double validationKernelsPosShare = double(validation_kernels_pos) / double(total_time);
    const double validationRangeProofsShare = double(validation_rangefroofs) / double(total_time);
    const double validationKernelsShare = double(validation_kernwls) / double(total_time);
    const double blocksShare = double(download_blocks_time) / double(total_time);

    Q_ASSERT( fabs(getHeadersShare + pibdShare + validationHeadersShare + validationKernelsPosShare + validationRangeProofsShare + validationKernelsShare + blocksShare - 1.0) < 0.0001 );

    double stageProgress = calcStageProgress(minVal, cur, maxVal);

    double progressRes = 0.0;
    switch( syncState ) {
        case SYNC_STATE::GETTING_HEADERS:
            progressRes = calcProgress(0.0, getHeadersShare, stageProgress);
            break;
        case SYNC_STATE::GETTING_PIBD:
            progressRes = calcProgress(getHeadersShare, pibdShare, stageProgress);
            break;
        case SYNC_STATE::VERIFY_KERNELS_HISTORY:
            progressRes = calcProgress(getHeadersShare, pibdShare, 1.0);
            break;
        case SYNC_STATE::VERIFY_HEADERS:
            progressRes = calcProgress(getHeadersShare+pibdShare, validationHeadersShare, stageProgress);
            break;
        case SYNC_STATE::VERIFY_KERNEKLS_POS:
            progressRes = calcProgress(getHeadersShare+pibdShare+validationHeadersShare, validationKernelsPosShare, stageProgress);
            break;
        case SYNC_STATE::VERIFY_RANGEPROOFS:
            progressRes = calcProgress(getHeadersShare+pibdShare+validationHeadersShare+validationKernelsPosShare, validationRangeProofsShare, stageProgress);
            break;
        case SYNC_STATE::VERIFY_KERNEL:
            progressRes = calcProgress(getHeadersShare+pibdShare+validationHeadersShare+validationKernelsPosShare+validationRangeProofsShare, validationKernelsShare, stageProgress);
            break;
        case SYNC_STATE::GETTING_BLOCKS:
            if (pibdSync) {
                progressRes = calcProgress(getHeadersShare+pibdShare+validationHeadersShare+validationKernelsPosShare+validationRangeProofsShare + validationKernelsShare, blocksShare, stageProgress);
            }
            else {
                progressRes = calcProgress(0.0, 1.0, stageProgress);
            }
            break;
    }

    if (progressRes<0.0)
        progressRes = 0.0;
    if (progressRes>1.0)
        progressRes = 1.0;

    return "Syncing " + QString::number( progressRes * 100.0, 'f', 1 ) + "%";
}

QString calculateRunningStatus(bool & pibdSyncPhase, ServerStats stats) {
    if ( stats.sync_status.status.isEmpty()  )
        return "Starting";

    int64_t  peersMaxHeight = 0;
    if ( stats.peer_stats.size() > 0 ) {
        peersMaxHeight = stats.peer_stats[0].height;
        for (const auto & p : stats.peer_stats) {
            peersMaxHeight = std::max(peersMaxHeight, p.height);
        }
    }

    if (config::isColdWallet()) {
        // Tweaking the node status.
        if (stats.sync_status.status == "AwaitingPeers")
            stats.sync_status.status = "NoSync";
    }


    if ( stats.sync_status.status == "NoSync") {
        pibdSyncPhase = false;
        // tolerance - two blocks
        if (stats.header_stats.height > 0) {
            if (stats.header_stats.height >= peersMaxHeight - 2) {
                return "Ready";
            } else {
                return "Waiting for " + QString::number(peersMaxHeight - stats.header_stats.height) + " top blocks...";
            }
        }
        else {
            return "Waiting";
        }
    }
    else {
        // In sync mode...
        if (stats.sync_status.status == "Initial") {
            return "Starting";
        }
        else if (stats.sync_status.status == "AwaitingPeers") {
            if (stats.peer_count>0)
                return "Found " + QString::number(stats.peer_count) + " peers";
            else
                return "Waiting for peers";
        }
        else if (stats.sync_status.status == "HeaderHashSync") {
            pibdSyncPhase = true;
            return "Handshaking";
        }
        else if (stats.sync_status.status == "HeaderSync") {
            pibdSyncPhase = true;
            return calcProgressStr( pibdSyncPhase, SYNC_STATE::GETTING_HEADERS, 0, stats.sync_status.pos,  stats.sync_status.total );
        }
        else if (stats.sync_status.status == "TxHashsetPibd") {
            pibdSyncPhase = true;
            return calcProgressStr( pibdSyncPhase, SYNC_STATE::GETTING_PIBD, 0, stats.sync_status.pos,  stats.sync_status.total );
        }
        else if (stats.sync_status.status == "ValidatingKernelsHistory") {
            pibdSyncPhase = true;
            return calcProgressStr( pibdSyncPhase, SYNC_STATE::VERIFY_KERNELS_HISTORY, 0, 0,1 );
        }
        else if (stats.sync_status.status == "TxHashsetHeadersValidation") {
            pibdSyncPhase = true;
            return calcProgressStr( pibdSyncPhase, SYNC_STATE::VERIFY_HEADERS, 0, stats.sync_status.pos,  stats.sync_status.total );
        }
        else if (stats.sync_status.status == "TxHashsetKernelsPosValidation") {
            pibdSyncPhase = true;
            return calcProgressStr( pibdSyncPhase, SYNC_STATE::VERIFY_KERNEKLS_POS, 0, stats.sync_status.pos,  stats.sync_status.total );
        }
        else if (stats.sync_status.status == "TxHashsetRangeProofsValidation") {
            pibdSyncPhase = true;
            return calcProgressStr( pibdSyncPhase, SYNC_STATE::VERIFY_RANGEPROOFS, 0, stats.sync_status.pos,  stats.sync_status.total );
        }
        else if (stats.sync_status.status == "TxHashsetKernelsValidation") {
            pibdSyncPhase = true;
            return calcProgressStr( pibdSyncPhase, SYNC_STATE::VERIFY_KERNEL, 0, stats.sync_status.pos,  stats.sync_status.total );
        }
        else if (stats.sync_status.status == "BodySync") {
            return calcProgressStr( pibdSyncPhase, SYNC_STATE::GETTING_BLOCKS, stats.sync_status.archive_height, stats.sync_status.pos,  stats.sync_status.total);
        }
        else if (stats.sync_status.status == "NoSync") {
            pibdSyncPhase = false;
            return "Done";
        }
        else {
            Q_ASSERT(false); // unknown stage
            return "Unknown";
        }
    }
}


void MwcNode::reportNodeFatalError( QString message ) {
    core::getWndManager()->messageTextDlg("Embedded MWC-Node Error", message);
    mwc::closeApplication();
}

void MwcNode::submitStartNodeResult(QString errorStr, int context_id, bool isStartCancelled) {
    QMetaObject::invokeMethod(this,
    [this, errorStr, context_id, isStartCancelled] {
        if (!errorStr.isEmpty()) {
            this->nodeContextId = context_id;
            if (!isStartCancelled) {
                this->reportNodeFatalError(errorStr);
            }
        }
        else {
            Q_ASSERT(context_id>=0);
            this->nodeContextId = context_id;
            notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::INFO, "Embedded mwc-node was started" );
        }
        this->startingNode = QFuture<void>();
        Q_ASSERT(!this->startingNode.isValid());
    },
    Qt::QueuedConnection);
}

// Return status And progress String
QPair<ServerStats, QString> MwcNode::getServerStats() {
    if (nodeContextId<0) {
        return QPair<ServerStats, QString>(ServerStats(), "Starting");
    }

    mwc_api::ApiResponse<ServerStats> stat_data = node::get_server_stats(nodeContextId);
    ServerStats res;
    QString status = "Offline";
    if (stat_data.hasError()) {
        logger::logInfo(logger::MWC_NODE, "get_server_stats responded with error: " + stat_data.error);
    }
    else {
        res = stat_data.response;
        status = calculateRunningStatus( pibdSyncPhase, stat_data.response);
    }

    return QPair<ServerStats, QString>(res, status);
}

QString MwcNode::callForiegnApi(const QString & request) {
    mwc_api::ApiResponse<QString> res = process_api_call(nodeContextId, "POST", "/v2/foreign", request);
    if (res.hasError()) {
        logger::logInfo(logger::MWC_NODE, "process_api_call responded with error: " + res.error);
    }
    return res.response;
}

void MwcNode::waitCancelStart() {
    if (startingNode.isValid()) {
        logger::logInfo(logger::MWC_NODE, "Starting task is active, waiting for it to stop...");

        stoppingFlag.storeRelease(1);

        while (startingNode.isRunning()) {
            // wait for auto finish
            QThread::msleep(200);
            QCoreApplication::processEvents();
        }
        logger::logInfo(logger::MWC_NODE, "Starting thread is stopped");
        startingNode = QFuture<void>();
        Q_ASSERT(!startingNode.isValid());
        stoppingFlag.storeRelease(0);
    }
}



}

