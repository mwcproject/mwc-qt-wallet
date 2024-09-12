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
#include "../util/ioutils.h"
#include <QDir>
#include "../core/appcontext.h"
#include "../core/global.h"
#include "../core/Config.h"
#include "../core/Notification.h"
#include "../core/WndManager.h"
#include "../core/Config.h"
#include "../util/Log.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "MwcNodeConfig.h"
#include <QTimer>
#include <QCoreApplication>

namespace node {



MwcNode::MwcNode(const QString & _nodePath, core::AppContext * _appContext) :
        QObject(),
        appContext(_appContext),
        nodePath(_nodePath)
{
    // Let's check node status every minute
    startTimer( CHECK_NODE_PERIOD );

    nwManager = new QNetworkAccessManager();
    connect( nwManager, &QNetworkAccessManager::finished, this, &MwcNode::replyFinished, Qt::QueuedConnection );
    restartCounter = 0;
}

MwcNode::~MwcNode() {
    if (isRunning()) {
        stop();
    }
}

QString MwcNode::getLogsLocation() const {
    QPair<bool,QString> nodePath = getMwcNodePath(lastDataPath, lastUsedNetwork);
    if (!nodePath.first) {
        core::getWndManager()->messageTextDlg("Error", nodePath.second);
        QCoreApplication::exit();
        return "";
    }

    return nodePath.second + "mwc-server.log";
}


void MwcNode::start(const QString & dataPath, const QString & network, bool tor ) {
    qDebug() << "MwcNode::start for network " + network;

    lastUsedNetwork = network;
    lastTor = tor;
    nodeSecret = "";
    nodeWorkDir = "";
    nonEmittedOutput = "";
    lastProcessedEvent = tries::NODE_OUTPUT_EVENT::NONE;
    nodeStatusString = "Waiting";

    // Start the binary
    Q_ASSERT(nodeProcess == nullptr);
    Q_ASSERT(nodeOutputParser == nullptr);

    qDebug() << "Starting mwc-node  " << nodePath;

    respondTimelimit = QDateTime::currentMSecsSinceEpoch() + int64_t(START_TIMEOUT * config::getTimeoutMultiplier());
    if (tor)
        respondTimelimit += START_TOR_TIMEOUT;

    nodeNoPeersFailCounter = 0;
    nodeOutOfSyncCounter = 0;
    nodeHeight = 0;
    peersMaxHeight = 0;
    syncedBeforeHorizon = false;
    firstRecievedBlockIdx = -1;
    syncIsDone = false;
    maxBlockHeight = 0;
    initChainHeight = 0;

    // Creating process and starting
    nodeProcess = initNodeProcess(dataPath, network, tor);
    nodeOutputParser = new tries::NodeOutputParser();

    connect( nodeOutputParser, &tries::NodeOutputParser::nodeOutputGenericEvent, this, &MwcNode::nodeOutputGenericEvent, Qt::QueuedConnection);
}

void MwcNode::stop() {
    qDebug() << "MwcNode::stop ...";
    logger::logInfo( "MWC-NODE", "Stopping mwc-node process" );

    QCoreApplication::processEvents();

    nodeProcDisconnect();

    if (nodeProcess) {

        if (nodeProcess->state() == QProcess::Running) {
            qDebug() << "Stopping mwc-node...";

            // QT is event based. Here we want to have blocking sync call and it is a problem
            // Here we are calling process event loop on our own because of that
            sendRequest("StopMwcNode", getNodeSecret(), "/v1/status?action=stop_node", REQUEST_TYPE::POST);

            QCoreApplication::processEvents();

            // Waiting for ptocess to finish...
            int64_t startTime = QDateTime::currentMSecsSinceEpoch();
            int64_t limitTime = startTime + int64_t(1000*120*config::getTimeoutMultiplier()); // about 2 minutes is a reasonamble time to stop, if wee have tor, it is slow

            while( nodeProcess->state() == QProcess::Running ) {
                if (QDateTime::currentMSecsSinceEpoch() > limitTime) {
                    if ( core::getWndManager()->questionTextDlg("Warning", "Stopping mwc-node process takes longer than expected.\nContinue to wait?",
                               "Yes", "No",
                               "Continue to wait and let MWC Node more time to start",
                               "Don't wait and kill MWC Node process even it can corrupt the node data",
                               true, false) == core::WndManager::RETURN_CODE::BTN1) {
                        config::increaseTimeoutMultiplier();
                        limitTime = QDateTime::currentMSecsSinceEpoch() + int64_t(1000*20*config::getTimeoutMultiplier());
                    }
                    else {
                        break;
                    }
                }
                if (nodeProcess->waitForFinished( 50 ))
                    break;
                QCoreApplication::processEvents();
            }

            QCoreApplication::processEvents();

            while( nodeProcess->state() == QProcess::Running ) {
                nodeProcess->kill();
            }
        }
        qDebug() << "mwc-node is exited";

        notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::INFO, "Embedded mwc-node is stopped." );

        nodeProcess->deleteLater();
        nodeProcess = nullptr;
    }

