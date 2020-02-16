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
#include "../windows/a_initaccount_w.h"
#include "../core/windowmanager.h"
#include "../core/appcontext.h"
#include "../state/statemachine.h"
#include "../windows/c_newwallet_w.h"
#include "../windows/c_newseed_w.h"
#include "../windows/c_newseedtest_w.h"
#include "../windows/c_enterseed.h"
#include "../windows/z_progresswnd.h"
#include "../util/Log.h"
#include "../control/messagebox.h"
#include "x_walletconfig.h"
#include "../util/ioutils.h"
#include "../util/Files.h"
#include "../util/Process.h"
#include "../core/global.h"
#include "../core/Notification.h"
#include "../core/Config.h"

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

    // Need to provision the wallet. If it is not running, mean that we wasn't be able to login.
    // So we have to init the wallet
    if ( !running && context->appContext->pullCookie<QString>("checkWalletInitialized")=="FAILED" ) {
        if (config::isOnlineNode()) {
            // Very first run. Need to create the wallet without any passwords
            // Let's not wait for anyhting
            context->wallet->start2init("");
            context->wallet->confirmNewSeed();
            return NextStateRespond( NextStateRespond::RESULT::DONE );
        }
        else {
            context->wndManager->switchToWindowEx( mwc::PAGE_A_INIT_ACCOUNT,
                        new wnd::InitAccount( context->wndManager->getInWndParent(), this,
                                (state::WalletConfig *) context->stateMachine->getState(STATE::WALLET_CONFIG) ) );

            // Provosion of new wallet, need to block locking
            context->stateMachine->blockLogout();

            return NextStateRespond( NextStateRespond::RESULT::WAIT_FOR_ACTION );
        }
    }

    // Just skip that step
    return NextStateRespond( NextStateRespond::RESULT::DONE );
}

// Executing another state
void InitAccount::exitingState() {
    context->stateMachine->unblockLogout();
}

// Get Password, Choose what to do
void InitAccount::setPassword(const QString & password) {
    pass = password;
    context->wndManager->switchToWindowEx( mwc::PAGE_A_NEW_WALLET,
            new wnd::NewWallet( context->wndManager->getInWndParent(), this ) );
}

// How to provision the wallet
void InitAccount::submitCreateChoice(NEW_WALLET_CHOICE newWalletChoice, MWC_NETWORK network) {
    // Apply network first
    Q_ASSERT( !context->wallet->isRunning() );
    wallet::WalletConfig walletCfg = context->wallet->getWalletConfig();
    QString nwName = network == MWC_NETWORK::MWC_MAIN_NET ? "Mainnet" : "Floonet";
    walletCfg.setDataPathWithNetwork( walletCfg.getDataPath(), nwName );

    // Store the nw name with architecture at the data folder.
    walletCfg.saveNetwork2DataPath(walletCfg.getDataPath(), nwName, util::getBuildArch() );

    context->wallet->setWalletConfig( walletCfg, context->appContext, context->mwcNode );

    switch (newWalletChoice) {
        case CREATE_NEW:
            // generate a new seed for a new wallet
            context->wallet->start2init(pass);
            break;
        case CREATE_WITH_SEED:
            context->wndManager->switchToWindowEx( mwc::PAGE_A_ENTER_SEED, new wnd::EnterSeed( context->wndManager->getInWndParent(), this ) );
            break;
        default:
            Q_ASSERT(false);
            break;
    }
}

// New see was created....
void InitAccount::onNewSeed(QVector<QString> sd) {
    if (config::isOnlineNode())
        return; // Online seed - just ignore it

    seed = sd;
    tasks = core::generateSeedTasks( seed ); // generate tasks
    seedTestWrongAnswers = 0;

#ifdef QT_DEBUG
    // We really don't want go trough all words
    tasks.resize(1);
#endif

    context->wndManager->switchToWindowEx( mwc::PAGE_A_NEW_WALLET_PASSPHRASE,
        new wnd::NewSeed( context->wndManager->getInWndParent(), this, getContext(),
                    seed ) );
}

void InitAccount::wndDeleted(wnd::NewSeed * )  {}

// New seed was acknoleged...
void InitAccount::submit() {
    // Start next task

    if (finishSeedVerification())
        return;

    Q_ASSERT(tasks.size()>0);
    if (tasks.size()==0)
        return;

    // Show verify dialog
    context->wndManager->switchToWindowEx( mwc::PAGE_A_PASSPHRASE_TEST,
            new wnd::NewSeedTest( context->wndManager->getInWndParent(), this, tasks[0].getWordIndex() ) );
}


