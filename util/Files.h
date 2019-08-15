#ifndef MWC_QT_WALLET_FILES_H
#define MWC_QT_WALLET_FILES_H

#include <QStringList>
#include <functional>

namespace util {

// Read all lines from text file
// if openOpenErrorCallback will be called in case of open file error,
QStringList readTextFile(QString fileName,  std::function<void()> openOpenErrorCallback = [](){} );

// Write text lines into the file
bool writeTextFile(QString fileName, QStringList lines );


}



#endif //MWC_QT_WALLET_FILES_H
