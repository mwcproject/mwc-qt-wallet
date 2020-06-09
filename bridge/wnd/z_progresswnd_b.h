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

#ifndef MWC_QT_WALLET_PROGRESSWND_H
#define MWC_QT_WALLET_PROGRESSWND_H

#include <QObject>
#include <QString>

namespace bridge {

class ProgressWnd : public QObject {
    Q_OBJECT
public:
    explicit ProgressWnd(QObject * parent = nullptr);
    ~ProgressWnd();

    Q_PROPERTY(QString callerId READ getCallerId WRITE setCallerId)
    QString getCallerId() { return callerId; }
    void setCallerId(const QString & id) { callerId = id; }

    void setHeader(QString callerId, QString header);
    void setMsgPlus(QString callerId, QString msgPlus);

    void initProgress(QString callerId, int min, int max);
    void updateProgress(QString callerId, int pos, QString msgProgress);

    // Request progress cancell.
    Q_INVOKABLE void cancelProgress();
signals:
    void sgnSetHeader(QString header);
    void sgnSetMsgPlus(QString msgPlus);

    void sgnInitProgress(int min, int max);
    void sgnUpdateProgress(int pos, QString msgProgress);

private:
    QString callerId;
};

}

#endif //MWC_QT_WALLET_PROGRESSWND_H
