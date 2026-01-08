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

#include "MwcNodeApi.h"
#include "../../mwc-wallet/mwc_wallet_lib/c_header/mwc_wallet_interface.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "../util/Log.h"
#include <QPair>

#include "util/stringutils.h"

namespace mwc_api {
    LogBufferedEntry::LogBufferedEntry(const QJsonObject & obj) {
        QJsonObject log_entry = obj["log_entry"].toObject();

        log = log_entry["log"].toString();
        level = log_entry["level"].toString();
        time_stamp = obj["time_stamp"].toInteger(0);
        id = obj["id"].toInteger(0);
    }

    QString LogBufferedEntry::toString() const {
        QString dump;
        QTextStream stream(&dump);
        stream << "LogBufferedEntry(log=" << log <<
            ", level=" << level <<
            ", time_stamp=" << time_stamp <<
            ", id=" << id <<
            ")";
        return dump;
    }


    QString LogBufferedEntry::toLogsLine() const {
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(time_stamp);
        QString time = dt.toString("yyyy-MM-dd hh:mm:ss.zzz");
        return time + " " + level + " " + log;
    }

    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    QString build_request(QString method_name, const QJsonObject & params, bool logRequest) {
        QJsonObject request;
        request["method"] = method_name;
        request["params"] = params;
        QString allocate_new_context_str = QJsonDocument(request).toJson(QJsonDocument::Compact);
        if (logRequest) {
            logger::logInfo( logger::MWC_NODE, QString("Calling method ") + method_name + " params" + allocate_new_context_str );
        }
        return allocate_new_context_str;
    }

    QPair<QJsonObject,QString> parseResponse(QString method_name, const char * result, bool logResponse) {
        if (logResponse)
            logger::logInfo(logger::MWC_NODE , QString("Response for method ") + method_name + ": " + util::string2shortStrR(result, 120) );

        QJsonParseError parseError;
        QJsonObject doc = QJsonDocument::fromJson(QByteArray(result), &parseError).object();
        Q_ASSERT(parseError.error == QJsonParseError::NoError);

        // Return response memory back to library.
        free_lib_string((char *)result);

        if (doc.value("success").toBool(false)) {
            return QPair<QJsonObject,QString>( doc.value("result").toObject(), "" );
        }
        else {
            return QPair<QJsonObject,QString>( QJsonObject(), doc.value("error").toString("") );
        }
    }

}

namespace node {
    ChainStats::ChainStats(const QJsonObject & obj ) {
        height = obj["height"].toInteger(0);
        last_block_h = obj["last_block_h"].toString();
        total_difficulty = obj["total_difficulty"].toInteger(0);
        latest_timestamp = obj["latest_timestamp"].toString();
    }

    QString ChainStats::toString() const {
        QString dump;
        QTextStream stream(&dump);
        stream << "ChainStats(height=" << height <<
            ", last_block_h=" << last_block_h <<
            ", total_difficulty=" << total_difficulty <<
            ", latest_timestamp=" << latest_timestamp <<
            ")";
        return dump;
    }

