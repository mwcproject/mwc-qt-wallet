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

#ifndef MWC_QT_WALLET_MWCNODECONFIG_H
#define MWC_QT_WALLET_MWCNODECONFIG_H

#include <QString>

namespace node {

struct MwcNodeConfig {
    QString network; // Network full name as it is defined in the config
    QString host;
    QString port;
    QString secret;

    void setData(QString network, QString host, QString port, QString secret);
};

QString getMwcNodePath(const QString & nodeDataPath, const QString & network);

// Update first and then get.
MwcNodeConfig getCurrentMwcNodeConfig(const QString & nodeDataPath, const QString & network );

}


#endif //MWC_QT_WALLET_MWCNODECONFIG_H
