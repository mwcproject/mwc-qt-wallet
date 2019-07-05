#include "TaskRecover.h"
#include <QDebug>
#include "../mwc713.h"
#include "../../util/Log.h"
#include <QThread>

namespace wallet {


static bool ProcessRecoverTask(const QVector<WEvent> &events,MWC713 *wallet713) {
    qDebug() << "ProcessRecoverTask with events: " << printEvents(events);

    QVector< WEvent > staring = filterEvents(events, WALLET_EVENTS::S_RECOVERY_STARTING );
    QVector< WEvent > done = filterEvents(events, WALLET_EVENTS::S_RECOVERY_DONE );
    QVector< WEvent > mqAddr = filterEvents(events, WALLET_EVENTS::S_YOUR_MWC_ADDRESS );
    QVector< WEvent > error = filterEvents(events, WALLET_EVENTS::S_ERROR );

    QStringList errorMessages;
    for (auto & evt : error) {
        if (!evt.message.isEmpty())
            errorMessages.append(evt.message);
    }

    wallet713->setRecoveryResults( staring.size() > 0, done.size()>0,
                                   mqAddr.size()<1 ? "" : mqAddr[0].message,
                                   errorMessages );
    return true;
}


// ----------------------------------- TaskRecoverProgressListener ---------------------------------

bool TaskRecoverProgressListener::processTask(const QVector<WEvent> &events) {
    // It is listener, one by one processing only
    Q_ASSERT(events.size()==1);

    const WEvent & evt = events[0];
    if (evt.event != S_RECOVERY_PROGRESS)
        return false;

    qDebug() << "TaskRecoverProgressListener::processTask with events: " << printEvents(events);

    QStringList lst = evt.message.split('|');
    Q_ASSERT(lst.size()==2);
    if (lst.size()!=2)
        return false;

    bool ok0 = false;
    int64_t limit = lst[0].toLongLong(&ok0); // Limit
    bool ok1 = false;
    int64_t pos = lst[1].toLongLong(&ok1); // position

    Q_ASSERT(ok0 && ok1);
    if (ok0 && ok1) {
        Q_ASSERT(pos<=limit);
        wallet713->setRecoveryProgress( pos, limit );
        return true;
    }

    return false;
}

// --------------------------- TaskRecoverFull ---------------------------------


bool TaskRecoverFull::processTask(const QVector<WEvent> &events) {
    return ProcessRecoverTask(events, wallet713);
}

QString TaskRecoverFull::calcCommand(QVector<QString> seed, QString password) const {
    Q_ASSERT( seed.size() == 24 );
    Q_ASSERT( password.size()>0 );

    QString cmd = "recover --mnemonic ";
    for (auto & s : seed)
        cmd += " " + s;

    cmd += " -p " + password;
    return cmd;
}

// ----------------------- TaskRecover1Type --------------------------

bool TaskRecover1Type::processTask(const QVector<WEvent> &events) {
    qDebug() << "TaskRecover1Type::processTask with events: " << printEvents(events);

    QVector< WEvent > mnem = filterEvents(events, WALLET_EVENTS::S_RECOVERY_MNEMONIC );

    if (mnem.isEmpty())
        wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::FATAL_ERROR, MWC713::MESSAGE_ID::GENERIC, "Get wrong respond from mwc713 process." );

    return true;
}

// --------------------- TaskRecover2Mnenonic -------------------------

bool TaskRecover2Mnenonic::processTask(const QVector<WEvent> &events) {
    qDebug() << "TaskRecover2Mnenonic::processTask with events: " << printEvents(events);

    QVector< WEvent > password = filterEvents(events, WALLET_EVENTS::S_PASSWORD_EXPECTED );
    if (password.isEmpty())
        wallet713->appendNotificationMessage( MWC713::MESSAGE_LEVEL::FATAL_ERROR, MWC713::MESSAGE_ID::GENERIC, "Get wrong respond from mwc713 process." );

    return true;
}

QString TaskRecover2Mnenonic::calcCommand(QVector<QString> seed) const {
    Q_ASSERT( seed.size() == 24 );

    QString cmd;
    for (auto & s : seed) {
        if (!cmd.isEmpty())
            cmd += " ";
        cmd += s;
    }
    return cmd;
}

// -------------------- TaskRecover3Password ----------------------

bool TaskRecover3Password::processTask(const QVector<WEvent> &events) {
    return ProcessRecoverTask(events, wallet713);
}

// -------------------- TaskRecover3Password ----------------------

void TaskRecoverShowMnenonic::onStarted() {
    logger::blockLogMwc713out( true );

    // Let's sleep some time. Needed to get a chance to stop listeners
    QThread::msleep(3000); // 3 seconds normally is enough to stop all listeners
}

bool TaskRecoverShowMnenonic::processTask(const QVector<WEvent> &events) {
    logger::blockLogMwc713out( false );
    QVector< WEvent > passPhrase = filterEvents(events, WALLET_EVENTS::S_PASS_PHRASE );

    if (passPhrase.size()>0) {

        QStringList phr = passPhrase[0].message.split(" ");
        qDebug() << "Get a passphrase, it has words: " << phr.size();

        QVector<QString> seed;

        for (QString &s : phr)
            seed.push_back(s);

        wallet713->setGettedSeed(seed);
        return true;
    }

    // Check for error
    QVector< WEvent > err = filterEvents(events, WALLET_EVENTS::S_ERROR );
    if (err.size()>0) {
        wallet713->setGettedSeed( QVector<QString>{err[0].message} );
    }
    else {
        wallet713->setGettedSeed( QVector<QString>() );
    }

    return true;
}

// ----------------------- TaskCheck --------------------------

bool TaskCheck::processTask(const QVector<WEvent> &events) {

    QVector< WEvent > lns = filterEvents(events, WALLET_EVENTS::S_LINE );
    bool ok = false;
    for ( auto & l : lns ) {
        if (l.message.contains("check and repair done!"))
            ok = true;
    }

    if (ok) {
        wallet713->setCheckResult(true, "");
        return true;
    }

    QStringList messages;
    QVector< WEvent > errs = filterEvents(events, WALLET_EVENTS::S_ERROR );
    for (auto & e : errs)
        messages.push_back(e.message);

    wallet713->setCheckResult(false, util::formatErrorMessages(messages));

    return true;
}


}