// Verify Dialog respond...
void InitAccount::submit(QString word) {
    if (tasks.size()==0) {
        Q_ASSERT(false);
        return;
    }

#ifdef QT_DEBUG
    // Allways treat as correct answer...
    Q_UNUSED(word);
    tasks.remove(0);
#else
    // Release, the normal way
    if (tasks[0].applyInputResults(word)) {
        // ok case
        tasks.remove(0);
        // retry with submit call
    }
    else {
        seedTestWrongAnswers++;
        bool restart = seedTestWrongAnswers>=3;

        control::MessageBox::messageText(nullptr, "Wrong word",
                                     "The word number " + QString::number(tasks[0].getWordIndex()) +
                                     " was typed incorrectly. " +
                                     "Please review your passphrase and we will try again starting " +
                                     (restart ? "from the beginning." : "where we left off.") );

        // regenerate if totally failed
        //  Test on per word basis. Used for random words order.  if (tasks[0].isTestCompletelyFailed()) {
        if (restart) {
            // generate a new tasks for a wallet
            tasks = core::generateSeedTasks(seed);
            seedTestWrongAnswers = 0;
        } else {
         /* In case of random order, put the failed order at the end of the Q.
            // add to the Q
            tasks.push_back( tasks[0] );
            tasks.remove(0);*/

           // normal order - just do nothing. The same word will be requested.
        }

        // switch to 'show seed' window
        context->wndManager->switchToWindowEx( mwc::PAGE_A_NEW_WALLET_PASSPHRASE,
            new wnd::NewSeed( context->wndManager->getInWndParent(), this, getContext(),
                        seed ) );
        return;
    }
#endif
    submit();
}

bool InitAccount::finishSeedVerification() {
    if (tasks.size()==0 && seed.size()>0) {
            // clean up the state
            context->wallet->confirmNewSeed();
            context->wallet->logout(true); // Stop the wallet with inti process first

            // Now need to start the normall wallet...
            context->wallet->start(false);
            context->wallet->loginWithPassword(pass);

            control::MessageBox::messageText(nullptr, "Congratulations!", "Thank you for confirming all words from your passphrase. Your wallet was successfully created");
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

    // switching to a progress Wnd
    progressWnd = (wnd::ProgressWnd*) context->wndManager->switchToWindowEx( mwc::PAGE_A_RECOVERY_FROM_PASSPHRASE,
            new wnd::ProgressWnd(context->wndManager->getInWndParent(), this, "Recovering account from the passphrase", "",
                                                                                                 "", false));

    // Stopping listeners first. Not checking if they are running.
    progressWnd->setMsgPlus("Preparing for recovery...");
    context->wallet->start2recover(seed, pass);

}

void InitAccount::onRecoverProgress( int progress, int maxVal ) {
    if ( progressWnd==nullptr ) // active indicator
        return;

    progressMaxVal = maxVal;
    progressWnd->initProgress(0, maxVal);

    QString msgProgress = "Recovering..." + QString::number(progress * 100 / maxVal) + "%";
    progressWnd->updateProgress(progress, msgProgress);
    progressWnd->setMsgPlus("");
}

void InitAccount::onRecoverResult(bool started, bool finishedWithSuccess, QString newAddress, QStringList errorMessages) {
    Q_UNUSED(newAddress);

    context->wallet->logout(true);

    if ( progressWnd==nullptr ) // active indicator
        return;


    if (finishedWithSuccess && progressWnd)
        progressWnd->updateProgress(progressMaxVal, "Done");

    QString errorMsg;
    if (errorMessages.size()>0) {
        errorMsg += "\nErrors:";
        for (auto & s : errorMessages)
            errorMsg += "\n" + s;
    }

    bool success = false;

    if (!started) {
        control::MessageBox::messageText(nullptr, "Recover failure", "Account recovery failed to start." + errorMsg);
    }
    else if (!finishedWithSuccess) {
        control::MessageBox::messageText(nullptr, "Recover failure", "Account recovery failed to finish." + errorMsg);
    }
    else {
        success = true;
        control::MessageBox::messageText(nullptr, "Success", "Your account was successfully recovered from the passphrase." + errorMsg);
    }

    if (success) {
        // Now need to start the normall wallet...
        context->wallet->start(false);
        context->wallet->loginWithPassword(pass);

        // We are done. Wallet is provisioned and restarted...
        context->stateMachine->executeFrom(STATE::NONE);
        return;
    }
    else {
        // switch back to the seed window
        context->wndManager->switchToWindowEx( mwc::PAGE_A_ENTER_SEED,
                new wnd::EnterSeed( context->wndManager->getInWndParent(), this ) );
        return;
    }
}

void InitAccount::cancel() {
    context->wndManager->switchToWindowEx( mwc::PAGE_A_NEW_WALLET,
            new wnd::NewWallet( context->wndManager->getInWndParent(), this ) );
}




}
