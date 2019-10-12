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
#include "../control/messagebox.h"
#include "../core/Notification.h"
#include "../util/Log.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "MwcNodeConfig.h"

namespace node {



MwcNode::MwcNode(QString _nodePath, core::AppContext * _appContext) :
        QObject(),
        appContext(_appContext),
        nodePath(_nodePath)
{
    // Let's check node status every minute
    startTimer( CHECK_NODE_PERIOD );

    nwManager = new QNetworkAccessManager();
    connect( nwManager, &QNetworkAccessManager::finished, this, &MwcNode::replyFinished, Qt::QueuedConnection );
}

MwcNode::~MwcNode() {
    if (isRunning()) {
        stop();
    }
}


void MwcNode::start( const QString & network ) {
    qDebug() << "MwcNode::start for network " + network;

    lastUsedNetwork = network;
    nodeSecret = "";
    nodeWorkDir = "";
    nonEmittedOutput = "";
    lastProcessedEvent = tries::NODE_OUTPUT_EVENT::NONE;

    // Start the binary
    Q_ASSERT(nodeProcess == nullptr);
    Q_ASSERT(nodeOutputParser == nullptr);

    qDebug() << "Starting mwc-node  " << nodePath;

    respondTimelimit = QDateTime::currentMSecsSinceEpoch() + int64_t(START_TIMEOUT * config::getTimeoutMultiplier());

    nodeNoPeersFailCounter = 0;
    nodeOutOfSyncCounter = 0;
    nodeHeight = 0;
    peersMaxHeight = 0;
    txhashsetHeight = 0;
    syncIsDone = false;
    maxBlockHeight = 0;
    initChainHeight = 0;

    // Creating process and starting
    nodeProcess = initNodeProcess(network);
    nodeOutputParser = new tries::NodeOutputParser();

    connect( nodeOutputParser, &tries::NodeOutputParser::nodeOutputGenericEvent, this, &MwcNode::nodeOutputGenericEvent, Qt::QueuedConnection);
}

void MwcNode::stop() {
    qDebug() << "MwcNode::stop ...";

    nodeProcDisconnect();

    if (nodeProcess) {
        nodeProcess->kill();
        if (!util::processWaitForFinished( nodeProcess, 30000, "mwc-node")) {
            nodeProcess->terminate();
        }
        qDebug() << "mwc-node is exited";

        notify::appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, "Embedded mwc-node is stopped" );

        nodeProcess->deleteLater();
        nodeProcess = nullptr;
    }

    if ( nodeOutputParser) {
        nodeOutputParser->deleteLater();
        nodeOutputParser = nullptr;
    }

}

// pass - provide password through env variable. If pass empty - nothing will be done
// paramsPlus - additional parameters for the process
QProcess * MwcNode::initNodeProcess( QString network ) {

    nodeWorkDir = getMwcNodePath(network);
    MwcNodeConfig nodeConf = getCurrentMwcNodeConfig( network );

    // Creating process and starting
    QProcess * process = new QProcess();
    process->setProcessChannelMode(QProcess::MergedChannels);

    // Let's check if we are fine with directories

    nodeSecret = nodeConf.secret;

    // Working dir must match config file
    process->setWorkingDirectory(nodeWorkDir);

    QStringList params;
    if (network.toLower().startsWith("floo"))
        params.push_back("--floonet");

    process->start(nodePath, params, QProcess::Unbuffered | QProcess::ReadWrite );

    while ( ! process->waitForStarted( (int)(10000 * config::getTimeoutMultiplier()) ) ) {
        switch (process->error())
        {
            case QProcess::FailedToStart:
                reportNodeFatalError( "mwc-node failed to start. Mwc node expected location at " + nodePath );
                return nullptr;
            case QProcess::Crashed:
                reportNodeFatalError( "mwc-node crashed during start" );
                return nullptr;
            case QProcess::Timedout:
                if (control::MessageBox::question(nullptr, "Warning", "Starting for mwc-node process is taking longer than expected.\nContinue to wait?",
                                                  "Yes", "No", true, false) == control::MessageBox::RETURN_CODE::BTN1) {
                    config::increaseTimeoutMultiplier();
                    continue; // retry with waiting
                }
                reportNodeFatalError( "mwc-node takes too much time to start. Something wrong with environment." );
                return nullptr;
            default:
                reportNodeFatalError( "mwc-node failed to start because of unknown error" );
                return nullptr;
        }
    }

    nodeProcConnect(process);

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
    logger::logMwcNodeOut("ERROR OCCURRED. Error = " + QString::number(error)  );

    qDebug() << "ERROR OCCURRED. Error = " << error;

    if (nodeProcess) {
        nodeProcess->deleteLater();
        nodeProcess = nullptr;
    }

    reportNodeFatalError( "mwc-node process exited. Process error: "+ QString::number(error) );

}

