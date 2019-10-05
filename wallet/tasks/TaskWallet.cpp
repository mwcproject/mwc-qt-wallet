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

#include "TaskWallet.h"
#include <QDebug>
#include "../mwc713.h"
#include "../../util/Log.h"
#include "../../core/Notification.h"

namespace wallet {

QVector<QString> calcSeedFromEvents(const QVector<WEvent> &events) {
    QVector<WEvent> lines = filterEvents(events, WALLET_EVENTS::S_LINE);

    int idx = 0;

    for (; idx < lines.size(); idx++) {
        if (lines[idx].message.contains("Your recovery phrase is")) { // This line might be started with prompt...
            idx++;
            break;
        }
    }

    QString passPhs;

    // Expected 24 words. 23 spaces plus many letters
    for (; idx < lines.size() && passPhs.size() < 42; idx++) {
        passPhs = lines[idx].message;
    }

    bool found = false;
    for (; idx < lines.size(); idx++) {
        if (lines[idx].message.contains("Please back-up these words in a non-digital format")) {
            found = true;
            break;
        }
    }

    if (!found)
        return QVector<QString>();

    // Get a passphrase, let's parse it
    QStringList phr = passPhs.split(" ");
    qDebug() << "Get a passphrase, it has words: " << phr.size();

    QVector<QString> seed;

    for (QString &s : phr)
        seed.push_back(s);

    return seed;
}


void TaskInit::onStarted() {
   logger::blockLogMwc713out( true );
}

bool TaskInit::processTask(const QVector<WEvent> & events) {

    logger::blockLogMwc713out(false);

    qDebug() << "TaskInit::processTask: " << printEvents(events);

    wallet713->setNewSeed(calcSeedFromEvents(events));
    return true;
}

//////////////////////////////////////////////////
// TaskStop
bool TaskStop::processTask(const QVector<WEvent> &events) {
    Q_UNUSED(events);
    wallet713->processStop(true);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////
//  TaskUnlock

bool TaskUnlock::processTask(const QVector<WEvent> & events) {
    qDebug() << "TaskUnlock::processTask with events: " << printEvents(events);

    QVector< WEvent > error = filterEvents(events, WALLET_EVENTS::S_PASSWORD_ERROR );

    wallet713->setLoginResult( error.empty() );

    if (error.empty())
        notify::appendNotificationMessage( notify::MESSAGE_LEVEL::INFO, "Successfully logged into the wallet");
    return true;
}

// static
QString TaskUnlock::buildWalletRequest(QString password) {
    QString res = "unlock";
    if (password.length() > 0)
        res += " -p " + util::toMwc713input(password);
    return res;
}

/////////////////////////////////////////////////////////////////////////////////
// TaskInitW

bool TaskInitW::processTask(const QVector<WEvent> & events) {
    QVector< WEvent > error = filterEvents(events, WALLET_EVENTS::S_INIT_WANT_ENTER );
    if (error.size()>0)
        return true; // Expected respond

    // Not expected respond, treat it an internal fatal error
    notify::appendNotificationMessage( notify::MESSAGE_LEVEL::FATAL_ERROR, "Internal error. Fail to init mwc713 wallet with a password" );
    return true;
}

//////////////////////////////////////////////
//   TaskInitWpressEnter

bool TaskInitWpressEnter::processTask(const QVector<WEvent> & events) {
    Q_UNUSED(events);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////
//  TaskLogout

bool TaskLogout::processTask(const QVector<WEvent> & events) {
    Q_UNUSED(events);
    wallet713->logout(false); // async call because task is already async and called from input thread
    return true;
}

//////////////////////////////////////////////////////////////////////////////
// TaskGetNextKey

bool TaskGetNextKey::processTask(const QVector<WEvent> & events) {

    QString mwc713output; // put all line in one. Acceptable for our needs
    for (auto & evt : events) {
        if (evt.event == WALLET_EVENTS::S_LINE)
            mwc713output += evt.message;
    }

    const char * identKWStr = "Identifier(";
    const char * pubKeyKWStr = "PublicKey(";
    const char * closeValueKWStr = ")";

    // Searching for  Identifier(0300000000000000000000000600000000), PublicKey(38abad70a72fba1fab4b4d72061f220c0d2b4dafcc8144e778376098575c965f5526b57e1c34624da2dc20dde2312696e7cf8da676e33376aefcc4742ed9cb79)
    int identIdx1 = mwc713output.indexOf(identKWStr);
    int identIdx2 = mwc713output.indexOf(closeValueKWStr, identIdx1);
    int pubKeyIdx1 = mwc713output.indexOf(pubKeyKWStr, identIdx2);
    int pubKeyIdx2 = mwc713output.indexOf(closeValueKWStr, pubKeyIdx1);

    identIdx1 += strlen(identKWStr);
    pubKeyIdx1 += strlen(pubKeyKWStr);

    // not found should trigger sires of failures. Even need to check all indexes, let't check all of them
    if ( identIdx1<0 || identIdx2<0 || pubKeyIdx1<0 || pubKeyIdx2<0 || identIdx1>=identIdx2 || pubKeyIdx1>=pubKeyIdx2 ) {
        wallet713->setGetNextKeyResult( false, "","", "Unable to parse mwc713 response",btcaddress, airDropAccPassword);
    }
    else  {
        wallet713->setGetNextKeyResult( true,
                                 mwc713output.mid( identIdx1, identIdx2 - identIdx1 ), mwc713output.mid( pubKeyIdx1, pubKeyIdx2 - pubKeyIdx1 ),
                                 "", btcaddress, airDropAccPassword );
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// TaskNodeInfo

bool TaskNodeInfo::processTask(const QVector<WEvent> & events) {
    int height = -1;
    int64_t difficulty = -1;
    int connections = -1;

    QString errors;

    int peerHeight = 0; // max from all peers

    for ( const auto evt : events ) {
        if (evt.event != WALLET_EVENTS::S_LINE)
            continue;

        QString ln = evt.message;
        if (ln.contains("Error", Qt::CaseInsensitive)) {
            if (errors.isEmpty())
                errors+="\n";
            errors+=ln;
        }

        if ( ln.startsWith("Height:") ) {
            bool ok = false;
            height = ln.mid( strlen("Height:") ).trimmed().toInt(&ok);
            if (!ok)
                height = -1;
        }

        if ( ln.startsWith("Total_Difficulty:") ) {
            bool ok = false;
            difficulty = ln.mid( strlen("Total_Difficulty:") ).trimmed().toLongLong(&ok);
            if (!ok)
                difficulty = -1;
        }

        if ( ln.startsWith("PeerInfo:") ) {
            // PeerInfo is not Json and we can't parse it. We just need to find number of peers.
            // Counting PeerInfoDisplay substrings for that
            connections = -1;
            int idx0 = 0;
            while (idx0>=0) {
                connections++;
                idx0 = ln.indexOf( "PeerInfoDisplay", idx0+1 );

                int idxH1 = ln.indexOf( "height:", idx0+1 );
                if (idxH1>0) {
                    idxH1 += strlen("height:");
                    int idxH2 = ln.indexOf( '}', idxH1 );
                    if (idxH2>0) {
                        bool ok = false;
                        int peerH = ln.mid(idxH1, idxH2 - idxH1 ).trimmed().toInt(&ok);
                        if (ok)
                            peerHeight = std::max(peerHeight, peerH);
                    }
                }

            }
        }
    }

    wallet713->setNodeStatus( height>=0 && difficulty>=0 && connections>=0, errors, height, peerHeight, difficulty, connections );
    return true;
}


}

