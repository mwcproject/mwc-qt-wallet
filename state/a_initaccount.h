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

#ifndef INITACCOUNT_H
#define INITACCOUNT_H

#include "state.h"
#include <QString>
#include <QObject>
#include <QVector>
#include "../core/testseedtask.h"

namespace wnd {
class NewSeed;
class ProgressWnd;
class EnterSeed;
class NewSeedTest;
}

namespace state {

const QString INIT_ACCOUNT_CALLER_ID = "InitAccount";

class InitAccount : public QObject, public State
{
    Q_OBJECT
public:
    InitAccount(StateContext * context);
    virtual ~InitAccount() override;

    // Step 1
    void setPassword(const QString & password);
    // Step 2
    enum MWC_NETWORK { MWC_MAIN_NET = 1, MWC_FLOO_NET=2 };
    void submitWalletCreateChoices(MWC_NETWORK network, QString instanceName);

    // Step 3  New seed
    void doneWithNewSeed();

    // Single Word verification
    void submitSeedWord(QString word);

    // Restart seed verification
    void restartSeedVerification();

    // create form seed
    void createWalletWithSeed( QVector<QString> sd );
private slots:
    void onNewSeed(QVector<QString> seed);

    void onLoginResult(bool ok);

    void onRecoverProgress( int progress, int maxVal );

    void onRecoverResult(bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages);

protected:
    virtual NextStateRespond execute() override;
    virtual void exitingState() override;

    // return true if done
    bool finishSeedVerification();
private:
    int progressMaxVal = 10;

    QString pass;
    QVector<QString> seed;
    QVector<core::TestSeedTask> tasks;
    int  seedTestWrongAnswers = 0;
};


}


#endif // INITACCOUNT_H
