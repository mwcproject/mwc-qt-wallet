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


#include "testRustIntegration.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QDir>
#include <QThread>

#include "../node/MwcNodeApi.h"

namespace test {

int8_t const * log_callback(void * context, int8_t const * log) {
    Q_ASSERT(context == nullptr);
    qDebug() << "Got log message: " << ((const char *) log);
    return nullptr;
}

int8_t const * blockchain_callback(void * context, int8_t const * log) {
    Q_ASSERT(context == nullptr);
    qDebug() << "Got log message: " << ((const char *) log);
    return nullptr;
}

void testMwcNodeApi() {

    auto create_context_res = node::create_context("Mainnet");
    Q_ASSERT(!create_context_res.hasError());
    int context_id = create_context_res.response;

    auto init_callback_logs_res = node::init_callback_logs("Debug", 1000, test::log_callback, nullptr );


    auto start_tor_res = node::start_tor(QDir::homePath() + "/test-mwc-qt-wallet/tor", nullptr);
    Q_ASSERT(!start_tor_res.hasError());

    auto create_server_res = node::create_server(context_id, QDir::homePath() + "/test-mwc-qt-wallet/node", nullptr,
                    test::blockchain_callback, nullptr );
    Q_ASSERT(!create_server_res.hasError());

    auto start_stratum_res = node::start_stratum(context_id);
    Q_ASSERT(!start_tor_res.hasError());

    auto start_discover_peers_res = node::start_discover_peers(context_id);
    Q_ASSERT(!start_discover_peers_res.hasError());

    auto start_sync_monitoring_res = node::start_sync_monitoring(context_id);
    Q_ASSERT(!start_sync_monitoring_res.hasError());

    auto start_listen_peers_res = node::start_listen_peers(context_id);
    Q_ASSERT(!start_listen_peers_res.hasError());

    auto start_dandelion_res = node::start_dandelion(context_id);
    Q_ASSERT(!start_dandelion_res.hasError());

    // Waiting for 5 minutes and monitor the status
    for (int i=0; i<30; i++) {
        QThread::sleep(10);
        auto get_server_stats_res = node::get_server_stats(context_id);
        Q_ASSERT(!get_server_stats_res.hasError());

        qDebug() << "Server status: " << get_server_stats_res.response.toString();
    }

    auto release_server_res = node::release_server(context_id);
    Q_ASSERT(!release_server_res.hasError());

    // Requesting all log messages
    quint64 last_id = 0;
    while (true) {
        auto get_buffered_logs_res = node::get_buffered_logs(last_id, 10);
        Q_ASSERT(!get_buffered_logs_res.hasError());
        if (get_buffered_logs_res.response.isEmpty())
            break;

        for (mwc_api::LogBufferedEntry log : get_buffered_logs_res.response ) {
            qDebug() << "Read buffered log line: " << log.id << " " << log.time_stamp << " " << log.level << "  " << log.log;
        }
        last_id = get_buffered_logs_res.response.last().id;
    }

    auto release_callback_logs_res = node::release_callback_logs();
    Q_ASSERT(!release_callback_logs_res.hasError());

    auto release_context_res = node::release_context(context_id);
    Q_ASSERT(!release_context_res.hasError());
}


}
