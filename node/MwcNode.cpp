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
    lastProcessedEvent = tries::NODE_OUTPUT_EVENT::NONE;

    // Start the binary
    Q_ASSERT(nodeProcess == nullptr);
    Q_ASSERT(nodeOutputParser == nullptr);

    qDebug() << "Starting mwc-node  " << nodePath;

    respondTimelimit = QDateTime::currentMSecsSinceEpoch() + START_TIMEOUT * config::getTimeoutMultiplier();

    nodeCheckFailCounter = 0;
    nodeOutOfSyncCounter = 0;
    lastKnownHeight = 0;

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
                notify::reportFatalError( "mwc-node failed to start. Mwc node expected location at " + nodePath );
                return nullptr;
            case QProcess::Crashed:
                notify::reportFatalError( "mwc-node crashed during start" );
                return nullptr;
            case QProcess::Timedout:
                if (control::MessageBox::question(nullptr, "Warning", "Starting for mwc-node process is taking longer than expected.\nContinue to wait?",
                                                  "Yes", "No", true, false) == control::MessageBox::RETURN_CODE::BTN1) {
                    config::increaseTimeoutMultiplier();
                    continue; // retry with waiting
                }
                notify::reportFatalError( "mwc-node takes too much time to start. Something wrong with environment." );
                return nullptr;
            default:
                notify::reportFatalError( "mwc-node failed to start because of unknown error" );
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

    notify::reportFatalError( "mwc-node process exited. Process error: "+ QString::number(error) );

}

void MwcNode::nodeProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    logger::logMwcNodeOut("Exit with exit code " + QString::number(exitCode) + ", Exit status:" + QString::number(exitStatus) );

    qDebug() << "mwc-node is exiting with exit code " << exitCode << ", exitStatus=" << exitStatus;

    if (nodeProcess) {
        nodeProcess->deleteLater();
        nodeProcess = nullptr;
    }

    notify::reportFatalError( "mwc-node process exited due some unexpected error. The exit code: " + QString::number(exitCode) + "\n\n"
                              "Please check if another instance of mwc-node is already running. In this case please terminate that process, or just reboot your computer.\n\n"
                              "If reboot doesn't help, please try to clean up mwc-node data at\n" + nodeWorkDir);
}

void MwcNode::mwcNodeReadyReadStandardError() {
    qDebug() << "get mwcNodeReadyReadStandardError call !!!";
    Q_ASSERT(nodeProcess);
    if (nodeProcess) {
        QString str( ioutils::FilterEscSymbols( nodeProcess->readAllStandardError() ) );

        notify::reportFatalError( "mwc713 process report error:\n" + str );
    }
}

void MwcNode::mwcNodeReadyReadStandardOutput() {
    if (nodeProcess) {
        QString str( ioutils::FilterEscSymbols( nodeProcess->readAllStandardOutput() ) );
        qDebug() << "Get output:" << str;
        logger::logMwcNodeOut(str);
        nodeOutputParser->processInput(str);
    }
}

void MwcNode::nodeOutputGenericEvent( tries::NODE_OUTPUT_EVENT event, QString message) {
    Q_UNUSED(message)

    int64_t nextTimeLimit = QDateTime::currentMSecsSinceEpoch();

    switch (event) {
        case tries::NODE_OUTPUT_EVENT::MWC_NODE_STARTED:
            nextTimeLimit += MWC_NODE_STARTED_TIMEOUT * config::getTimeoutMultiplier();
            nodeOutOfSyncCounter = 0;
            lastProcessedEvent = event;
            notify::appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, "Embedded mwc-node was started" );
            break;
        case tries::NODE_OUTPUT_EVENT::MWC_NODE_RECEIVE_HEADER:
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, "Embedded mwc-node requesting headers to sync up" );
            }
            // expected no break
        case tries::NODE_OUTPUT_EVENT::ASK_FOR_TXHASHSET_ARCHIVE:
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, "Embedded mwc-node requesting transaction archive" );
            }
            // expected no break
        case tries::NODE_OUTPUT_EVENT::HANDLE_TXHASHSET_ARCHIVE:
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, "Embedded mwc-node processing transaction archive" );
            }
            // expected no break
        case tries::NODE_OUTPUT_EVENT::VERIFY_RANGEPROOFS_FOR_TXHASHSET:
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, "Embedded mwc-node validating range proofs" );
            }
            // expected no break
        case tries::NODE_OUTPUT_EVENT::VERIFY_KERNEL_SIGNATURES:
            if (lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, "Embedded mwc-node validating kernel signatures" );
            }
            // expected no break
        case tries::NODE_OUTPUT_EVENT::RECIEVE_BLOCK_HEADERS_START:
        case tries::NODE_OUTPUT_EVENT::RECEIVE_BLOCK_START:
            if (  lastProcessedEvent < event) {
                lastProcessedEvent = event;
                notify::appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, "Embedded mwc-node proocessing blocks to sync up" );
            }
            // expected no break
            nextTimeLimit += MWC_NODE_SYNC_MESSAGES * config::getTimeoutMultiplier();
            nodeOutOfSyncCounter = 0;
            break;
        case tries::NODE_OUTPUT_EVENT::RECEIVE_BLOCK_LISTEN:
            nextTimeLimit += RECEIVE_BLOCK_LISTEN * config::getTimeoutMultiplier();
            break;
        case tries::NODE_OUTPUT_EVENT::NETWORK_ISSUES:
            notify::appendNotificationMessage( notify::MESSAGE_LEVEL::WARNING, "Embedded mwc-node experiencing network issues" );
            nextTimeLimit += NETWORK_ISSUES * config::getTimeoutMultiplier();
            break;
        case tries::NODE_OUTPUT_EVENT::ADDRESS_ALREADY_IN_USE:
            notify::reportFatalError("Unable to start local mwc-node because of error:\n"
                                     "'Address already in use'\n"
                                     "It is likely that another mwc-node instance is still running, "
                                     "or there is another app "
                                     "that is using the same ports.");
            break;
        default:
            Q_ASSERT(false);
            break;
    }

    respondTimelimit = std::max(respondTimelimit, nextTimeLimit);
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

    if ( nodeCheckFailCounter > API_FAILURE_LIMIT || nodeOutOfSyncCounter>API_FAILURE_LIMIT ) {
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
        nodeCheckFailCounter++;
        return;
    }

    QJsonParseError error;
    QJsonDocument   jsonDoc = QJsonDocument::fromJson(strReply.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        nodeCheckFailCounter++;
        return;
    }

    if (tag == "Peers") {
        /*

   [ {"capabilities":{"bits":15},"user_agent":"MW/MWC 2.4.0","version":1,"addr":"34.238.121.224:13414","direction":"Outbound","total_difficulty":211876551,"height":103630},
         {"capabilities":{"bits":15},"user_agent":"MW/MWC 2.4.0-beta.1","version":1,"addr":"52.13.204.202:13414","direction":"Outbound","total_difficulty":211876551,"height":103630}]
         */

        QJsonArray  jsonRespond = jsonDoc.array();

        for ( int p=0; p<jsonRespond.size(); p++ ) {
            QJsonObject peer = jsonRespond[p].toObject();
            int peerHeight = peer["heihgt"].toInt();
            if ( peerHeight > lastKnownHeight - 3 ) {
                nodeOutOfSyncCounter++;
                break;
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
        lastKnownHeight =        jsonRespond["tip"].toObject()["height"].toInt(0);
        logger::logInfo("MwcNode", "mwc node status: connections=" + QString::number(connections) +
                " height="+QString::number(lastKnownHeight));

        if (connections == 0)
            nodeCheckFailCounter++;
    }

}



}