    if ( nodeOutputParser) {
        nodeOutputParser->deleteLater();
        nodeOutputParser = nullptr;
    }

    QCoreApplication::processEvents();
}

// pass - provide password through env variable. If pass empty - nothing will be done
// paramsPlus - additional parameters for the process
QProcess * MwcNode::initNodeProcess(const QString & dataPath, const QString & network, bool tor ) {
    lastDataPath = dataPath;

    QPair<bool,QString> nodeDataFullPath = getMwcNodePath(dataPath, network);
    if (!nodeDataFullPath.first) {
        core::getWndManager()->messageTextDlg("Error", nodeDataFullPath.second);
        QCoreApplication::exit();
        return nullptr;
    }
    nodeWorkDir = nodeDataFullPath.second;
    MwcNodeConfig nodeConf = getCurrentMwcNodeConfig( dataPath, network, tor );

    QString nodeExecutablePath = QFileInfo(nodePath).canonicalFilePath();
    if (nodeExecutablePath.isEmpty()) {
        // file not found. Let's  report it clear way
        logger::logInfo("MWC-NODE", "error. mwc-node canonical path is empty");

        reportNodeFatalError( "mwc-node executable is not found. Expected location at:\n\n" + nodePath );
        return nullptr;
    }

    // Creating process and starting
    QProcess * process = new QProcess();
    process->setProcessChannelMode(QProcess::MergedChannels);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert( "TOR_EXE_NAME", config::getTorPath() );
    process->setProcessEnvironment(env);

    // Let's check if we are fine with directories

    nodeSecret = nodeConf.secret;

    // Working dir must match config file
    process->setWorkingDirectory(nodeWorkDir);

    nodeProcConnect(process);

    // >  mwc --floonet server --allow_to_stop run

    QStringList params;
    if (network.toLower().startsWith("floo"))
        params.push_back("--floonet");

    // Running as server with stop_node API
    params.push_back("server");
    params.push_back("--allow_to_stop");
    params.push_back("run");

    nodeStartTime = QDateTime::currentMSecsSinceEpoch();

    commandLine = nodeExecutablePath;
    for (auto & p : params) {
        commandLine += " '" + p + "'";
    }

    logger::logInfo( "MWC-NODE", "Starting mwc-node process: " + commandLine + "  at working dir:" + nodeWorkDir );

    process->start(nodePath, params, QProcess::Unbuffered | QProcess::ReadWrite );

    while ( ! process->waitForStarted( (int)( 10000 * config::getTimeoutMultiplier()) ) ) {
        logger::logInfo( "MWC-NODE", "Failed to start mwc-node process" );
        switch (process->error())
        {
            case QProcess::FailedToStart:
                reportNodeFatalError( "mwc-node failed to start. Mwc node expected location at " + nodePath + "\n\nCommand line:\n\n" + ">> cd '"+nodeWorkDir+"'\n>> " + commandLine  );
                return nullptr;
            case QProcess::Crashed:
                reportNodeFatalError( QString("mwc-node crashed during start\n\nCommand line:\n\n") + ">> cd '" + nodeWorkDir+"'\n>> " + commandLine );
                return nullptr;
            case QProcess::Timedout:
                if (core::getWndManager()->questionTextDlg(nullptr, "Warning", "Starting for mwc-node process is taking longer than expected.\nContinue to wait?"
                                "\n\nCommand line:\n\n" + QString(">> cd '") + nodeWorkDir+"'\n>> " + commandLine,
                                "Yes", "No",
                                "Wait more and give MWC Node more time to start",
                                "Don't wait and kill MWC Node process even it can corrupt the node data",
                                true, false) == core::WndManager::RETURN_CODE::BTN1) {
                    config::increaseTimeoutMultiplier();
                    continue; // retry with waiting
                }
                reportNodeFatalError( QString("mwc-node takes too much time to start. Something wrong with environment.\n\nCommand line:\n\n") + ">> cd '"+nodeWorkDir+"'\n>> " + commandLine );
                return nullptr;
            default:
                reportNodeFatalError( QString("mwc-node failed to start because of unknown error\n\nCommand line:\n\n") + ">> cd '"+nodeWorkDir+"'\n>> " + commandLine );
                return nullptr;
        }
    }

    return process;
}

QString MwcNode::getNodeSecret() {
    if (!nodeSecret.isEmpty())
        return nodeSecret;

    nodeSecret = "mwc-node-broken";
    //QString secretFn = nodeWorkDir + QDir::separator() + "api_secret";

    return nodeSecret;
}


