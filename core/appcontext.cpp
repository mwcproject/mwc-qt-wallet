#include "appcontext.h"
#include "../util/ioutils.h"
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QMessageBox>
#include <QDir>

namespace core {

const static QString settingsFileName("context.dat");

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
    QString dataPath = ioutils::initAppDataPath("context");

    QFile file(dataPath + "/" + settingsFileName);
    if ( !file.open(QIODevice::ReadOnly) ) {
        // first run, no file exist
        return false;
     }

     QDataStream in(&file);
     in.setVersion(QDataStream::Qt_5_7);

     int id = 0;
     in >> id;
     if (id!=0x4780)
         return false;

     in >> currentAccount;

     int st;
     in >> st;
     activeWndState = (state::STATE)st;

     in >> pathStates;
     in >> intVectorStates;

     sendCoinsParams.loadData(in);

     return true;
}

void AppContext::saveData() const {
    QString dataPath = ioutils::initAppDataPath("context");

    QString filePath = dataPath + "/" + settingsFileName;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::information(nullptr,
              "Unable to save settings",
              "Unable to save gui-wallet settings to " + filePath +
              " Error: " + file.errorString());
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_7);

    out << 0x4780;
    out << currentAccount;
    out << int(activeWndState);
    out << pathStates;
    out << intVectorStates;

    sendCoinsParams.saveData(out);
}



}
