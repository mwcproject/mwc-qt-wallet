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

void AppContext::setPassHash(const QString & pass) {
    if (pass.length()==0) {
        passHash = -1;
        return;
    }

    passHash = qHash(pass) % 0xFFFF;
}

bool AppContext::checkPassHash(const QString & pass) const {
    return passHash == qHash(pass) % 0xFFFF;
}

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


bool AppContext::loadData() {
    QString dataPath = ioutils::initAppDataPath("context");

    QFile file(dataPath + "/" + settingsFileName);
    if ( !file.open(QIODevice::ReadOnly) ) {
        // first run, no file exist
        return false;
     }

     QDataStream in(&file);
     in.setVersion(QDataStream::Qt_5_12);

     int id = 0;
     in >> id;
     if (id<0x6546 || id>0x6549)
         return false;

     in >> passHash;
     in >> network;

     if (id>=0x6547) {
         int st;
         in >> st;
         st = activeWndState;
     }

     if (id>=0x6548) {
         sendCoinsParams.loadData(in);
     }

     if (id>=0x6549) {
         in >> pathStates;
     }

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
    out.setVersion(QDataStream::Qt_5_12);

    out << 0x6549;
    out << passHash;
    out << network;
    out << int(activeWndState);
    out << pathStates;

    sendCoinsParams.saveData(out);
}



}