void MwcNode::nodeProcDisconnect() {
    for (auto & cnt : processConnections) {
        disconnect(cnt);
    }
    processConnections.clear();
}

void MwcNode::nodeProcConnect(QProcess * process) {
    nodeProcDisconnect();

    Q_ASSERT(processConnections.isEmpty());
    Q_ASSERT(process);

    if (process) {
        processConnections.push_back( connect( process, &QProcess::errorOccurred, this, &MwcNode::nodeErrorOccurred, Qt::QueuedConnection) );

        processConnections.push_back(connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                                            SLOT(nodeProcessFinished(int, QProcess::ExitStatus))));

        processConnections.push_back( connect( process, &QProcess::readyReadStandardError, this, &MwcNode::mwcNodeReadyReadStandardError, Qt::QueuedConnection) );
        processConnections.push_back( connect( process, &QProcess::readyReadStandardOutput, this, &MwcNode::mwcNodeReadyReadStandardOutput, Qt::QueuedConnection) );

    }
}


void MwcNode::nodeErrorOccurred(QProcess::ProcessError error) {
    logger::logInfo("MWC-NODE", "Unable to start mwc-node process. ProcessError=" + QString::number(error) );
    qDebug() << "Unable to start mwc-node process. ProcessError=" << error;

    qDebug() << "ERROR OCCURRED. Error = " << error;

    if (nodeProcess) {
        logger::logInfo("MWC-NODE", "stdout: " + nodeProcess->readAllStandardOutput() );
        logger::logInfo("MWC-NODE", "stderr: " + nodeProcess->readAllStandardError() );

        nodeProcess->deleteLater();
        nodeProcess = nullptr;
    }

    reportNodeFatalError( "mwc-node process exited. Process error: "+ QString::number(error) +
                                  + "\n\nCommand line:\n\n" + ">> cd '"+nodeWorkDir+"'\n>> " + commandLine);
}

void MwcNode::nodeProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    logger::logInfo("MWC-NODE", "Exit with exit code " + QString::number(exitCode) + ", Exit status:" + QString::number(exitStatus) );
    qDebug() << "mwc-node is exiting with exit code " << exitCode << ", exitStatus=" << exitStatus;

    if (nodeProcess) {
        logger::logInfo("MWC-NODE", "stdout: " + nodeProcess->readAllStandardOutput() );
        logger::logInfo("MWC-NODE", "stderr: " + nodeProcess->readAllStandardError() );

        nodeProcess->deleteLater();
        nodeProcess = nullptr;
    }

    stop();

    if ( !isFinalRun() ) {
        // First try to stop another instamse if it is there
        restartCounter++;
        MwcNodeConfig mainnetConfig = getCurrentMwcNodeConfig(lastDataPath, "Mainnet", lastTor);
        MwcNodeConfig floonetConfig = getCurrentMwcNodeConfig(lastDataPath, "Floonet", lastTor);

        // Let's request other embedded local node to stop. There is a high chance that it is running and take the port.
        if (!mainnetConfig.secret.isEmpty())
            sendRequest("StopMainNet", mainnetConfig.secret, "/v1/status?action=stop_node", REQUEST_TYPE::POST);

        if (!floonetConfig.secret.isEmpty())
            sendRequest("StopFlooNet", floonetConfig.secret, "/v1/status?action=stop_node", REQUEST_TYPE::POST);

        if ( isFinalRun() ) {
            // Last try. Let's clean the data
            QDir dir(nodeWorkDir);
            dir.removeRecursively();
            notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::CRITICAL, "Embedded mwc-node data was cleaned, probably it was corrupted");
        }
        // restart the node in 20 seconds. Stopping takes time
        QTimer::singleShot(1000 * 20, this, &MwcNode::onRestartNode);
    }
    else {
        reportNodeFatalError( "mwc-node process exited due some unexpected error. The exit code: " + QString::number(exitCode) + "\n\n"
                              "Please check if you have enough disk space, and no antivirus preventing mwc-node to start.\n"
                              "Check if "+ (lastTor?"TOR process or ":"") + "another instance of mwc-node is already running. In this case please terminate that process, or reboot your computer.\n\n"
                              "If steps above didn't help, please try to clean up mwc-node data at\n" + nodeWorkDir +
                              "\n\nYou might use command line for troubleshooting:\n\n>> cd '" + nodeWorkDir + "'\n>> " +  commandLine + "\n\n");
    }
}

// One short timer to restart the node. Usinng instead of sleep
void MwcNode::onRestartNode() {
    // Because starting process is long, user might go forward, so in the progress might be another instance.
    // It is fine, this task is already executed
    if (!isRunning())
        start( lastDataPath, lastUsedNetwork, lastTor );
}


