// Copyright 2025 The MWC Developers
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


#include "node_client.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRandomGenerator>

#include "MwcNode.h"
#include "MwcNodeApi.h"
#include "core/Config.h"
#include "core/Notification.h"
#include "util/httpclient.h"
#include "util/Log.h"
#include "../wallet/wallet.h"

namespace node {

const int NODE_SYNC_CHECK_INTERVAL_SEC = 60;

const QStringList & getPublicNodes(const QString &network) {
    static const QStringList mainNetPublicNodes = {
        "https://mwc713.mwc.mw",
        "https://mwc7132.mwc.mw",
        "https://mwc7134.mwc.mw",
        "https://mwc7135.mwc.mw"
    };

    static const QStringList flooNetPublicNodes = {
        //"https://mwc713.floonet.mwc.mw",
        "http://13.236.73.43:13413"
    };

    return network.compare("Mainnet", Qt::CaseInsensitive) == 0
               ? mainNetPublicNodes
               : flooNetPublicNodes;
}

void updatePablicNodeLatency() {

}

const QString & getAuthorizationValue(const QString &network) {
    static const QString publicNodeSecret = "11ne3EAUtOXVKwhxm84U";
    static const QString mainNetAuthValue = QString::fromLatin1(
                       QString("mwcmain:%1")
                           .arg(publicNodeSecret)
                           .toUtf8()
                           .toBase64()
                    );
    static const QString flooNetAuthValue = QString::fromLatin1(
                       QString("mwcfloo:%1")
                           .arg(publicNodeSecret)
                           .toUtf8()
                           .toBase64()
                    );

    return network.compare("Mainnet", Qt::CaseInsensitive) == 0
               ? mainNetAuthValue
               : flooNetAuthValue;
}

NodeClient::NodeClient(QString _network, node::MwcNode * _embeddedNode, wallet::Wallet * _wallet) :
    network(_network), embeddedNode(_embeddedNode), wallet(_wallet)
{
    Q_ASSERT(wallet);
    Q_ASSERT(embeddedNode);
    publicNodeIdx = QRandomGenerator::global()->bounded( getPublicNodes(network).size() );

    QString publicNodeClientUrl = getPublicNodes(network)[publicNodeIdx];
    publicNodeClient = new util::HttpClient(publicNodeClientUrl);
}

NodeClient::~NodeClient() {
    QMutexLocker locker(&callMutex);
    if (publicNodeClient!=nullptr) {
        delete publicNodeClient;
        publicNodeClient = nullptr;
    }
}

void NodeClient::updateNodeSelectionLocked(int retry) {
    if (QDateTime::currentSecsSinceEpoch() > nodeStatusCheckTime) {
        if (config::isOnlineWallet()) {
            QPair<ServerStats, QString> embeddedStats = embeddedNode->getServerStats();
            qint64 embeddedNodeHeight = embeddedStats.first.chain_stats.height;

            logger::logInfo(logger::QT_WALLET, "Update node selection call");

            // Checking public node stats
            QString resJStr = callPublicNodeForeignApiLocked("{\"jsonrpc\": \"2.0\",\"method\": \"get_tip\",\"params\": [],\"id\": 1}");
            QJsonParseError parseError;
            QJsonObject res = resJStr.isEmpty() ? QJsonObject() : QJsonDocument::fromJson(resJStr.toUtf8(), &parseError).object();
            Q_ASSERT(parseError.error == QJsonParseError::NoError);

            qint64 publicNodeHeight = 0;
            if (!res.isEmpty()) {
                publicNodeHeight = res["result"].toObject()["Ok"].toObject()["height"].toInteger();
            }
            else {
                // Changing public node
                if (retry>0) {
                    publicNodeIdx = (publicNodeIdx + 1) % getPublicNodes(network).size();
                    if (publicNodeClient!=nullptr) {
                        delete publicNodeClient;
                    }
                    QString publicNodeClientUrl = getPublicNodes(network)[publicNodeIdx];
                    publicNodeClient = new util::HttpClient(publicNodeClientUrl);
                    updateNodeSelectionLocked(retry-1);
                    return;
                }
            }

            usePublicNode = (embeddedNodeHeight < publicNodeHeight-2);

            logger::logInfo(logger::QT_WALLET, "Embedded node hight: " + QString::number(embeddedNodeHeight) +
                " public node height: " + QString::number(publicNodeHeight) + "  Selected: " + (usePublicNode ? "public" : "embedded"));
        }
        else {
            Q_ASSERT( config::isColdWallet() || config::isOnlineNode() );
            // Both onlineNode & cold wallet Does use ONLY the internal node, no reasons to use public nodes
            // Online node needs to archive the blockchain, cold wallet can't connect
            usePublicNode = false;
        }

        nodeStatusCheckTime = QDateTime::currentSecsSinceEpoch() + NODE_SYNC_CHECK_INTERVAL_SEC;
    }
}


QString NodeClient::foreignApiRequest(const QString & request) {

    if (wallet->getStartStatus() == wallet::Wallet::STARTED_MODE::OFFLINE) {
        return "{}"; // empty response, wallet is exiting
    }

    QMutexLocker locker(&callMutex);

    updateNodeSelectionLocked();

    bool skipLogs = request.contains("get_tip") || request.contains("get_header");

    QString res;
    if (usePublicNode) {
        res = callPublicNodeForeignApiLocked(request).remove('\n');
        if (!skipLogs) {
            logger::logInfo(logger::NODE_CLIENT, "Public Node IO: " + util::string2shortStrR(request, 50) + "   Response: " + util::string2shortStrR( res, 70) );
        }
    }
    else {
        res = embeddedNode->callForiegnApi(request).remove('\n');
        if (!skipLogs) {
            logger::logInfo(logger::NODE_CLIENT, "Embedded Node IO: " + util::string2shortStrR(request, 50) + "   Response: " + util::string2shortStrR( res, 70) );
        }
    }

    if (request.contains("\"get_tip\"")) {
        if (!res.isEmpty()) {
            QJsonParseError parseError;
            QJsonObject doc = QJsonDocument::fromJson(res.toUtf8(), &parseError).object();
            Q_ASSERT(parseError.error == QJsonParseError::NoError);

            qint64 height = doc["result"].toObject()["Ok"].toObject()["height"].toInteger(0);
            if (height>0) {
                lastNodeHeight = height;
            }
        }
    }

    return res;
}

QString NodeClient::callPublicNodeForeignApiLocked(const QString & request) {
    if (publicNodeClient == nullptr) {
        Q_ASSERT(false);
        return "";
    }

    QString res = publicNodeClient->sendRequest(util::HttpClient::POST, "/v2/foreign",
                          {}, // key/value
                          {"Content-Type", "application/json", // key/value
                              "Connection", "keep-alive",
                              "Authorization", "Basic " + getAuthorizationValue(network)
                          },
                          request.toUtf8(),
                          30000,
                          false);

    return res;
}

wallet::NodeStatus NodeClient::requestNodeStatus() {
    QMutexLocker locker(&callMutex);
    return requestNodeStatusLocked();
}

wallet::NodeStatus NodeClient::requestNodeStatusLocked() {

    updateNodeSelectionLocked();
    wallet::NodeStatus result;
    if (usePublicNode) {
        result.internalNode = false;
        QString tipJStr = callPublicNodeForeignApiLocked("{\"jsonrpc\": \"2.0\",\"method\": \"get_tip\",\"params\": [],\"id\": 1}");
        QJsonParseError parseError;
        QJsonObject tip = tipJStr.isEmpty() ? QJsonObject() : QJsonDocument::fromJson(tipJStr.toUtf8(), &parseError).object();
        Q_ASSERT(parseError.error == QJsonParseError::NoError);

        QJsonObject peers;
        if (!tip.isEmpty()) {
            QString peersJStr = callPublicNodeForeignApiLocked("{\"jsonrpc\": \"2.0\",\"method\": \"get_connected_peers\",\"params\": [],\"id\": 1}");
            peers = peersJStr.isEmpty() ? QJsonObject() : QJsonDocument::fromJson(peersJStr.toUtf8(), &parseError).object();
            Q_ASSERT(parseError.error == QJsonParseError::NoError);
        }

        if (!tip.isEmpty() && !peers.isEmpty()) {
            result.nodeHeight = tip["result"].toObject()["Ok"].toObject()["height"].toInteger();
            result.totalDifficulty = tip["result"].toObject()["Ok"].toObject()["total_difficulty"].toInteger();

            QJsonArray peersData = peers["result"].toObject()["Ok"].toArray();
            result.connections = peersData.size();
            result.peerHeight = 0;
            result.online = peersData.size()>3;
            for (int k=0; k<peersData.size(); k++) {
                QJsonObject peerData = peersData[k].toObject();
                qint64 peer_height = peerData["height"].toInteger();
                result.peerHeight = std::max((qint64) result.peerHeight, (qint64) peer_height);
            }

            lastNodeHeight = result.nodeHeight;
        }
    }

    QPair<ServerStats, QString> statsRes = embeddedNode->getServerStats();
    if (!usePublicNode) {
        ServerStats stats = statsRes.first;
        result.internalNode = true;
        result.online = (config::isColdWallet() && stats.chain_stats.height>0) || stats.peer_stats.size()>0;

        if (config::isOnlineWallet()) {
            if (!result.online) {
                // switching to puplic node
                usePublicNode = true;
                return requestNodeStatusLocked();
            }
        }
        result.nodeHeight = stats.chain_stats.height;
        result.peerHeight = 0;
        for (const auto & peer : stats.peer_stats) {
            result.peerHeight = std::max((qint64) result.peerHeight, (qint64) peer.height);
        }
        result.totalDifficulty = stats.chain_stats.total_difficulty;
        result.connections = stats.peer_count;
        lastNodeHeight = result.nodeHeight;
    }

    lastInternalNodeState = statsRes.second;
    if (usePublicNode && lastInternalNodeState == "Ready") {
        qint64 curTime = QDateTime::currentSecsSinceEpoch();
        if ( nodeStatusCheckTime > curTime+3)
            nodeStatusCheckTime = curTime+3;
    }
    lastStatus = result;

    lastStatusTime = QDateTime::currentSecsSinceEpoch();
    return result;
}

bool NodeClient::isNodeHealthy() {
   if (lastStatusTime < QDateTime::currentSecsSinceEpoch() - 30) {
       NodeClient::requestNodeStatus();
   }
   return lastStatus.isHealthy();
}

QString NodeClient::getLastInternalNodeState() const {
    QMutexLocker locker(&callMutex);
    return lastInternalNodeState;
}


}
