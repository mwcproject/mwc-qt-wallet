// Copyright 2020 The MWC Developers
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

#ifndef MWC_QT_WALLET_G_FINALIZE_B_H
#define MWC_QT_WALLET_G_FINALIZE_B_H

#include "QObject"

namespace bridge {

class Finalize : public QObject {
Q_OBJECT
public:
    explicit Finalize(QObject * parent = nullptr) : QObject(parent) {}
    ~Finalize() {};

    // Finalize file slate.
    Q_INVOKABLE void uploadFileTransaction(QString fileName);

    // Finalize slatepack slate.
    Q_INVOKABLE void uploadSlatepackTransaction( QString slatepack, QString slateJson, QString sender );

    // Check if the node healthy enough to do finalization.
    Q_INVOKABLE bool isNodeHealthy();
};

}

#endif //MWC_QT_WALLET_G_FINALIZE_B_H