void MwcNode::mwcNodeReadyReadStandardError() {
    qDebug() << "get mwcNodeReadyReadStandardError call !!!";
    Q_ASSERT(nodeProcess);
    if (nodeProcess) {
        QString str( ioutils::FilterEscSymbols( nodeProcess->readAllStandardError() ) );

        reportNodeFatalError( "mwc-node process report error:\n" + str );
    }
}

void MwcNode::mwcNodeReadyReadStandardOutput() {
    if (nodeProcess) {
        QString str( ioutils::FilterEscSymbols( nodeProcess->readAllStandardOutput() ) );
        qDebug() << "Get output:" << str;
        logger::logMwcNodeOut(str);
        nodeOutputParser->processInput(str);

        nonEmittedOutput += str;

        QString ln;

        for (int t=0; t<nonEmittedOutput.length(); t++) {
            QChar ch = nonEmittedOutput[t];
            if ( ch=='\r' || ch=='\n' ) {
                if (!ln.isEmpty()) {
                    emit onMwcOutputLine(ln);
                    outputLines.push_front(ln);
                    while( outputLines.size() > 10000 ) // List should be OK with that. It is optimized for head/tail ops.
                        outputLines.pop_back();

                    ln = "";
                }
            }
            else {
                ln += ch;
            }
        }

        nonEmittedOutput = ln;
    }
}

enum class SYNC_STATE {GETTING_HEADERS, TXHASHSET_REQUEST, TXHASHSET_IN_PROGRESS, TXHASHSET_GET, GETTING_PIBD, VERIFY_RANGEPROOFS_FOR_TXHASHSET, VERIFY_KERNEL_SIGNATURES, GETTING_BLOCKS };
// return progress in the range [0-1.0]
static QString calcProgressStr( int initChainHeight , bool syncedBeforeHorizon, int peersMaxHeight, SYNC_STATE syncState, int value ) {
    // timing:
    // Headers: 11:00
    // range proofs download & anpack: 0:34
    // Verify ranges: 3:23
    // Getting Blocks: 2:50
    // Total time: 18:00

    const double getHeadersShare = 0.6;
    // Calculating shares for operations. Note, txHash stage is optional.
    double getTxHashShare, verifyRangeProofsShare, verifyKernelSignaturesShare;
    if (syncedBeforeHorizon) {
        getTxHashShare = 0.25;
        double totalShare = (1.0 - getHeadersShare-getTxHashShare - 0.05); // 0.05 for getting blocks
        Q_ASSERT(totalShare>0.0);
        verifyRangeProofsShare = 0.6 * totalShare;
        verifyKernelSignaturesShare = 0.4 * totalShare;
    }
    else {
        getTxHashShare = 0.0;
        verifyRangeProofsShare = 0.0;
        verifyKernelSignaturesShare = 0.0;
    }

    const double gettingBlocksShare = 1.0 - (getHeadersShare + getTxHashShare + verifyRangeProofsShare + verifyKernelSignaturesShare);
    Q_ASSERT( gettingBlocksShare >= 0.0 );

    double progressRes = 0.0;

    switch( syncState ) {
        case SYNC_STATE::GETTING_HEADERS: {
            //
            progressRes = double(std::max(0, value - initChainHeight)) / double(std::max(1, peersMaxHeight - initChainHeight)) * getHeadersShare  + 0.0;
            break;
        }
        case SYNC_STATE::TXHASHSET_REQUEST:
            progressRes = getHeadersShare;
            break;
        case SYNC_STATE::TXHASHSET_IN_PROGRESS:
            progressRes = getHeadersShare + getTxHashShare * (value/100.0);
            break;
        case SYNC_STATE::TXHASHSET_GET:
            progressRes = getHeadersShare + getTxHashShare;
            break;
        case SYNC_STATE::GETTING_PIBD:
            progressRes = getHeadersShare + getTxHashShare * (value/1000.0);
            break;
        case SYNC_STATE::VERIFY_RANGEPROOFS_FOR_TXHASHSET:
            progressRes = getHeadersShare + getTxHashShare +
                    (value/1000.0) * verifyRangeProofsShare;
            break;
        case SYNC_STATE::VERIFY_KERNEL_SIGNATURES:
            progressRes = getHeadersShare + getTxHashShare + verifyRangeProofsShare +
                    (value/1000.0) * verifyKernelSignaturesShare;
            break;
        case SYNC_STATE::GETTING_BLOCKS:
            progressRes = getHeadersShare + getTxHashShare + verifyRangeProofsShare + verifyKernelSignaturesShare +
                    (value/1000.0) * gettingBlocksShare;
            break;
    }

    if (progressRes<0.0)
        progressRes = 0.0;
    if (progressRes>1.0)
        progressRes = 1.0;


    return "Syncing " + QString::number( progressRes * 100.0, 'f', 1 ) + "%";
}

