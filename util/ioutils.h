#ifndef IOUTILS_H
#define IOUTILS_H

#include <QString>

namespace ioutils {

// init the directory to store app data
// return path value
QString getAppDataPath(QString localPath = "")  noexcept(false);

QByteArray FilterEscSymbols( const QByteArray & data );

}

#endif // IOUTILS_H