    SyncStatus::SyncStatus(const QJsonValue & stat ) {
        pos = total = archive_height = 0;
        if (stat.isString()) {
            status = stat.toString();
        }
        else {
            Q_ASSERT(stat.isObject());
            QJsonObject obj = stat.toObject();
            status = obj.keys().first();
            obj = obj.value(status).toObject();
            if (status == "HeaderHashSync") {
                pos = obj["completed_blocks"].toInteger(0);
                total = obj["total_blocks"].toInteger(0);
            }
            else if (status == "HeaderSync") {
                pos = obj["current_height"].toInteger(0);
                total = obj["archive_height"].toInteger(0);
            }
            else if (status == "TxHashsetPibd") {
                pos = obj["recieved_segments"].toInteger(0);
                total = obj["total_segments"].toInteger(0);
            }
            else if (status == "TxHashsetHeadersValidation") {
                pos = obj["headers"].toInteger(0);
                total = obj["headers_total"].toInteger(0);
            }
            else if (status == "TxHashsetKernelsPosValidation") {
                pos = obj["kernel_pos"].toInteger(0);
                total = obj["kernel_pos_total"].toInteger(0);
            }
            else if (status == "TxHashsetRangeProofsValidation") {
                pos = obj["rproofs"].toInteger(0);
                total = obj["rproofs_total"].toInteger(0);
            }
            else if (status == "TxHashsetKernelsValidation") {
                pos = obj["kernels"].toInteger(0);
                total = obj["kernels_total"].toInteger(0);
            }
            else if (status == "BodySync") {
                pos = obj["current_height"].toInteger(0);
                total = obj["highest_height"].toInteger(0);
                archive_height = obj["archive_height"].toInteger(0);
            }
            else {
                Q_ASSERT(false);
            }
        }
    }

    QString SyncStatus::toString() const {
        QString dump;
        QTextStream stream(&dump);
        stream << "SyncStatus(status=" << status <<
            ", pos=" << pos <<
            ", total=" << total <<
            ", archive_height=" << archive_height <<
            ")";
        return dump;
    }


    PeerStats::PeerStats(const QJsonObject & obj ) {
        state = obj["state"].toString();
        addr = obj["addr"].toString();
        user_agent = obj["user_agent"].toString();
        total_difficulty = obj["total_difficulty"].toInteger(0);
        height = obj["height"].toInteger(0);
        direction = obj["direction"].toString();
        last_seen = obj["last_seen"].toString();
        sent_bytes_per_sec = obj["sent_bytes_per_sec"].toInteger(0);
        received_bytes_per_sec = obj["received_bytes_per_sec"].toInteger(0);
        capabilities = obj["capabilities"].toInt(0);
    }

    QString PeerStats::toString() const {
        QString dump;
        QTextStream stream(&dump);
        stream << "PeerStats(state=" << state <<
            ", addr=" << addr <<
            ", user_agent=" << user_agent <<
            ", total_difficulty=" << total_difficulty <<
            ", height=" << height <<
            ", direction=" << direction <<
            ", last_seen=" << last_seen <<
            ", sent_bytes_per_sec=" << sent_bytes_per_sec <<
            ", received_bytes_per_sec=" << received_bytes_per_sec <<
            ", capabilities=" << capabilities;

        return dump;
    }

    ServerStats::ServerStats(const QJsonObject & obj ) {
        peer_count = obj["peer_count"].toInt(0);
        chain_stats = ChainStats( obj["chain_stats"].toObject() );
        header_stats = ChainStats( obj["header_stats"].toObject() );

        sync_status = SyncStatus(obj["sync_status"]);

        QJsonArray peers = obj["peer_stats"].toArray();
        for (int i=0; i<peers.size(); i++) {
            peer_stats.push_back( PeerStats(peers[i].toObject()) );
        }

        disk_usage_gb = obj["disk_usage_gb"].toString();
    }