void MwcNode::nodeOutputGenericEvent( tries::NODE_OUTPUT_EVENT event, QString message) {

    int64_t nextTimeLimit = QDateTime::currentMSecsSinceEpoch();

    switch (event) {
        case tries::NODE_OUTPUT_EVENT::MWC_NODE_STARTED:
            nextTimeLimit += int64_t(MWC_NODE_STARTED_TIMEOUT * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;
            lastProcessedEvent = event;
            notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::INFO, "Embedded mwc-node was started" );
            break;
        case tries::NODE_OUTPUT_EVENT::WAITING_FOR_PEERS:
            lastProcessedEvent = event;
            nodeStatusString = "Waiting for peers";
            emit onMwcStatusUpdate(nodeStatusString);
            break;

        case tries::NODE_OUTPUT_EVENT::INITIAL_CHAIN_HEIGHT: {
            // update initial chain height

            // message: 365479725 @ 117749 [0099c40fb902]
            int idx1 = message.indexOf(" @ ");
            int idx2 = message.indexOf("[", idx1);
            Q_ASSERT(idx1>0 && idx2>0);
            if (idx1>0 && idx2>0) {
                idx1 += strlen(" @ ");
                initChainHeight = message.mid(idx1, idx2-idx1).trimmed().toInt();
            }
            break;
        }

        case tries::NODE_OUTPUT_EVENT::MWC_NODE_RECEIVE_HEADER: {
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO,
                                                  "Embedded mwc-node requesting headers to sync up");
            }
            nextTimeLimit += int64_t(MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;

            // We are getting headers during all stages. But we want to display progress only for the step 1.
            if (lastProcessedEvent <= event) {
                // for progress need to parse the second element
                QStringList params = message.split('|');
                int height = 0;
                if (params.size() >= 2) {
                    // 3.226.135.253:13414, height 31361
                    QString heightInfo = params[1].trimmed();
                    int pos = heightInfo.lastIndexOf(' ');
                    if (pos > 0) {
                        height = heightInfo.mid(pos + 1).toInt();

                        initChainHeight = std::min(initChainHeight, height);
                    }
                }

                nodeStatusString = "Getting headers";
                if (height > 0 && peersMaxHeight > 0)
                    nodeStatusString = calcProgressStr( initChainHeight , syncedBeforeHorizon, peersMaxHeight, SYNC_STATE::GETTING_HEADERS, height );

                emit onMwcStatusUpdate(nodeStatusString);
            }
            break;
        }
        case tries::NODE_OUTPUT_EVENT::ASK_FOR_TXHASHSET_ARCHIVE: {
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO,
                                                  "Embedded mwc-node requesting transaction archive");
            }
            // archive can be large, let's wait extra
            nextTimeLimit += int64_t( 3 * MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;
            syncedBeforeHorizon = true;

            nodeStatusString = calcProgressStr( initChainHeight , syncedBeforeHorizon, peersMaxHeight, SYNC_STATE::TXHASHSET_REQUEST, 0 );
            emit onMwcStatusUpdate(nodeStatusString);
            break;
        }
        case tries::NODE_OUTPUT_EVENT::TXHASHSET_ARCHIVE_IN_PROGRESS: {
            // archive can be large, let's wait extra
            nextTimeLimit += int64_t(MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;
            syncedBeforeHorizon = true;

            // for progress need to parse the second element
            QStringList params = message.split('|');
            Q_ASSERT(params.size()==2);
            if (params.size() >= 2) {
                // Downloading 624 MB chain state, done 118 MB
                // 624  118
                int total = params[0].toInt();
                int done = params[1].toInt();
                if (total>0 && done<total) {
                    nodeStatusString = calcProgressStr( initChainHeight , syncedBeforeHorizon, peersMaxHeight, SYNC_STATE::TXHASHSET_IN_PROGRESS, done * 100 / total );
                    emit onMwcStatusUpdate(nodeStatusString);
                }
            }
            break;
        }
        // expected no break
        case tries::NODE_OUTPUT_EVENT::HANDLE_TXHASHSET_ARCHIVE: {
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO,
                                                  "Embedded mwc-node processing transaction archive");
            }
            // tx Hash really might take a while to process
            nextTimeLimit += int64_t( 10 * MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;
            syncedBeforeHorizon = true;

            if (! message.contains("DONE") ) {
                nodeStatusString = calcProgressStr( initChainHeight , syncedBeforeHorizon, peersMaxHeight, SYNC_STATE::TXHASHSET_GET, 0 );
                emit onMwcStatusUpdate(nodeStatusString);
            }
            break;
        }
        case tries::NODE_OUTPUT_EVENT::MWC_NODE_RECEIVE_PIBD_BLOCK: {
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO,
                                                  "Embedded mwc-node requesting PIBD blocks to sync up");
            }
            nextTimeLimit += int64_t(MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;
            syncedBeforeHorizon = true;

            // We are getting headers during all stages. But we want to display progress only for the step 1.
            if (lastProcessedEvent <= event) {
                // for progress need to parse the second element
                QStringList params = message.split('|');
                if (params.size() >= 2) {
                    int64_t received = params[0].toInt();
                    int64_t total = params[1].toInt();

                    nodeStatusString = "Getting PIBD blocks";
                    if (received > 0 && total > 0 && received<total) {
                        nodeStatusString = calcProgressStr(initChainHeight, syncedBeforeHorizon, peersMaxHeight,
                                                           SYNC_STATE::GETTING_PIBD, int(received * 1000 / total));
                    }
                    else if (received==total) {
                        nodeStatusString = "Processing";  // calcProgressStr(initChainHeight, txhashsetHeight, peersMaxHeight, SYNC_STATE::GETTING_PIBD, 1000); // done with pibd, processing might take longer then usual
                        nextTimeLimit += int64_t(PIBD_IS_DONE * config::getTimeoutMultiplier());
                        // Me might wait for a while, nodeOutOfSyncCounter needs to be negative for that
                        nodeOutOfSyncCounter = -NODE_OUT_OF_SYNC_FAILURE_LIMIT*5;
                    }
                    emit onMwcStatusUpdate(nodeStatusString);
                }
            }
            break;
        }

        // expected no break
        case tries::NODE_OUTPUT_EVENT::VERIFY_RANGEPROOFS_FOR_TXHASHSET: {
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO,
                                                  "Embedded mwc-node validating range proofs");
            }

            // tx Hash really might take a while to process
            nextTimeLimit += int64_t( 10 * MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;
            syncedBeforeHorizon = true;

            QStringList params = message.split('|');
            if (params.size() >= 2) {
                int64_t received = params[0].toInt();
                int64_t total = params[1].toInt();

                if (received>0 && received<=total) {
                    nodeStatusString = calcProgressStr( initChainHeight , syncedBeforeHorizon, peersMaxHeight, SYNC_STATE::VERIFY_RANGEPROOFS_FOR_TXHASHSET, received*1000/total );
                    emit onMwcStatusUpdate(nodeStatusString);
                }
            }
            break;
        }
        case tries::NODE_OUTPUT_EVENT::VERIFY_KERNEL_SIGNATURES: {
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO,
                                                  "Embedded mwc-node validating kernel signatures");
            }

            nextTimeLimit += int64_t(MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;
            syncedBeforeHorizon = true;

            QStringList params = message.split('|');
            if (params.size() >= 2) {
                int64_t received = params[0].toInt();
                int64_t total = params[1].toInt();

                if (received>0 && received<=total) {
                    nodeStatusString = calcProgressStr(initChainHeight, syncedBeforeHorizon, peersMaxHeight,
                                                       SYNC_STATE::VERIFY_KERNEL_SIGNATURES, received*1000/total);
                    emit onMwcStatusUpdate(nodeStatusString);
                }
            }
            break;
        }
        case tries::NODE_OUTPUT_EVENT::RECEIVE_BLOCK_START: {
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage(bridge::MESSAGE_LEVEL::INFO,
                                                  "Embedded mwc-node processing blocks to sync up");
            }
            // expected no break
            nextTimeLimit += int64_t(MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;

            if (!syncIsDone) { // Async process, order is not guaranteed
                QStringList params = message.split('|');
                if (params.size() >= 2) {
                    int64_t received = params[0].toInt();
                    int64_t total = params[1].toInt();

                    if (received>=firstRecievedBlockIdx && received<=total) {
                        if (firstRecievedBlockIdx<0)
                            firstRecievedBlockIdx = received;

                        if (received>maxBlockHeight)
                            maxBlockHeight = received;

                        if (total>firstRecievedBlockIdx) {
                            nodeStatusString = calcProgressStr(initChainHeight, syncedBeforeHorizon, peersMaxHeight,
                                                               SYNC_STATE::GETTING_BLOCKS, (received-firstRecievedBlockIdx)*1000 / (total-firstRecievedBlockIdx) );
                            emit onMwcStatusUpdate(nodeStatusString);
                        }
                    }
                }
            }

            break;
        }
        case tries::NODE_OUTPUT_EVENT::SYNC_IS_DONE:{
            nextTimeLimit += int64_t(MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;

            syncIsDone = true;

            // message: 365444412 @ 117485 [0d4879faafaa]
            int idx1 = message.indexOf(" @ ");
            int idx2 = message.indexOf("[", idx1);
            Q_ASSERT(idx1>0 && idx2>0);
            if (idx1>0 && idx2>0) {
                idx1 += strlen(" @ ");
                int syncHeight = message.mid(idx1, idx2-idx1).trimmed().toInt();
                maxBlockHeight = std::max( maxBlockHeight, syncHeight );
            }

            updateRunningStatus();
            break;
        }

        case tries::NODE_OUTPUT_EVENT::RECEIVE_BLOCK_LISTEN: {
            nextTimeLimit += int64_t(RECEIVE_BLOCK_LISTEN * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0; // It is still normal syncronization, need to reset the counter

            // message: 2a695957b396 at 102204 from 34.238.121.224:13414 [in/out/kern: 0/1/1] going to process.
            int idx1 = message.indexOf("at ");
            int idx2 = message.indexOf(" from ");
            if (idx1 > 0 && idx2 > 0) {
                idx1 += strlen("at ");
                int handledH = message.mid(idx1, idx2 - idx1).toInt();
                initChainHeight = std::min(initChainHeight, handledH);

                if (handledH > maxBlockHeight) {
                    maxBlockHeight = handledH;

                    updateRunningStatus();
                }
            }
            break;
        }
        case tries::NODE_OUTPUT_EVENT::NETWORK_ISSUES:
            notify::appendNotificationMessage( bridge::MESSAGE_LEVEL::WARNING, "Embedded mwc-node experiencing network issues" );
            nextTimeLimit += int64_t(NETWORK_ISSUES * config::getTimeoutMultiplier());

            nodeStatusString = "Waiting for peers";
            emit onMwcStatusUpdate(nodeStatusString);

            break;
        case tries::NODE_OUTPUT_EVENT::ADDRESS_ALREADY_IN_USE:
            if (isFinalRun()) {
                QString message = "Unable to start local mwc-node because of error:\n"
                                  "'Address already in use'\n";
                if (lastTor) {
                    message += "Very likely tor process is already running. Please stop running tor process, or reboot your computer.";
                }
                else {
                    message += "It is likely that another mwc-node instance is still running, "
                               "another app active and using the same ports.";
                }

                reportNodeFatalError(message);
            }
            break;
        default:
            Q_ASSERT(false);
            break;
    }

    respondTimelimit = std::max(respondTimelimit, nextTimeLimit);
}

