#include "TaskAccount.h"
#include <QDebug>
#include "../mwc713.h"
#include <QtAlgorithms>

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

    qDebug() << "TaskAccountList::processTask with events: " << printEvents(events);


    int idx=events.size()-1;
    for ( ; idx>=0; idx-- ) {
        if ( events[idx].event == WALLET_EVENTS::S_ACCOUNTS_TITLE )
            break;
    }

    if (idx<0) {
        wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::CRITICAL, MWC713::MESSAGE_ID::GENERIC, "Unable to get a list of accounts from mwc713" );
        return false; // No data to process.
    }

    // Collecting account

    QVector< QPair<QString, QString> > accounts;


    for ( idx++; idx<events.size(); idx++ ) {
        if ( events[idx].event == WALLET_EVENTS::S_TABLE_LINE2 ) {
            // <account_name>|<XX/XX>
            QStringList lst = events[idx].message.split("|");
            if (lst.length()==2) {
                accounts.push_back( QPair<QString, QString>( widenDerivPath(lst[1].trimmed()), lst[0].trimmed()) );
            }
        }
    }

    qSort(accounts);

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
    if ( events.size()!=1 && events[0].event == WALLET_EVENTS::S_READY )
        wallet713->switchToAccount( switchAccountName );
    else
        wallet713->switchToAccount( "" );

    return true;
}

// ------------------------- TaskAccountRename --------------------------

bool TaskAccountRename::processTask(const QVector<WEvent> &events) {
    QVector< WEvent > lns = filterEvents( events, WALLET_EVENTS::S_LINE );
    QVector< WEvent > errors = filterEvents( events, WALLET_EVENTS::S_ERROR );

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

    return str.mid(idx+1).trimmed().remove('.').toLong();
}

bool TaskAccountInfo::processTask( const QVector<WEvent> & events) {
    QVector< WEvent > infoEvts = filterEvents( events, WALLET_EVENTS::S_ACCOUNTS_INFO_SUM );
    QVector< WEvent > warnings = filterEvents( events, WALLET_EVENTS::S_GENERIC_WARNING );
    QVector< WEvent > lns = filterEvents( events, WALLET_EVENTS::S_LINE );

    if ( infoEvts.size()!=1 ) {
        wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::CRITICAL, MWC713::MESSAGE_ID::GENERIC, "Unable to mwc713 'info' about current account" );
        return false;
    }

    QStringList infoData = infoEvts[0].message.split('|');
    bool ok = (infoData.size()==2);
    QString currentAccountName;
    if (ok)
        currentAccountName = infoData[0];

    int64_t height = 0;
    if (ok) {
        height = infoData[1].toLong(&ok);
    }

    int lnIdx = 0;
    int64_t totalNano = -1;
    for ( ; lnIdx<lns.size(); lnIdx++ ) {
        if ( lns[lnIdx].message.contains("Total ") ) {
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
        wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::CRITICAL, MWC713::MESSAGE_ID::GENERIC, "Unable to parse mwc713 'info' output" );
        return false;
    }

    wallet713->infoResults( currentAccountName, height,
            totalNano, waitingConfNano, lockedNano, spendableNano,
            warnings.size()>0 );
    return true;
}

// ------------------------- TaskAccountProgress --------------------------

bool TaskAccountProgress::processTask(const QVector<WEvent> &events) {
    wallet713->updateAccountProgress( pos, total );
    return true;
}

// ---------------------- TaskAccountListFinal -------------------------
bool TaskAccountListFinal::processTask(const QVector<WEvent> &events) {
    wallet713->updateAccountFinalize(accountName2switch);
    return true;
}


}

