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
#include <QDir>

namespace ioutils {

// init the directory to store app data
// return: <success, data/error>
QPair<bool,QString> getAppDataPath(QString localPath )
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    if (path.isEmpty())
        return QPair<bool,QString>(false, "Qt error. Cannot determine home dir location.");
    path += "/mwc-qt-wallet/";

    // Update the windows we can't just append paths because of the drive.
    // path can be c:/Users/....
    // localPath:  e:/data
    // In this case we can't append them, we have to use local path

    // localPath can be appended only if it doesn't start with drive...
    bool localPathIsRoot = false;
    for ( const QFileInfo & drive : QDir::drives() ) {
        if ( localPath.startsWith( drive.path() ) ) {
            localPathIsRoot = true;
            break;
        }
    }

    QDir d(path);
    QString dataPath = localPathIsRoot ? localPath : (d.absolutePath() + QDir::separator() + localPath);
    dataPath = QDir::cleanPath( dataPath );
    if ( !d.mkpath(dataPath) )
        return QPair<bool,QString>(false, "Unable create app data directory: " + d.absolutePath());

    return QPair<bool,QString>(true,dataPath);
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