void MwcNode::updateRunningStatus() {
    Q_ASSERT(syncIsDone);

    QString newStatus;

    // tolerance - two blocks
    if (maxBlockHeight > 0) {
        if (maxBlockHeight >= peersMaxHeight - 2) {
            newStatus = "Ready";
        } else {
            newStatus = "waiting for " + QString::number(peersMaxHeight - maxBlockHeight) + " top blocks...";
        }
    }
    else {
        newStatus = "Waiting";
    }

    if (nodeStatusString != newStatus) {
        nodeStatusString = newStatus;
        emit onMwcStatusUpdate(nodeStatusString);
    }
}


void MwcNode::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event)

    if ( nodeProcess== nullptr || nodeOutputParser== nullptr )
        return;

    bool need2restart = false;

    // Check if timer expired and we need to logout...
    if ( respondTimelimit != 0 && QDateTime::currentMSecsSinceEpoch() > respondTimelimit ) {

        // Cold storage should be handles probably with Network issue message.
        // Will support that case later
        logger::logInfo("MwcNode", "Restarting node because no ONLINE activity was detected");
        need2restart = true;
    }
    if ( nodeNoPeersFailCounter > NODE_NO_PEERS_FAILURE_LIMITS || nodeOutOfSyncCounter > NODE_OUT_OF_SYNC_FAILURE_LIMIT ) {
        // need to restart
        logger::logInfo("MwcNode", "Restarting node because API didn't get expected info from the node during long time period");
        need2restart = true;
    }

    if (need2restart) {
        stop();
        start(lastDataPath, lastUsedNetwork, lastTor);
        return;
    }

    // Let's make API calls to verify the node status
    sendRequest( "Peers", getNodeSecret(), "/v1/peers/connected");
    sendRequest( "Status", getNodeSecret(), "/v1/status");
}

