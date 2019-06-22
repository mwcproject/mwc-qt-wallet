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
    if ( events.size()!=1 && events[0].event == WALLET_EVENTS::S_READY ) {
        wallet713->createNewAccount( newAccountName );
        return true;
    }
    return false;
}

// ---------------------------- TaskAccountSwitch -------------------------

bool TaskAccountSwitch::processTask(const QVector<WEvent> &events) {
    if ( events.size()!=1 && events[0].event == WALLET_EVENTS::S_READY ) {
        wallet713->switchToAccount( switchAccountName );
        return true;
    }
    return false;
}

// ------------------------- TaskAccountInfo --------------------------
bool TaskAccountInfo::processTask( const QVector<WEvent> & events) {
    QVector< WEvent > infoEvts = filterEvents( events, WALLET_EVENTS::S_ACCOUNTS_INFO_SUM );
    QVector< WEvent > warnings = filterEvents( events, WALLET_EVENTS::S_GENERIC_WARNING );

    if ( infoEvts.size()!=1 ) {
        wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::CRITICAL, MWC713::MESSAGE_ID::GENERIC, "Unable to mwc713 'info' about current account" );
        return false;
    }

    QStringList infoData = infoEvts[0].message.split('|');
    bool ok = (infoData.size()==6);
    QString currentAccountName;
    if (ok)
        currentAccountName = infoData[0];

    long height = 0;
    if (ok) {
        height = infoData[1].toLong(&ok);
    }

    long totalNano = 0;
    if (ok) {
        totalNano = infoData[2].remove('.').toLong(&ok);
    }

    long waitingConfNano = 0;
    if (ok) {
        waitingConfNano = infoData[3].remove('.').toLong(&ok);
    }

    long lockedNano = 0;
    if (ok) {
        lockedNano = infoData[4].remove('.').toLong(&ok);
    }

    long spendableNano = 0;
    if (ok) {
        spendableNano = infoData[5].remove('.').toLong(&ok);
    }

    if (!ok) {
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

