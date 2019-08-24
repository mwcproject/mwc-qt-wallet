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
    path += "/mwc-qt-wallet/";
#endif

     QDir d(path);
     QString dataPath = QDir::cleanPath( d.absolutePath() + QDir::separator() + localPath );
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