// Very simple request. No params, no body, no ssl
void MwcNode::sendRequest( const QString & tag, QString secret,
                const QString & api, REQUEST_TYPE reqType) {

    QString url = "http://localhost:13413" + api;

    qDebug() << "Sending request: " << url << "  tag:" << tag;

    QUrl requestUrl( url );

    QNetworkRequest request;

    logger::logInfo("MwcNode", "Requesting: " + requestUrl.toString());
    request.setUrl( requestUrl );
    request.setHeader(QNetworkRequest::ServerHeader, "application/json");

    // HTTP Basic authentication header value: base64(username:password)
    QString user = lastUsedNetwork.toLower().contains("floo") ? QString("mwcfloo") : QString("mwcmain");
    if (tag == "StopMainNet")
        user = "mwcmain";
    if (tag == "StopFlooNet")
        user = "mwcfloo";

    QString concatenated = user + ":" + secret;
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    QNetworkReply *reply = nullptr;
    if (reqType == REQUEST_TYPE::GET) {
        reply = nwManager->get(request);
    }
    else {
        // So far body is allways empty for us
        reply = nwManager->post(request, QByteArray() );
    }
    Q_ASSERT(reply);

    if (reply) {
        reply->setProperty("tag", QVariant(tag));
        // Respond will be send back async
    }
}

