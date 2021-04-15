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

#include "a_initaccount.h"
#include "../wallet/wallet.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../util/Log.h"
#include "x_walletconfig.h"
#include "../util/ioutils.h"
#include "../util/Files.h"
#include "../util/Process.h"
#include "../core/global.h"
#include "../core/Notification.h"
#include "../core/Config.h"
#include "../core/WndManager.h"
#include "../bridge/BridgeManager.h"
#include "../bridge/wnd/z_progresswnd_b.h"
#include <QDir>

namespace state {

InitAccount::InitAccount(StateContext * context) :
        State(context, STATE::STATE_INIT)
{
    QObject::connect( context->wallet, &wallet::Wallet::onNewSeed, this, &InitAccount::onNewSeed, Qt::QueuedConnection );
    QObject::connect( context->wallet, &wallet::Wallet::onLoginResult, this, &InitAccount::onLoginResult, Qt::QueuedConnection );

    // Creating connections...
    QObject::connect(context->wallet, &wallet::Wallet::onRecoverProgress,
                     this, &InitAccount::onRecoverProgress, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onRecoverResult,
                     this, &InitAccount::onRecoverResult, Qt::QueuedConnection);

}

InitAccount::~InitAccount() {

}

NextStateRespond InitAccount::execute() {
    bool running = context->wallet->isRunning();

    currentPage = InitAccountPage::None;

    // Need to provision the wallet. If it is not running, mean that we wasn't be able to login.
    // So we have to init the wallet
    if ( !running && context->appContext->pullCookie<QString>("checkWalletInitialized")=="FAILED" ) {
        if (config::isOnlineNode()) {
            // Very first run. Need to create the wallet without any passwords
            context->wallet->start2init("");
            context->wallet->confirmNewSeed();
            context->wallet->logout(true); // Stop will wait enough time
            return NextStateRespond( NextStateRespond::RESULT::DONE );
        }
        else {
            showInitAccountPage();

            return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
        }
    }

    // Just skip that step
    return NextStateRespond( NextStateRespond::RESULT::DONE );
}

void InitAccount::showInitAccountPage() {
    QString walletPath = context->appContext->getCurrentWalletInstance(false);

    QPair<bool,QString> res = ioutils::getAppDataPath("");

    if (res.first) {
        QDir baseDir(res.second);
        walletPath = baseDir.absoluteFilePath(walletPath);
        QDir wpath(walletPath);
        walletPath = wpath.absolutePath();
    }

    core::getWndManager()->pageInitAccount(walletPath, context->appContext->getCookie<bool>("restoreWalletFromSeed") );
    currentPage = InitAccountPage::PageInitAccount;

    // Provision of new wallet, need to block locking
    context->stateMachine->blockLogout("InitAccount");
}

// Restore form the seed is cancelled by user.
void InitAccount::cancel() {
    context->stateMachine->setActionWindow( state::STATE::START_WALLET, true );
}

// Executing another state
void InitAccount::exitingState() {
    context->stateMachine->unblockLogout("InitAccount");
}

bool InitAccount::mobileBack() {
    switch (currentPage) {
        case InitAccountPage::None: return false;
        case InitAccountPage::PageInitAccount: return false;
        case InitAccountPage::PageEnterSeed: {
            //showInitAccountPage(); // mobile doesn't have this page
            //return true;
            return false;
        }
        case InitAccountPage::PageNewSeed: {
            //showInitAccountPage(); // mobile doesn't have this page
            //return true;
            return false;
        }
        case InitAccountPage::PageNewSeedTest: {
            // Regenerating everything as for failure and go back to passphrase
            generateWordTasks();
            core::getWndManager()->pageNewSeed(mwc::PAGE_A_NEW_WALLET_PASSPHRASE, seed);
            currentPage = InitAccountPage::PageNewSeed;
            return true;
        }
        case InitAccountPage::PageProgressWnd: return true; // Do nothing, not exiting. We are syncing.
    }
    Q_ASSERT(false);
    return false;
}


// Get Password, Choose what to do
void InitAccount::setPassword(const QString & password ) {
    pass = password;
}

// How to provision the wallet
void InitAccount::submitWalletCreateChoices( MWC_NETWORK network, QString instanceName) {
    // Apply network first
    Q_ASSERT( !context->wallet->isRunning() );
    QString path = context->appContext->getCurrentWalletInstance(false);

    wallet::WalletConfig walletCfg = context->wallet->getWalletConfig();
    QString nwName = network == MWC_NETWORK::MWC_MAIN_NET ? "Mainnet" : "Floonet";
    walletCfg.updateNetwork(nwName);
    walletCfg.updateDataPath(path);

    // Store the nw name with architecture at the data folder.
    walletCfg.saveNetwork2DataPath(walletCfg.getDataPath(), nwName, util::getBuildArch(), instanceName );

    context->wallet->setWalletConfig(walletCfg, false );

    if (context->appContext->getCookie<bool>("restoreWalletFromSeed")) {
        // Enter seed to restore
        core::getWndManager()->pageEnterSeed();
        currentPage = InitAccountPage::PageEnterSeed;
    }
    else {
        // generate a new seed for a new wallet
        context->wallet->start2init(pass);
    }
}

// New see was created....
void InitAccount::onNewSeed(QVector<QString> sd) {
    if (config::isOnlineNode())
        return; // Online seed - just ignore it

    seed = sd;

    generateWordTasks();

    core::getWndManager()->pageNewSeed( mwc::PAGE_A_NEW_WALLET_PASSPHRASE, seed );
    currentPage = InitAccountPage::PageNewSeed;
}

void InitAccount::generateWordTasks() {
#ifdef WALLET_DESKTOP
    tasks = core::generateSeedTasks( seed,0, seed.length() ); // generate tasks
#endif
#ifdef WALLET_MOBILE
    tasks = core::generateSeedTasks( seed, 0, std::min(6, seed.length()) ); // generate tasks
#endif
    Q_ASSERT(tasks.size()>0);

    seedTestWrongAnswers = 0;
}


// New seed was acknoleged...
void InitAccount::doneWithNewSeed() {
    // Start next task
    if (finishSeedVerification())
        return;

    Q_ASSERT(tasks.size()>0);
    if (tasks.size()==0)
        return;

    // Show verify dialog
    core::getWndManager()->pageNewSeedTest( tasks[0].getWordIndex() );
    currentPage = InitAccountPage::PageNewSeedTest;
}


// Verify Dialog respond...
void InitAccount::submitSeedWord(QString word) {
    if (tasks.size()==0) {
        Q_ASSERT(false);
        return;
    }

/*#ifdef QT_DEBUG
    // Allways treat as correct answer and don't ask more...
    Q_UNUSED(word);
//    tasks.remove(0);
    tasks.clear();
#else*/
    // Release, the normal way
    if (tasks[0].applyInputResults(word)) {
        // ok case
        tasks.remove(0);
        // retry with submit call
    }
    else {
        seedTestWrongAnswers++;
        bool restart = seedTestWrongAnswers>=3;

        core::getWndManager()->messageTextDlg("Wrong word",
                                     "The word number " + QString::number(tasks[0].getWordIndex()) +
                                     " was typed incorrectly. " +
                                     "Please review your passphrase and we will try again starting " +
                                     (restart ? "from the beginning." : "where we left off.") );

        // regenerate if totally failed
        //  Test on per word basis. Used for random words order.  if (tasks[0].isTestCompletelyFailed()) {
        if (restart) {
            // generate a new tasks for a wallet
            generateWordTasks();
        } else {
         /* In case of random order, put the failed order at the end of the Q.
            // add to the Q
            tasks.push_back( tasks[0] );
            tasks.remove(0);*/

           // normal order - for desktop do nothing.
           // For mobile wallet we need to change the tasks
#ifdef WALLET_MOBILE
            tasks = core::generateSeedTasks( seed, tasks[0].getWordIndex(), tasks.length()); // generate tasks
#endif
        }

        // switch to 'show seed' window
        core::getWndManager()->pageNewSeed(mwc::PAGE_A_NEW_WALLET_PASSPHRASE, seed);
        currentPage = InitAccountPage::PageNewSeed;
        return;
    }
//#endif
    doneWithNewSeed();
}

// Restart seed verification
void InitAccount::restartSeedVerification() {
    // generate a new tasks for a wallet
    generateWordTasks();

    // switch to 'show seed' window
    core::getWndManager()->pageNewSeed(mwc::PAGE_A_NEW_WALLET_PASSPHRASE, seed);
    currentPage = InitAccountPage::PageNewSeed;
}

bool InitAccount::finishSeedVerification() {
    if (tasks.size()==0 && seed.size()>0) {
        // clean up the state
        context->wallet->confirmNewSeed();
        context->wallet->logout(true); // Stop the wallet with inti process first

        // Now need to start the normall wallet...
        context->wallet->start();
        context->wallet->loginWithPassword(pass);

        core::getWndManager()->messageTextDlg("Congratulations!",
                 "Thank you for confirming all words from your passphrase. Your wallet was successfully created");
        // onLoginResult - will be the next step
        return true;
    }
    return false;
}

void InitAccount::onLoginResult(bool ok) {
    if ( seed.isEmpty() )
        return;

    seed.clear();

    if ( !ok  ) {
        notify::reportFatalError("Unfortunately we unable to login into mwc713 wallet after provisioning. Internal error.");
        return;
    }

    // switch to the next state
    context->stateMachine->executeFrom(STATE::NONE);
}


///////////////////////////////////////////////////////////////////////////////////////
///// Create from seed

// Second Step, switching to the progress and starting this process at mwc713
void InitAccount::createWalletWithSeed( QVector<QString> sd ) {
    seed = sd;

    if (!config::isColdWallet()) {
        // Check if the node is cloud one. Issue than wallet nned to recover the data and it is problem if wallet will failed..
        const wallet::WalletConfig & config = context->wallet->getWalletConfig();
        wallet::MwcNodeConnection nodeConnection = context->appContext->getNodeConnection(config.getNetwork());

        if (!nodeConnection.isCloudNode()) {
            if ( core::WndManager::RETURN_CODE::BTN2 !=
                    core::getWndManager()->questionTextDlg("Node connection",
                             "Because restore process requires connection to the running node, we are switching your wallet to the Cloud MWC Node.\n\n"
                                                   "If you prefer different setting, please update your MWC Node connection after",
                             "Cancel", "Continue",
                             "Cancel restore and don't change MWC Node connection",
                             "Change MWC Node connection and continue with wallet restore",
                             false, true))
                return;

            nodeConnection.setAsCloud();
            context->appContext->updateMwcNodeConnection( config.getNetwork(), nodeConnection );
            context->wallet->setWalletConfig( config, false );
        }
    }

    // switching to a progress Wnd
    core::getWndManager()->pageProgressWnd(mwc::PAGE_A_RECOVERY_FROM_PASSPHRASE, INIT_ACCOUNT_CALLER_ID,
              "Recovering account from the passphrase", "", "", false);
    currentPage = InitAccountPage::PageProgressWnd;

    // Stopping listeners first. Not checking if they are running.
    for (auto p :  bridge::getBridgeManager()->getProgressWnd())
        p->setMsgPlus(INIT_ACCOUNT_CALLER_ID, "Preparing for recovery...");

    context->wallet->start2recover(seed, pass);
}

void InitAccount::onRecoverProgress( int progress, int maxVal ) {
    progressMaxVal = maxVal;

    QString msgProgress = "Recovering..." + QString::number(progress * 100 / maxVal) + "%";
    for (auto p :  bridge::getBridgeManager()->getProgressWnd()) {
        p->initProgress(INIT_ACCOUNT_CALLER_ID, 0, maxVal);
        p->updateProgress(INIT_ACCOUNT_CALLER_ID, progress, msgProgress);
        p->setMsgPlus(INIT_ACCOUNT_CALLER_ID, "");
    }
}

void InitAccount::onRecoverResult(bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages) {
    Q_UNUSED(newAddress);

    context->wallet->logout(true);

    if (finishedWithSuccess) {
        for (auto p :  bridge::getBridgeManager()->getProgressWnd())
            p->updateProgress(INIT_ACCOUNT_CALLER_ID, progressMaxVal, "Done");
    }

    QString errorMsg;
    if (errorMessages.size()>0) {
        errorMsg += "\nErrors:";
        for (auto & s : errorMessages)
            errorMsg += "\n" + s;
    }

    bool success = false;

    if (!started) {
        core::getWndManager()->messageTextDlg("Recover failure", "Account recovery failed to start." + errorMsg);
    }
    else if (!finishedWithSuccess) {
        core::getWndManager()->messageTextDlg("Recover failure", "Account recovery failed to finish." + errorMsg);
    }
    else {
        success = true;
        core::getWndManager()->messageTextDlg("Success", "Your account was successfully recovered from the passphrase." + errorMsg);
    }

    if (success) {
        // Now need to start the normall wallet...
        context->wallet->start();
        context->wallet->loginWithPassword(pass);

        // We are done. Wallet is provisioned and restarted...
        context->stateMachine->executeFrom(STATE::NONE);
        return;
    }
    else {
        // switch back to the seed window
        core::getWndManager()->pageEnterSeed();
        currentPage = InitAccountPage::PageEnterSeed;
        return;
    }
}




}
