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

#include "TaskAccount.h"
#include <QDebug>
#include "../mwc713.h"
#include <QtAlgorithms>
#include "../../core/Notification.h"
#include "TaskTransaction.h"

namespace wallet {

// ------------------------------- TaskAccountList ------------------------------------------

static QString widenDerivPath(QString path ) {
    QStringList lst = path.split("/");
    QString  res;

    for ( auto s : lst ) {
        while(s.length()<3)
            s.insert(0,'0');
        res+=s;
    }

    while(res.length()<10)
        res.insert(0, '0');

    return res;
}

bool TaskAccountList::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );

    int idx = 0;

    for ( ;idx<lns.size(); idx++ ) {
        if ( lns[idx].message.contains("____ Wallet Accounts ____")  )
            break;
    }
    idx++;

    for ( ;idx<lns.size(); idx++ ) {
        if ( lns[idx].message.contains("Parent BIP-32 Derivation Path")  )
            break;
    }
    idx++;

    for ( ;idx<lns.size(); idx++ ) {
        if ( lns[idx].message.contains("-------------------------------")  )
            break;
    }
    idx++;

    if (idx>=lns.size()) {
        notify::appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL, "Unable to get a list of accounts from mwc713" );
        return true; // No data to process.
    }

    // Collecting account
    QVector< QPair<QString, QString> > accounts;

    const char * sep = " | m/";
    for ( ;idx<lns.size(); idx++ ) {
        const QString & ln = lns[idx].message;
        int i = ln.lastIndexOf(sep);

        if (i>0) {
            QString acc = ln.left(i).trimmed();
            QString path = ln.mid( i + (int)strlen(sep) );

            accounts.push_back( QPair<QString, QString>( widenDerivPath(path), acc ) );
        }
    }

    std::sort(accounts.begin(), accounts.end());

    QVector<QString> res;
    for (auto ap : accounts)
        res.push_back(ap.second);

    wallet713->updateAccountList( res );
    return true;
}

// ----------------------- TaskAccountCreate -------------------------------

bool TaskAccountCreate::processTask(const QVector<WEvent> &events) {
    if ( events.size()==1 && events[0].event == WALLET_EVENTS::S_READY )
        wallet713->createNewAccount( newAccountName );
    else
        wallet713->createNewAccount( "" );

    return true;
}

// ---------------------------- TaskAccountSwitch -------------------------

bool TaskAccountSwitch::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > errors = filterEvents( events, WALLET_EVENTS::S_GENERIC_ERROR );
    for ( const auto & e : errors) {
        notify::appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL, "Unable to switch account. " + e.message );
    }
    return true;
}

// ------------------------- TaskAccountRename --------------------------

bool TaskAccountRename::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > lns = filterEvents( events, WALLET_EVENTS::S_LINE );
    QVector< WEvent > errors = filterEvents( events, WALLET_EVENTS::S_GENERIC_ERROR );

    // Check for success...
    // rename acct from 'second' to 'forth'
    bool ok = false;
    for (auto & ln : lns) {
        if (ln.message.contains("rename acct from") && ln.message.contains("' to '")) {
            ok = true;
            break;
        }
    }

    if (ok) {
        wallet713->updateRenameAccount(oldName, newName, createAccountSimulation,
                                           true, "");
        return true;
    }
    else {
        QString errorMessage;
        if (errors.isEmpty()) {
            errorMessage = "Didn't get expected reply form mwc713";
        }
        else {
            errorMessage = errors[0].message;
        }

        wallet713->updateRenameAccount(oldName, newName, createAccountSimulation, false, errorMessage);
        return true;
    }
}

// ------------------------- TaskAccountInfo --------------------------

static int64_t extractMwc( const QString & str ) {
    int idx = str.lastIndexOf('|');
    if (idx<0)
        return -1;
    return str.mid(idx+1).trimmed().remove('.').toLongLong();
}

