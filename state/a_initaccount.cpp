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
#include <QJsonArray>

#include "util/message_mapper.h"

namespace state {

InitAccount::InitAccount(StateContext * context) :
        State(context, STATE::STATE_INIT)
{
    // Creating connections...
    QObject::connect(context->wallet, &wallet::Wallet::onScanProgress,
                     this, &InitAccount::onScanProgress, Qt::QueuedConnection);

    QObject::connect(context->wallet, &wallet::Wallet::onScanDone,
                     this, &InitAccount::onScanDone, Qt::QueuedConnection);

}

InitAccount::~InitAccount() {
    executeDeleteSeedPath();
}

NextStateRespond InitAccount::execute() {
    bool running = context->wallet->getStartStatus() != wallet::Wallet::STARTED_MODE::OFFLINE;

    currentPage = InitAccountPage::None;

    // Need to provision the wallet. If it is not running, mean that we wasn't be able to login.
    QString checkWalletInitialized = context->appContext->pullCookie<QString>("checkWalletInitialized");
    // So we have to init the wallet

    if (config::isOnlineNode()) {
        if ( !running) {
            QPair<QString, QString> network_path = context->appContext->getOnlineNodeWalletNetworkAndPath();
            if (!context->initWalletNode(network_path.second, network_path.first)) {
                core::getWndManager()->messageTextDlg("Wallet Init Error",
                "Unable to build temporary wallet with base path " + network_path.first + " and network " + network_path.second );
                mwc::closeApplication();
            }
            if (checkWalletInitialized=="FAILED" ) {
                // Very first run. Need to create the wallet without any passwords
                QPair<QStringList, QString> init_res = context->wallet->start2init("", 24);
                if (!init_res.second.isEmpty()) {
                    core::getWndManager()->messageTextDlg("Wallet Init Error",
                                        util::mapMessage(init_res.second));
                    mwc::closeApplication();
                }
                context->wallet->logout(); // Stop will wait enough time
                if (!context->initWalletNode(network_path.second, network_path.first)) {
                    core::getWndManager()->messageTextDlg("Wallet Init Error",
                    "Unable to build temporary wallet with base path " + network_path.first + " and network " + network_path.second );
                    mwc::closeApplication();
                }
            }
            QString loginError = context->wallet->loginWithPassword("", context->appContext);
            if (!loginError.isEmpty()) {
                core::getWndManager()->messageTextDlg("Internal error",
                    "Unable to open temporary wallet with base path " + network_path.first + " and network " + network_path.second );
                mwc::closeApplication();
            }
        }
    }
    else {
        if ( !running && checkWalletInitialized=="FAILED" ) {
            showInitAccountPage();
            return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
        }
    }

    if ( !running && checkWalletInitialized=="FAILED" ) {
        if (config::isOnlineNode()) {
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
    executeDeleteSeedPath();
    logger::logInfo(logger::STATE, "Call InitAccount::cancel");
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
        case InitAccountPage::PageSeedLength: {
            // Mobile should never be here, it is not supported so far.
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
    logger::logInfo(logger::STATE, "Call InitAccount::setPassword with <password>");
    pass = password;
}

// How to provision the wallet
void InitAccount::submitWalletCreateChoices( MWC_NETWORK network, QString instanceName) {
    logger::logInfo(logger::STATE, "Call InitAccount::submitWalletCreateChoices with network=" + QString::number(network) + " instanceName=" + instanceName);
    // Apply network first
    Q_ASSERT( context->wallet->getStartStatus() == wallet::Wallet::STARTED_MODE::OFFLINE );
    QString path = context->appContext->getCurrentWalletInstance(false);

    wallet::WalletConfig walletCfg;
    QString nwName = network == MWC_NETWORK::MWC_MAIN_NET ? "Mainnet" : "Floonet";
    walletCfg.setData(nwName, path);

    // Store the nw name with architecture at the data folder.
    walletCfg.saveNetwork2DataPath(walletCfg.getDataPath(), nwName, util::getBuildArch(), instanceName );

    bool success = context->isWalletDataValid(path, false);
    Q_ASSERT(success);
    if (!success) {
        logger::logError(logger::QT_WALLET, "Failed to init created wallet");
        core::getWndManager()->messageTextDlg("Error", "Failed to init created wallet");
        return;
    }

    if (context->appContext->getCookie<bool>("restoreWalletFromSeed")) {
        // Enter seed to restore
        core::getWndManager()->pageEnterSeed();
        currentPage = InitAccountPage::PageEnterSeed;
    }
    else {
#ifdef WALLET_MOBILE
        // generate a new seed for a new wallet
        submitSeedLength(24);
#else
        core::getWndManager()->pageSeedLength();
        currentPage = InitAccountPage::PageSeedLength;
#endif
    }

    if (!context->initWalletNode(path)) {
        logger::logError(logger::QT_WALLET, "Unable to init wallet for data location " + path);
    }
}

void InitAccount::submitSeedLength(int words) {
    logger::logInfo(logger::STATE, "Call InitAccount::submitSeedLength with words=" + QString::number(words));
    seedLength = words;
    QPair<QStringList, QString> result = context->wallet->start2init(pass, seedLength);

    deleteSeedPath = context->wallet->getWalletDataPath();

    if (!result.second.isEmpty()) {
        core::getWndManager()->messageTextDlg("Wallet Init Error",
                    util::mapMessage(result.second));
        return;
    }

    seed = result.first;

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
    logger::logInfo(logger::STATE, "Call InitAccount::doneWithNewSeed");
    // Start next task
    if (finishSeedVerification()) {
        deleteSeedPath = "";
        return;
    }

    Q_ASSERT(tasks.size()>0);
    if (tasks.size()==0)
        return;

    // Show verify dialog
    core::getWndManager()->pageNewSeedTest( tasks[0].getWordIndex() );
    currentPage = InitAccountPage::PageNewSeedTest;
}


// Verify Dialog respond...
void InitAccount::submitSeedWord(QString word) {
    logger::logInfo(logger::STATE, "Call InitAccount::submitSeedWord with word=<hidden>");
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
    logger::logInfo(logger::STATE, "Call InitAccount::restartSeedVerification");
    // generate a new tasks for a wallet
    generateWordTasks();

    // switch to 'show seed' window
    core::getWndManager()->pageNewSeed(mwc::PAGE_A_NEW_WALLET_PASSPHRASE, seed);
    currentPage = InitAccountPage::PageNewSeed;
}

bool InitAccount::finishSeedVerification() {
    if (tasks.size()==0 && seed.size()>0) {
        seed.clear();

        wallet::WalletConfig config = context->wallet->getWalletConfig();
        node::NodeClient * nodeCient = context->wallet->getNodeClient();

        // clean up the state
        context->wallet->logout(); // Stop the wallet with init process first

        QString err = context->wallet->init(config.getNetwork(), config.getDataPath(), nodeCient);
        if ( !err.isEmpty() ) {
            notify::reportFatalError("Unfortunately we unable to init the wallet after provisioning. " + util::mapMessage(err));
            return true;
        }

        // Now need to start the normall wallet...
        err = context->wallet->loginWithPassword(pass, context->appContext);
        if ( !err.isEmpty() ) {
            notify::reportFatalError("Unfortunately we unable to login into the wallet after provisioning. " + util::mapMessage(err));
            return true;
        }
        if ( config::isOnlineWallet() ) {
            context->wallet->listeningStart( context->appContext->isFeatureMWCMQS(), context->appContext->isFeatureTor());
        }

        core::getWndManager()->messageTextDlg("Congratulations!",
                 "Thank you for confirming all words from your passphrase. Your wallet was successfully created");

        context->stateMachine->executeFrom(STATE::NONE);
        return true;
    }
    return false;
}


///////////////////////////////////////////////////////////////////////////////////////
///// Create from seed

// Second Step, switching to the progress and starting this process at mwc713
void InitAccount::createWalletWithSeed( QVector<QString> sd ) {
    logger::logInfo(logger::STATE, "Call InitAccount::createWalletWithSeed with seedLength=" + QString::number(sd.size()));
    // Here we just created a seed. But scan is still due
    QString err = context->wallet->start2recover(sd, pass);
    if ( !err.isEmpty() ) {
        core::getWndManager()->messageTextDlg("Wallet Init Error",
                    util::mapMessage(err));
        return;
    }

    wallet::WalletConfig config = context->wallet->getWalletConfig();
    node::NodeClient * nodeCient = context->wallet->getNodeClient();
    context->wallet->logout();

    err = context->wallet->init(config.getNetwork(), config.getDataPath(), nodeCient);
    if ( !err.isEmpty() ) {
        notify::reportFatalError("Unfortunately we unable to init the wallet after restore. " + util::mapMessage(err));
        return;
    }

    err = context->wallet->loginWithPassword(pass, context->appContext);
    if ( !err.isEmpty() ) {
        notify::reportFatalError("Unfortunately we unable to login into the wallet after restore. " + util::mapMessage(err));
        return;
    }

    recoverResponseId = context->wallet->update_wallet_state();

    // switching to a progress Wnd
    core::getWndManager()->pageProgressWnd(mwc::PAGE_A_RECOVERY_FROM_PASSPHRASE, recoverResponseId,
              "Recovering account from the passphrase", "", "", false);
    currentPage = InitAccountPage::PageProgressWnd;

    // Stopping listeners first. Not checking if they are running.
    for (auto p :  bridge::getBridgeManager()->getProgressWnd())
        p->setMsgPlus(recoverResponseId, "Preparing for recovery...");

}

void InitAccount::onScanProgress( QString responseId, QJsonObject statusMessage ) {
    if (responseId!=recoverResponseId)
        return;

    if (statusMessage.contains("Scanning")) {
        QJsonArray vals = statusMessage["Scanning"].toArray();
        QString message = vals[1].toString();
        int percent_progress = vals[2].toInt();

        for (auto p :  bridge::getBridgeManager()->getProgressWnd()) {
            p->initProgress(recoverResponseId, 0, 100);
            p->updateProgress(recoverResponseId, percent_progress, util::mapMessage(message));
            p->setMsgPlus(recoverResponseId, "");
        }
    }
}

void InitAccount::onScanDone( QString responseId, bool fullScan, int height, QString errorMessage ) {
    if (responseId!=recoverResponseId)
        return;

    Q_UNUSED(fullScan);
    Q_UNUSED(height);

    wallet::WalletConfig config = context->wallet->getWalletConfig();
    node::NodeClient * nodeCient = context->wallet->getNodeClient();

    context->wallet->logout();

    for (auto p :  bridge::getBridgeManager()->getProgressWnd())
        p->updateProgress(recoverResponseId, 100, errorMessage.isEmpty() ? "Done" : "Recover failure");

    recoverResponseId = "";

    if (!errorMessage.isEmpty()) {
        core::getWndManager()->messageTextDlg("Recover failure", util::mapMessage(errorMessage) );
        return;
    }
    else {
        core::getWndManager()->messageTextDlg("Success", "Your account was successfully recovered from the passphrase.");
    }

    if (errorMessage.isEmpty()) {
        // Now need to start the normall wallet...
        QString err = context->wallet->init(config.getNetwork(), config.getDataPath(), nodeCient);
        if ( !err.isEmpty() ) {
            notify::reportFatalError("Unfortunately we unable to init the wallet after restore. " + util::mapMessage(err));
            return;
        }

        err = context->wallet->loginWithPassword(pass, context->appContext);
        if (err.isEmpty()) {
            if ( config::isOnlineWallet() ) {
                context->wallet->listeningStart( context->appContext->isFeatureMWCMQS(), context->appContext->isFeatureTor());
            }
            // We are done. Wallet is provisioned and restarted...
            context->stateMachine->executeFrom(STATE::NONE);
            return;
        }
        core::getWndManager()->messageTextDlg("Wallet login error", util::mapMessage(err) );
    }

    // Failure
    // switch back to the seed window
    core::getWndManager()->pageEnterSeed();
    currentPage = InitAccountPage::PageEnterSeed;
}

void InitAccount::executeDeleteSeedPath() {
    if (deleteSeedPath.isEmpty())
        return;
    QDir dir(deleteSeedPath);
    dir.removeRecursively();
    deleteSeedPath = "";
}


}
