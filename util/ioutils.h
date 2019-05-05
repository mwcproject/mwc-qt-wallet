#ifndef IOUTILS_H
#define IOUTILS_H

#include <QString>

namespace ioutils {

// init the directory to store app data
// return path value
QString initAppDataPath(QString localPath = "")  noexcept(false);

}

#endif // IOUTILS_H