void MwcNode::nodeProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    logger::logMwcNodeOut("Exit with exit code " + QString::number(exitCode) + ", Exit status:" + QString::number(exitStatus) );

    qDebug() << "mwc-node is exiting with exit code " << exitCode << ", exitStatus=" << exitStatus;

    if (nodeProcess) {
        nodeProcess->deleteLater();
        nodeProcess = nullptr;
    }

    reportNodeFatalError( "mwc-node process exited due some unexpected error. The exit code: " + QString::number(exitCode) + "\n\n"
                              "Please check if another instance of mwc-node is already running. In this case please terminate that process, or just reboot your computer.\n\n"
                              "If reboot doesn't help, please try to clean up mwc-node data at\n" + nodeWorkDir);
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

void MwcNode::nodeOutputGenericEvent( tries::NODE_OUTPUT_EVENT event, QString message) {

    int64_t nextTimeLimit = QDateTime::currentMSecsSinceEpoch();

    switch (event) {
        case tries::NODE_OUTPUT_EVENT::MWC_NODE_STARTED:
            nextTimeLimit += int64_t(MWC_NODE_STARTED_TIMEOUT * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;
            lastProcessedEvent = event;
            notify::appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, "Embedded mwc-node was started" );
            break;
        case tries::NODE_OUTPUT_EVENT::WAITING_FOR_PEERS:
            lastProcessedEvent = event;
            nodeStatusString = "Waiting for peers to join";
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
                maxBlockHeight = initChainHeight;
            }
            break;
        }

        case tries::NODE_OUTPUT_EVENT::MWC_NODE_RECEIVE_HEADER: {
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage(notify::MESSAGE_LEVEL::INFO,
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
                    }
                }

                nodeStatusString = "Step 1 of 3, getting headers...";
                if (height > 0 && peersMaxHeight > 0)
                    nodeStatusString +=
                            " " + QString::number(double(std::max(0, height - initChainHeight)) /
                                                  double(std::max(1, peersMaxHeight - initChainHeight)) * 100.0, 'f',
                                                  1) + "%";

                emit onMwcStatusUpdate(nodeStatusString);
            }
            break;
        }
        case tries::NODE_OUTPUT_EVENT::ASK_FOR_TXHASHSET_ARCHIVE: {
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage(notify::MESSAGE_LEVEL::INFO,
                                                  "Embedded mwc-node requesting transaction archive");
            }
            nextTimeLimit += int64_t(MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;

            // for progress need to parse the second element
            QStringList params = message.split('|');
            if (params.size() >= 2) {
                // 114586 0a78e3f9d6c5.
                QString heightInfo = params[1].trimmed();
                int pos = heightInfo.indexOf(' ');
                if (pos > 0) {
                    txhashsetHeight = heightInfo.left(pos).toInt();
                }
            }

            nodeStatusString = "Step 2 of 3, requesting txhashset archive... 0.5%";
            emit onMwcStatusUpdate(nodeStatusString);
            break;
        }
            // expected no break
        case tries::NODE_OUTPUT_EVENT::HANDLE_TXHASHSET_ARCHIVE: {
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage(notify::MESSAGE_LEVEL::INFO,
                                                  "Embedded mwc-node processing transaction archive");
            }
            nextTimeLimit += int64_t(MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;

            if (! message.contains("DONE") ) {
                nodeStatusString = "Step 2 of 3, extracting txhashset archive... 2.5%";
                emit onMwcStatusUpdate(nodeStatusString);
            }
            break;
        }
            // expected no break
        case tries::NODE_OUTPUT_EVENT::VERIFY_RANGEPROOFS_FOR_TXHASHSET: {
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage(notify::MESSAGE_LEVEL::INFO,
                                                  "Embedded mwc-node validating range proofs");
            }

            nextTimeLimit += int64_t(MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;

            int handledH = message.trimmed().toInt();
            if (handledH>0 && handledH<txhashsetHeight) {
                double progress = 5.0 + double( std::max(0, handledH-initChainHeight) ) / double( std::max(1, txhashsetHeight-initChainHeight) ) * 60.0; // 5-65 range
                nodeStatusString = "Step 2 of 3, verifying rangeproofs... " + QString::number(progress, 'f', 1) + "%";
                emit onMwcStatusUpdate(nodeStatusString);
            }
            break;
        }
        case tries::NODE_OUTPUT_EVENT::VERIFY_KERNEL_SIGNATURES: {
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage(notify::MESSAGE_LEVEL::INFO,
                                                  "Embedded mwc-node validating kernel signatures");
            }

            nextTimeLimit += int64_t(MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;

            int handledH = message.trimmed().toInt();
            if (handledH>0 && handledH<txhashsetHeight) {
                double progress = 65.0 + double( std::max(0, handledH-initChainHeight) ) / double( std::max(1, txhashsetHeight-initChainHeight) ) * 35.0; // 65-100 range
                nodeStatusString = "Step 2 of 3, verifying kernels... " + QString::number(progress, 'f', 1) + "%";
                emit onMwcStatusUpdate(nodeStatusString);
            }
            break;
        }
        case tries::NODE_OUTPUT_EVENT::RECEIVE_BLOCK_START: {
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage(notify::MESSAGE_LEVEL::INFO,
                                                  "Embedded mwc-node processing blocks to sync up");
            }
            // expected no break
            nextTimeLimit += int64_t(MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier());
            nodeOutOfSyncCounter = 0;

            if (!syncIsDone) { // Async process, order is not guaranteed

                // Message: 140e019e22d0 at 114601 from 52.13.204.202:13414 [in/out/kern: 0/1/1] going to process.
                int idx1 = message.indexOf("at ");
                int idx2 = message.indexOf(" from ");
                if (idx1 > 0 && idx2 > 0) {
                    idx1 += strlen("at ");
                    int handledH = message.mid(idx1, idx2 - idx1).toInt();

                    if (handledH>maxBlockHeight) {
                        maxBlockHeight = handledH;
                        if (handledH > 0 && handledH >= txhashsetHeight && handledH < peersMaxHeight) {
                            double progress =
                                    double( std::max(0, handledH - std::max(initChainHeight, txhashsetHeight) )) /
                                        double( std::max(1, peersMaxHeight - std::max(initChainHeight, txhashsetHeight)) ) *
                                    100.0;
                            nodeStatusString = "Step 3 of 3, getting blocks... " + QString::number(progress, 'f', 1) + "%";
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

            // message: 2a695957b396 at 102204 from 34.238.121.224:13414 [in/out/kern: 0/1/1] going to process.
            int idx1 = message.indexOf("at ");
            int idx2 = message.indexOf(" from ");
            if (idx1 > 0 && idx2 > 0) {
                idx1 += strlen("at ");
                int handledH = message.mid(idx1, idx2 - idx1).toInt();

                if (handledH > maxBlockHeight) {
                    maxBlockHeight = handledH;

                    updateRunningStatus();
                }
            }
            break;
        }
        case tries::NODE_OUTPUT_EVENT::NETWORK_ISSUES:
            notify::appendNotificationMessage( notify::MESSAGE_LEVEL::WARNING, "Embedded mwc-node experiencing network issues" );
            nextTimeLimit += int64_t(NETWORK_ISSUES * config::getTimeoutMultiplier());

            nodeStatusString = "Unable connect to peers. Waiting for peers to connect...";
            emit onMwcStatusUpdate(nodeStatusString);

            break;
        case tries::NODE_OUTPUT_EVENT::ADDRESS_ALREADY_IN_USE:
            reportNodeFatalError("Unable to start local mwc-node because of error:\n"
                                     "'Address already in use'\n"
                                     "It is likely that another mwc-node instance is still running, "
                                     "or there is another app active "
                                     "and using the same ports.");
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
    if (maxBlockHeight >= peersMaxHeight - 2) {
        newStatus = "In sync and running...";
    } else {
        newStatus = "Running and waiting for " + QString::number(peersMaxHeight - maxBlockHeight) + " top blocks...";
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
        start(lastUsedNetwork);
        return;
    }

    // Let's make API calls to verify the node status
    sendRequest( "Peers", getNodeSecret(), "/v1/peers/connected");
    sendRequest( "Status", getNodeSecret(), "/v1/status");
}

// Very simple request. No params, no body, no ssl
void MwcNode::sendRequest( const QString & tag, QString secret,
                const QString & api) {

    QString url = "http://localhost:13413" + api;

    qDebug() << "Sending request: " << url << "  tag:" << tag;

    QUrl requestUrl( url );

    QNetworkRequest request;

    logger::logInfo("MwcNode", "Requesting: " + requestUrl.toString());
    request.setUrl( requestUrl );
    request.setHeader(QNetworkRequest::ServerHeader, "application/json");

    // HTTP Basic authentication header value: base64(username:password)
    QString concatenated = (lastUsedNetwork.toLower().contains("floo") ? QString("mwcfloo") : QString("mwcmain")) + ":" + secret;
    QByteArray data = concatenated.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    QNetworkReply *reply = nwManager->get(request);
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
                nodeStatusString = "Connected to " + QString::number(jsonRespond.size()) + " peers, waiting for data...";
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
        logger::logInfo("MwcNode", "mwc node status: connections=" + QString::number(connections) +
                " height="+QString::number(nodeHeight));

        if (connections == 0)
            nodeNoPeersFailCounter++;
    }

}

void MwcNode::reportNodeFatalError( QString message ) {

    if ( control::MessageBox::RETURN_CODE::BTN2 == control::MessageBox::question(nullptr, "Embedded MWC-Node Error",
            message + "\n\nIf Embedded mwc-node doesn't work for you, please switch to MWC Cloud node before exit", "Keep Embedded", "Switch to Cloud", false, true ) ) {

        // Switching to the cloud node
        wallet::MwcNodeConnection mwcNodeConnection = appContext->getNodeConnection( lastUsedNetwork );
        mwcNodeConnection.setData( wallet::MwcNodeConnection::NODE_CONNECTION_TYPE::CLOUD );
        appContext->updateMwcNodeConnection(lastUsedNetwork, mwcNodeConnection );
    }


    mwc::closeApplication();

}




}

