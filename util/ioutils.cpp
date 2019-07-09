#include "ioutils.h"
#include <QStandardPaths>
#include "../core/mwcexception.h"
#include <QDir>

namespace ioutils {

// init the directory to store app data
QString getAppDataPath(QString localPath )
{
#ifdef Q_OS_WIN
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (path.isEmpty())
        throw core::MwcException("Qt error. Cannot determine app data location");
#else
    QString path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if (path.isEmpty())
        throw core::MwcException("Qt error. Cannot determine home dir location.");
    path += "/mwc-gui-wallet/";
#endif

     QDir d(path);
     QString dataPath = d.absolutePath() + "/" + localPath;
     if ( !d.mkpath(dataPath) )
         throw core::MwcException("Unable create app data directory: " + d.absolutePath());

     return dataPath;
}

QByteArray FilterEscSymbols( const QByteArray & data ) {
    QByteArray res;

    bool inEsc = false;
    for (char d : data) {
        if (d==27)
            inEsc = true;

        if (!inEsc)
            res.append( d );

        if (d=='m')
            inEsc=false;
    }

    return res;
}


}
