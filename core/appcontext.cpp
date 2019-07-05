#include "appcontext.h"
#include "../util/ioutils.h"
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QDir>
#include "../control/messagebox.h"

namespace core {

const static QString settingsFileName("context.dat");


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



}