    QString ServerStats::toString() const {
        QString dump;
        QTextStream stream(&dump);
        stream << "ServerStats(peer_count=" << peer_count <<
            ", chain_stats=" << chain_stats.toString() <<
                ", header_stats=" << header_stats.toString() <<
                    ", sync_status=" << sync_status.toString() <<
                        ", peer_stats[";

        for (const auto & p : peer_stats) {
            if (dump.back() != '[')
                stream << " ,";
            stream << p.toString();
        }

        stream << "]" <<
            ", disk_usage_gb=" << disk_usage_gb <<
                ")";

        return dump;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    static QString buildHookCbName(int context_id) {
        return QString("hook_cb_") + QString::number(context_id);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Create session context. Call it per node/wallet instance
    mwc_api::ApiResponse<int> create_context(QString chainNetworkName) {
        Q_ASSERT(!chainNetworkName.isEmpty());
        QJsonObject params;
        params["chain_type"] = chainNetworkName;

        QString request = mwc_api::build_request("create_context", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("create_context", result);

        if (response.second.isEmpty()) {
            return mwc_api::ApiResponse<int>( response.first["context_id"].toInt(-1) );
        }
        else {
            return mwc_api::ApiResponse<int>( -1, response.second );
        }
    }

    // Release created instance. Will stop all related running services
    mwc_api::ApiResponse<bool> release_context(int context_id) {
        QString cb_name = buildHookCbName(context_id);
        unregister_lib_callback(cb_name.toStdString().c_str());

        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("release_context", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("release_context", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    // logLevel values: Error, Warn, Info, Debug, Trace
    // callback will be called with: callback(logCallbackContext, <log line json>
    mwc_api::ApiResponse<bool> init_callback_logs(const QString & logLevel, int bufferLogSize,
        int8_t const * (*logCallback)(void *, int8_t const *), void * logCallbackContext )
    {
        // Register the callback function first
        register_lib_callback("log_callback", logCallback, logCallbackContext);

        //
        QJsonObject params;
        params["log_callback_name"] = "log_callback";
        params["log_level"] = logLevel;
        Q_ASSERT(bufferLogSize>0);
        params["log_buffer_size"] = bufferLogSize;

        QString request = mwc_api::build_request("init_callback_logs", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("init_callback_logs", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    // Reset callback logs.
    mwc_api::ApiResponse<bool> release_callback_logs() {
        unregister_lib_callback("log_callback");
        QString request = mwc_api::build_request("release_callback_logs", QJsonObject());
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("release_callback_logs", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    // Read some logs from the buffer
    mwc_api::ApiResponse<QVector<mwc_api::LogBufferedEntry>> get_buffered_logs(uint64_t last_known_id, int result_size_limit) {
        Q_ASSERT(result_size_limit>0);

        QJsonObject params;
        params["last_known_id"] = qint64(last_known_id);
        params["result_size_limit"] = result_size_limit;

        QString request = mwc_api::build_request("get_buffered_logs", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("get_buffered_logs", result);

        if (response.second.isEmpty()) {
            auto jarr = response.first["log_entries"].toArray();
            QVector<mwc_api::LogBufferedEntry> res;
            for (const auto & jitm : jarr) {
                res.push_back( mwc_api::LogBufferedEntry(jitm.toObject()));
            }
            return mwc_api::ApiResponse<QVector<mwc_api::LogBufferedEntry>>(res);
        }
        else {
            return mwc_api::ApiResponse<QVector<mwc_api::LogBufferedEntry>>( QVector<mwc_api::LogBufferedEntry>(), response.second );
        }
    }

    // Start tor process
    // base_dir - base data directory where tor related data can be stored
    // webtunnel_bridge - optional webtunnel collection string. Use it for extra privacy. See Tor documentation for format details.
    mwc_api::ApiResponse<bool> start_tor(const QString & base_dir, QString * webtunnel_bridge) {
        QJsonObject params;
        params["base_dir"] = base_dir;
        QJsonObject config;
        config["tor_enabled"] = true;
        config["tor_external"] = false;
        if (webtunnel_bridge)
            config["webtunnel_bridge"] = *webtunnel_bridge;
        params["config"] = config;

        QString request = mwc_api::build_request("start_tor", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("start_tor", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    // Accelerare Tor/Arti init related stopping so it will fail instead of finish
    // Need to use accelerate App fast start/stop
    mwc_api::ApiResponse<bool> shutdown_tor() {
        QJsonObject params;

        QString request = mwc_api::build_request("shutdown_tor", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("shutdown_tor", result);

        return mwc_api::ApiResponse<bool>( true, "" );
    }

    mwc_api::ApiResponse<QPair<bool, bool>> tor_status() {
        QJsonObject params;
        QString request = mwc_api::build_request("tor_status", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("tor_status", result);

        if (response.second.isEmpty()) {
            return mwc_api::ApiResponse<QPair<bool, bool>>( QPair<bool, bool>(response.first["started"].toBool(false), response.first["healthy"].toBool(false)) );
        }
        else {
            return mwc_api::ApiResponse<QPair<bool, bool>>( QPair<bool, bool>(false, false), response.second );
        }

    }


    // blockChainCallback receives events: header_received, block_received, transaction_received, block_accepted
    // Note, Create server here has minimal set of params. In reality config is large and if it is needed to expand the list,
    // update "create_server" on mwc-node side as well
    mwc_api::ApiResponse<bool> create_server(int context_id, const QString & db_root, const QString * onion_expanded_key,
                    int8_t const * (*blockChainCallback)(void *, int8_t const *), void * blockChainCallbackContext) {
        QJsonObject params;
        params["context_id"] = context_id;
        params["db_root"] = db_root;
        if (onion_expanded_key) {
            params["onion_expanded_key"] = *onion_expanded_key;
        }

        if (blockChainCallback) {
            QString cb_name = buildHookCbName(context_id);
            register_lib_callback( cb_name.toStdString().c_str(), blockChainCallback, blockChainCallbackContext);
            params["hook_callback_name"] = cb_name;
        }

        QString request = mwc_api::build_request("create_server", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("create_server", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    // Stop and release server
    mwc_api::ApiResponse<bool> release_server(int context_id) {
        QString cb_name = buildHookCbName(context_id);
        unregister_lib_callback(cb_name.toStdString().c_str());

        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("release_server", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("release_server", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    // Start stratum. Needed for the mining only. Not expect that somebody will call it.
    // In case it is needed, Update create server, add configs fir for stratum
    mwc_api::ApiResponse<bool> start_stratum(int context_id) {
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("start_stratum", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("start_stratum", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    // Start p2p discovery job
    mwc_api::ApiResponse<bool> start_discover_peers(int context_id) {
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("start_discover_peers", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("start_discover_peers", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    // Start node sync job
    mwc_api::ApiResponse<bool> start_sync_monitoring(int context_id) {
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("start_sync_monitoring", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("start_sync_monitoring", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    // Start peers listening job
    mwc_api::ApiResponse<bool> start_listen_peers(int context_id) {
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("start_listen_peers", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("start_listen_peers", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    // Start dandelion job
    mwc_api::ApiResponse<bool> start_dandelion(int context_id) {
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("start_dandelion", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("start_dandelion", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }

    // Start dandelion job
    mwc_api::ApiResponse<ServerStats> get_server_stats(int context_id) {
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("get_server_stats", params, false);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("get_server_stats", result, false);

        if (response.second.isEmpty()) {
            return mwc_api::ApiResponse<ServerStats>( ServerStats(response.first) );
        }
        else {
            return mwc_api::ApiResponse<ServerStats>( ServerStats(), response.second );
        }
    }

    // Init feature for process_api_call calls
    mwc_api::ApiResponse<bool> init_call_api(int context_id) {
        QJsonObject params;
        params["context_id"] = context_id;

        QString request = mwc_api::build_request("init_call_api", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("init_call_api", result);

        return mwc_api::ApiResponse<bool>( response.second.isEmpty(), response.second );
    }


    mwc_api::ApiResponse<QString> process_api_call(int context_id, const QString & method, const QString & uri, const QString & body) {
        QJsonObject params;
        params["context_id"] = context_id;
        params["method"] = method;
        params["uri"] = uri;
        params["body"] = body;

        QString request = mwc_api::build_request("process_api_call", params);
        char * result = process_mwc_node_request( request.toStdString().c_str() );
        QPair<QJsonObject,QString> response = mwc_api::parseResponse("process_api_call", result);

        QString responseBody;
        if (response.second.isEmpty()) {
            responseBody = response.first["response"].toString();
        }
        return mwc_api::ApiResponse<QString>( responseBody, response.second );
    }

}
