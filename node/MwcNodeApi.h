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

#ifndef MWC_QT_WALLET_MWCNODEAPI_H
#define MWC_QT_WALLET_MWCNODEAPI_H

#include <QString>
#include <QVector>
#include <QJsonObject>

namespace mwc_api {
    template <class T>
    struct ApiResponse {
        T response;
        QString error;

        ApiResponse() = default;
        ApiResponse(const ApiResponse &) = default;
        ApiResponse(ApiResponse &&) = default;
        ApiResponse & operator = (const ApiResponse &) = default;

        ApiResponse(const T &resp, const QString &err = QString())
            : response(resp), error(err) {}

        bool hasError() const { return !error.isEmpty(); }
    };

    struct LogBufferedEntry {
        QString log;
        QString level;
        uint64_t time_stamp = 0;
        uint64_t id = 0;

        LogBufferedEntry(const QJsonObject & obj);
        QString toString() const;
        QString toLogsLine() const;
    };

    QString build_request(QString method_name, const QJsonObject & params, bool logRequest = true);

    QPair<QJsonObject,QString> parseResponse(QString method_name, const char * result, bool logResponse = true);
}


namespace node {


    struct ChainStats {
        int64_t height = -1;
        QString last_block_h;
        uint64_t total_difficulty = 0;
        QString latest_timestamp;

        ChainStats() {}
        ChainStats(const QJsonObject & obj );
        QString toString() const;
    };

    struct SyncStatus {
        QString status; // Sync stage name. See SyncStatus at  mwc-node/chain/src/types.rs
        uint64_t pos = 0; // whatever current value
        uint64_t total = 0; // total number of the same units
        uint64_t archive_height = 0; // if known, archive height

        SyncStatus() {}
        SyncStatus(const QJsonValue & obj );
        QString toString() const;
    };

    struct PeerStats {
        QString state;
        QString addr;
        QString user_agent;
        uint64_t total_difficulty = 0;
        int64_t height = -1;
        QString direction;
        QString last_seen;
        uint64_t sent_bytes_per_sec = 0;
        uint64_t received_bytes_per_sec = 0;
        int capabilities;

        PeerStats() {}
        PeerStats(const QJsonObject & obj );
        QString toString() const;
    };

    struct ServerStats {
        int peer_count = -1;
        ChainStats chain_stats;
        ChainStats header_stats;
        SyncStatus sync_status;
        QVector<PeerStats> peer_stats;
        QString disk_usage_gb;

        ServerStats() {}
        ServerStats(const QJsonObject & obj );
        QString toString() const;
    };

    // Here is a basic MWC node API to support embedded mwc node
    // Returns context_id
    mwc_api::ApiResponse<int> create_context(QString chainNetworkName);

    // Release created instance. Will stop all related running services
    mwc_api::ApiResponse<bool> release_context(int context_id);

    // logLevel values: Error, Warn, Info, Debug, Trace
    // callback will be called with: callback(logCallbackContext, <log line json>
    mwc_api::ApiResponse<bool> init_callback_logs(const QString & logLevel, int bufferLogSize,
                        int8_t const * (*logCallback)(void *, int8_t const *), void * logCallbackContext );

    // Reset callback logs.
    mwc_api::ApiResponse<bool> release_callback_logs();

    // Read some logs from the buffer
    mwc_api::ApiResponse<QVector<mwc_api::LogBufferedEntry>> get_buffered_logs(uint64_t last_known_id, int result_size_limit);

    // Start tor process
    // base_dir - base data directory where tor related data can be stored
    // webtunnel_bridge - optional webtunnel collection string. Use it for extra privacy. See Tor documentation for format details.
    mwc_api::ApiResponse<bool> start_tor(const QString & base_dir, QString * webtunnel_bridge);

    // Accelerare Tor/Arti init related stopping so it will fail instead of finish
    // Need to use accelerate App fast start/stop
    mwc_api::ApiResponse<bool> shutdown_tor();

    mwc_api::ApiResponse<QPair<bool, bool>> tor_status();

    // blockChainCallback receives events: header_received, block_received, transaction_received, block_accepted
    // Note, Create server here has minimal set of params. In reality config is large and if it is needed to expand the list,
    // update "create_server" on mwc-node side as well
    mwc_api::ApiResponse<bool> create_server(int context_id, const QString & db_root, const QString * onion_expanded_key,
                    int8_t const * (*blockChainCallback)(void *, int8_t const *), void * blockChainCallbackContext);

    // Stop and release server
    mwc_api::ApiResponse<bool> release_server(int context_id);

    // Start stratum. Needed for the mining only. Not expect that somebody will call it.
    // In case it is needed, Update create server, add configs fir for stratum
    mwc_api::ApiResponse<bool> start_stratum(int context_id);

    // Start p2p discovery job
    mwc_api::ApiResponse<bool> start_discover_peers(int context_id);

    // Start node sync job
    mwc_api::ApiResponse<bool> start_sync_monitoring(int context_id);

    // Start peers listening job
    mwc_api::ApiResponse<bool> start_listen_peers(int context_id);

    // Start dandelion job
    mwc_api::ApiResponse<bool> start_dandelion(int context_id);

    // Start dandelion job
    mwc_api::ApiResponse<ServerStats> get_server_stats(int context_id);

    // Init feature for process_api_call calls
    mwc_api::ApiResponse<bool> init_call_api(int context_id);

    // process Foreign API directly.
    // Return - foreign API response as it is. Normally it is a json string
    mwc_api::ApiResponse<QString>  process_api_call(int context_id, const QString & method, const QString & uri, const QString & body);

}

#endif //MWC_QT_WALLET_MWCNODEAPI_H
