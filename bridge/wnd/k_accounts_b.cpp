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

#include "k_accounts_b.h"
#include "../BridgeManager.h"
#include "../../state/state.h"
#include "../../wallet/wallet.h"
#include "../../state/k_accounts.h"
#include "../../state/x_walletconfig.h"
#include "../../core/global.h"
#include "../../util/Log.h"
#include <QMap>

namespace bridge {

static state::Accounts * getState() { return (state::Accounts *) state::getState(state::STATE::ACCOUNTS); }
static state::WalletConfig * getConfig() { return (state::WalletConfig *) state::getState(state::STATE::WALLET_CONFIG); }

static core::AppContext * getAppContext() {
    return state::getStateContext()->appContext;
}

Accounts::Accounts(QObject * parent) : QObject(parent) {
    bridge::getBridgeManager()->addAccounts(this);
}

Accounts::~Accounts() {
    bridge::getBridgeManager()->removeAccounts(this);
}

// Return data ready to insert into the Accounts table
// [accountName, Spendable, Awaiting, Locked, Total
QVector<QString> Accounts::getAccountsBalancesToShow() {
    logger::logInfo(logger::BRIDGE, "Call Accounts::getAccountsBalancesToShow");
    wallet::Wallet * wallet = state::getStateContext()->wallet;

    QVector<wallet::AccountInfo> accounts = wallet->getWalletBalance(
        getConfig()->getSendCoinsParams().inputConfirmationNumber, true, getAppContext()->getLockedOutputs() );
    QVector<QString> result;

    for (auto & acc : accounts) {
        QVector<QString> data{ acc.accountName, util::nano2one(acc.currentlySpendable), util::nano2one(acc.awaitingConfirmation),
                               util::nano2one(acc.lockedByPrevTransaction), util::nano2one(acc.total), acc.accountPath };
        result.append(data);
    }
    return result;
}

// Validate new account name. Return Empty string for ok.
// Otherwise return error message
QString Accounts::validateNewAccountName(QString accountName) {
    logger::logInfo(logger::BRIDGE, "Call Accounts::validateNewAccountName with accountName=" + accountName);
    if (accountName.startsWith("-"))
        return "You can't start account name from '-' symbol.";

    for ( auto & pref : mwc::BANNED_ACCOUT_PREFIXES ) {
        if (accountName.startsWith(pref))
            return "Please specify account name without prefix '" + pref + "'";
    }

    QPair <bool, QString> valRes = util::validateMwc713Str( accountName );
    if (!valRes.first) {
        return valRes.second;
    }

    // Check for account names
    {
        wallet::Wallet * wallet = state::getStateContext()->wallet;
        QVector<wallet::Account> accounts = wallet->listAccounts();

        for (auto & acc : accounts) {
            if (acc.label == accountName) {
                return "Account with name '" + accountName + "' already exists. Please specify a unique account name to create.";
            }
        }
    }
    return "";
}

// Check if can delete the account
bool Accounts::canDeleteAccount(QString accountName) {
    logger::logInfo(logger::BRIDGE, "Call Accounts::canDeleteAccount with accountName=" + accountName);
    wallet::Wallet * wallet = state::getStateContext()->wallet;
    QVector<wallet::AccountInfo> accounts = wallet->getWalletBalance(
        getConfig()->getSendCoinsParams().inputConfirmationNumber, true, {});

    for (auto & acc : accounts) {
        if (acc.accountName == accountName)
            return acc.canDelete();
    }

    return true;
}

// Switch to transfer funds page
void Accounts::doTransferFunds() {
    logger::logInfo(logger::BRIDGE, "Call Accounts::doTransferFunds");
    getState()->doTransferFunds();
}

// Delete account (wallet can do rename only with prefix that will be hidden after)
// Check Signal: sgnAccountRenamed(bool success, QString errorMessage);
void Accounts::deleteAccount( QString accountName ) {
    logger::logInfo(logger::BRIDGE, "Call Accounts::deleteAccount with accountName=" + accountName);
    getState()->deleteAccount(accountName);
}



}