void MwcNode::replyFinished(QNetworkReply* reply) {
    // processing reply object first
    QNetworkReply::NetworkError errCode = reply->error();
    QString tag = reply->property("tag").toString();
    QString strReply (reply->readAll().trimmed());
    reply->deleteLater();
    reply = nullptr;

    if (errCode != QNetworkReply::NoError) {
        nodeNoPeersFailCounter++;
        return;
    }

    QJsonParseError error;
    QJsonDocument   jsonDoc = QJsonDocument::fromJson(strReply.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        nodeNoPeersFailCounter++;
        return;
    }

    if (tag == "Peers") {
        /*

   [ {"capabilities":{"bits":15},"user_agent":"MW/MWC 2.4.0","version":1,"addr":"34.238.121.224:13414","direction":"Outbound","total_difficulty":211876551,"height":103630},
         {"capabilities":{"bits":15},"user_agent":"MW/MWC 2.4.0-beta.1","version":1,"addr":"52.13.204.202:13414","direction":"Outbound","total_difficulty":211876551,"height":103630}]
         */

        QJsonArray  jsonRespond = jsonDoc.array();

        if (jsonRespond.size()>0) {
            for (int p = 0; p < jsonRespond.size(); p++) {
                QJsonObject peer = jsonRespond[p].toObject();
                int peerHeight = peer["height"].toInt();
                peersMaxHeight = std::max(peersMaxHeight , peerHeight);
            }

            if (peersMaxHeight > nodeHeight - 3) {
                nodeOutOfSyncCounter++;
            }

            if (syncIsDone)
                updateRunningStatus();

            if (nodeStatusString.contains("peers")) {
                nodeStatusString = "Found " + QString::number(jsonRespond.size()) + " peers";
                emit onMwcStatusUpdate(nodeStatusString);
            }
        }

    }
    else if (tag == "Status") {
        /*
{
  "protocol_version": 1,
  "user_agent": "MW/MWC 2.4.1-beta.1",
  "connections": 2,
  "tip": {
    "height": 103600,
    "last_block_pushed": "0a0e80db59108bae033927c0d5834425964e91adbf47c824bf05ae3c37cdd402",
    "prev_block_to_last": "342ff38e168c99984553f9ceaf5edf33d21db39b3de4d767a62a4a35c0f3b166",
    "total_difficulty": 211198808
  }
}
         */

        QJsonObject   jsonRespond = jsonDoc.object();

        int connections =   jsonRespond["connections"].toInt(0);
        nodeHeight =        jsonRespond["tip"].toObject()["height"].toInt(0);
        logger::logInfo("MwcNode", "MWC Node status: connections=" + QString::number(connections) +
                " height="+QString::number(nodeHeight));

        if (connections == 0)
            nodeNoPeersFailCounter++;
    }

}

void MwcNode::reportNodeFatalError( QString message ) {

    if ( config::isOnlineNode() ) {
        core::getWndManager()->messageTextDlg("Embedded MWC-Node Error", message);
    }
    else
    {
        if ( core::WndManager::RETURN_CODE::BTN2 == core::getWndManager()->questionTextDlg("Embedded MWC-Node Error",
                message + "\n\nIf Embedded mwc-node doesn't work for you, please switch to MWC Cloud Node before exit",
                "Keep Embedded", "Switch to Cloud",
                "I understand what is the problem and I want to keep connection to local embedded MWC Node",
                "Embedded node doesn't work for me, I prefer to switch to the public MWC node",
                false, true ) ) {

            // Switching to the cloud node
            wallet::MwcNodeConnection mwcNodeConnection = appContext->getNodeConnection( lastUsedNetwork );
            mwcNodeConnection.setAsCloud();
            appContext->updateMwcNodeConnection(lastUsedNetwork, mwcNodeConnection );
        }
    }

    mwc::closeApplication();

}




}

