// Copyright 2035 The MWC Developers
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

#include "TorProcess.h"
#include "MwcNodeApi.h"
#include "util/Log.h"

namespace tor {
    static bool torStarted = false;

    QPair<bool, bool> getTorStatus() {
        mwc_api::ApiResponse<QPair<bool, bool>> status = node::tor_status();
        if (status.hasError()) {
            logger::logInfo( logger::MWC_WALLET, "Unable to get tor status. " + status.error);
        }
        return status.response;
    }

    // return error message. Empty if OK
    QString startEmbeddedTor(const QString & torDataPath) {
        if (torStarted)
            return "";

        mwc_api::ApiResponse<bool> res = node::start_tor(torDataPath, nullptr);
        if (res.hasError()) {
            logger::logInfo(logger::MWC_WALLET, "Unable to start Tor. " + res.error);
            return res.error;
        }

        torStarted = true;
        logger::logInfo(logger::MWC_WALLET, "Tor service is started");

        return "";
    }
}
