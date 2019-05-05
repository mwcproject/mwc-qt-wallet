#include "ioutils.h"
#include <QStandardPaths>
#include "../core/mwcexception.h"
#include <QDir>

namespace ioutils {

// init the directory to store app data
QString initAppDataPath(QString localPath )
{
    auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (path.isEmpty())
        throw core::MwcException("Qt error. Cannot determine app data location");

     QDir d(path);
     QString dataPath = d.absolutePath() + "/" + localPath;
     if ( !d.mkpath(dataPath) )
         throw core::MwcException("Unable create app data directory: " + d.absolutePath());

     return dataPath;
}

}
