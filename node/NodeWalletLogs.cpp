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


#include "NodeWalletLogs.h"
#include <QMutex>
#include <QMutexLocker>

#include "util/Log.h"

namespace rust_logs {

    static bool rustLogsInitialized = false;

     int8_t const * rust_logs(void * context, int8_t const * logLine) {
        Q_UNUSED(context);
        QString logLn((const char *)logLine);
        if (logLn.contains("[scrubbed]") || logLn.contains("removing circuit leg") || logLn.contains("missing previous key") || logLn.contains("Spawning reactor"))
            return nullptr;
        logger::logInfo( logger::LIB_LOGS, logLn.trimmed());
        return nullptr;
    }

    // inti logs if not initialized yet
    void initRustLogs(const QString & logLevel) {
        if (rustLogsInitialized)
            return;

        rustLogsInitialized = true;
        mwc_api::ApiResponse<bool> res = node::init_callback_logs(logLevel, 500,
                            rust_logs, nullptr );
        Q_ASSERT(!res.hasError());
        if (res.hasError()) {
            logger::logInfo(logger::MWC_WALLET, "Unable to initialize the logs. " + res.error);
        }
    }

    QVector<mwc_api::LogBufferedEntry> getLatestLogs() {
        mwc_api::ApiResponse<QVector<mwc_api::LogBufferedEntry>> lines = node::get_buffered_logs(0, 1000);
        Q_ASSERT(!lines.hasError());
        if (lines.hasError()) {
            logger::logInfo(logger::MWC_WALLET, "Unable to retrieve logs. " + lines.error);
            return QVector<mwc_api::LogBufferedEntry>();
        }
        return lines.response;
    }

}