bool TaskAccountInfo::processTask( const QVector<WEvent> & events) {
    QVector< WEvent > infoEvts = filterEvents( events, WALLET_EVENTS::S_ACCOUNTS_INFO_SUM );
    QVector< WEvent > warnings = filterEvents( events, WALLET_EVENTS::S_GENERIC_WARNING );
    QVector< WEvent > lns = filterEvents( events, WALLET_EVENTS::S_LINE );

    if ( infoEvts.size()!=1 ) {
        notify::appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL, "Unable to mwc713 'info' about current account" );
        return false;
    }

    QStringList infoData = infoEvts[0].message.split('|');
    bool ok = (infoData.size()==2);
    QString currentAccountName;
    int64_t height = 0;
    if (ok) {
        currentAccountName = infoData[0];
        height = infoData[1].toLongLong(&ok);
    }

    int lnIdx = 0;
    int64_t totalNano = -1;
    for ( ; lnIdx<lns.size(); lnIdx++ ) {
        if ( lns[lnIdx].message.contains("Confirmed Total ") ) {
            totalNano = extractMwc(lns[lnIdx].message);
            break;
        }
    }
    //

    int64_t waitingConfNano = -1;
    for ( ; lnIdx<lns.size(); lnIdx++ ) {
        if ( lns[lnIdx].message.contains("Awaiting Confirmation ") ) {
            waitingConfNano = extractMwc(lns[lnIdx].message);
            break;
        }
    }

    int64_t waitingFinalizetinNano = -1;
    for ( ; lnIdx<lns.size(); lnIdx++ ) {
        if ( lns[lnIdx].message.contains("Awaiting Finalization ") ) {
            waitingFinalizetinNano = extractMwc(lns[lnIdx].message);
            break;
        }
    }

    int64_t lockedNano = -1;
    for ( ; lnIdx<lns.size(); lnIdx++ ) {
        if ( lns[lnIdx].message.contains("Locked by previous transaction") ) {
            lockedNano = extractMwc(lns[lnIdx].message);
            break;
        }
    }

    int64_t spendableNano = -1;
    for ( ; lnIdx<lns.size(); lnIdx++ ) {
        if ( lns[lnIdx].message.contains("Currently Spendable ") ) {
            spendableNano = extractMwc(lns[lnIdx].message);
            break;
        }
    }

    if ( !ok || totalNano<0 || waitingConfNano<0 || lockedNano<0 || spendableNano<0 ) {
        notify::appendNotificationMessage( notify::MESSAGE_LEVEL::CRITICAL, "Unable to parse mwc713 'info' output" );
        return false;
    }

    // Check if need to update the HODL outputs. Do that is balance was changed
    AccountInfo acc;
    QVector<AccountInfo> accounts = wallet713->getWalletBalance();
    for (const AccountInfo & ai : accounts) {
        if (currentAccountName == ai.accountName) {
            acc = ai;
            break;
        }
    }
    bool noChange = ( totalNano + waitingFinalizetinNano == acc.total &&
            waitingConfNano + waitingFinalizetinNano == acc.awaitingConfirmation &&
            lockedNano == acc.lockedByPrevTransaction &&
            spendableNano == acc.currentlySpendable);

    auto timeout = TaskOutputsForAccount::TIMEOUT;

    if (!noChange)
        wallet713->getEventCollector()->addTask( TASK_PRIORITY::TASK_NOW, { QPair<Mwc713Task*,int64_t>(new TaskOutputsForAccount( wallet713, acc.accountName), timeout)}, 0 );

    wallet713->infoResults( currentAccountName, height,
                            totalNano, waitingConfNano, waitingFinalizetinNano, lockedNano, spendableNano,
                            warnings.size()>0 );

    return true;
}

// ------------------------- TaskAccountProgress --------------------------

bool TaskAccountProgress::processTask(const QVector<WEvent> &events) {
    Q_UNUSED(events);
    wallet713->updateAccountProgress( pos, total );
    return true;
}

// ---------------------- TaskAccountListFinal -------------------------
bool TaskAccountListFinal::processTask(const QVector<WEvent> &events) {
    Q_UNUSED(events);
    wallet713->updateAccountFinalize();
    return true;
}


}

