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

#ifndef MWC_QT_WALLET_FILETRANSACTION_H
#define MWC_QT_WALLET_FILETRANSACTION_H

#include <QObject>

namespace bridge {

class FileTransaction : public QObject {
Q_OBJECT
public:
    explicit FileTransaction(QObject * parent = nullptr);
    ~FileTransaction();

    // Caller Id property must be set right after creation
    // Caller Id is expected to be a name of the state the call that FileTransaction window
    Q_PROPERTY(QString callerId READ getCallerId WRITE setCallerId)
    QString getCallerId() const { return callerId; }
    void setCallerId(const QString id) { callerId = id; }

    void hideProgress();

    Q_INVOKABLE void ftBack();
    // Files transaction page, continue with a file
    Q_INVOKABLE void ftContinue(QString fileName, QString resultTxFileName, bool fluff);
    // Files transaction page, continue with a Slatepack
    Q_INVOKABLE void ftContinueSlatepack(QString slatepack, QString txUuid, QString resultTxFileName, bool fluff);

    Q_INVOKABLE bool needResultTxFileName();

    Q_INVOKABLE QString getResultTxPath();
    Q_INVOKABLE void updateResultTxPath(QString path);

    Q_INVOKABLE bool isNodeHealthy();

signals:
    void sgnHideProgress();

private:
    QString callerId;
};

}

#endif //MWC_QT_WALLET_FILETRANSACTION_H
