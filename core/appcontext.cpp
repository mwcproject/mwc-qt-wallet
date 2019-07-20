#include "appcontext.h"
#include "../util/ioutils.h"
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QDir>
#include "../control/messagebox.h"
#include "../core/global.h"

namespace core {

const static QString settingsFileName("context.dat");
const static QString airdropRequestsFileName("requests.dat");


void SendCoinsParams::saveData(QDataStream & out) const {
    out << int(0x348A4);
    out << inputConfirmationNumber;
    out << changeOutputs;
}

bool SendCoinsParams::loadData(QDataStream & in) {
    int id = -1;
    in >> id;
    if ( id!=0x348A4 )
        return false;

    in >> inputConfirmationNumber;
    in >> changeOutputs;
    return true;
}


AppContext::AppContext() {
    loadData();

    // Check if airdrop request need to be cleaned up
    QVector<state::AirdropRequests> airDropData = loadAirdropRequests();

    int64_t cleanTimeLimit = QDateTime::currentMSecsSinceEpoch() - mwc::AIRDROP_TRANS_KEEP_TIME_MS;
    int sz = airDropData.size();
    for (int t=sz-1; t>=0; t--) {
        if (airDropData[t].timestamp < cleanTimeLimit) {
            airDropData.remove(t);
        }
    }

    if (sz != airDropData.size())
        saveAirdropRequests(airDropData);
}

AppContext::~AppContext() {
    saveData();
}

/*void AppContext::setPassHash(const QString & pass) {
    if (pass.length()==0) {
        passHash = -1;
        return;
    }

    passHash = qHash(pass) % 0xFFFF;
}

bool AppContext::checkPassHash(const QString & pass) const {
    return passHash == qHash(pass) % 0xFFFF;
}*/

// Get last path state. Default: Home dir
QString AppContext::getPathFor( QString name ) const {
    if (!pathStates.contains(name))
        return QDir::homePath();

    return pathStates[name];
}

// update path state
void AppContext::updatePathFor( QString name, QString path ) {
    pathStates[name] = path;
}

QVector<int> AppContext::getIntVectorFor( QString name ) const {
    if ( !intVectorStates.contains(name) )
        return QVector<int>();

    return intVectorStates[name];
}

void AppContext::updateIntVectorFor( QString name, const QVector<int> & data ) {
    intVectorStates[name] = data;
}


bool AppContext::loadData() {
    QString dataPath = ioutils::getAppDataPath("context");

    QFile file(dataPath + "/" + settingsFileName);
    if ( !file.open(QIODevice::ReadOnly) ) {
        // first run, no file exist
        return false;
     }

     QDataStream in(&file);
     in.setVersion(QDataStream::Qt_5_7);

     int id = 0;
     in >> id;
     if (id!=0x4782)
         return false;

     in >> recieveAccount;
     in >> currentAccountName;

     int st;
     in >> st;
     activeWndState = (state::STATE)st;

     in >> pathStates;
     in >> intVectorStates;

     sendCoinsParams.loadData(in);

     return true;
}

void AppContext::saveData() const {
    QString dataPath = ioutils::getAppDataPath("context");

    QString filePath = dataPath + "/" + settingsFileName;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        control::MessageBox::message(nullptr,
              "ERROR",
              "Unable to save gui-wallet settings to " + filePath +
              "\nError: " + file.errorString());
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_7);

    out << 0x4782;
    out << recieveAccount;
    out << currentAccountName;
    out << int(activeWndState);
    out << pathStates;
    out << intVectorStates;

    sendCoinsParams.saveData(out);
}

// AirdropRequests will handle differently
void AppContext::saveAirdropRequests( const QVector<state::AirdropRequests> & data ) {
    QString dataPath = ioutils::getAppDataPath("context");

    QString filePath = dataPath + "/" + airdropRequestsFileName;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        control::MessageBox::message(nullptr,
                                     "ERROR",
                                     "Unable to aidrop requests to " + filePath +
                                     "\nError: " + file.errorString());
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_7);

    out << 0x8327d1;
    int sz = data.size();
    out << sz;
    for (auto & d : data ) {
        d.saveData(out);
    }
}

QVector<state::AirdropRequests> AppContext::loadAirdropRequests() const {

    QVector<state::AirdropRequests> res;

    QString dataPath = ioutils::getAppDataPath("context");

    QFile file(dataPath + "/" + airdropRequestsFileName);
    if ( !file.open(QIODevice::ReadOnly) ) {
        // first run, no file exist
        return res;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_7);

    int id = 0;
    in >> id;
    if (id!=0x8327d1)
        return res;

    int sz = 0;
    in >> sz;

    while(sz>0) {
        sz--;
        state::AirdropRequests req;
        if (!req.loadData(in))
            break;
        res.push_back(req);
    }

    return res;
}


}
