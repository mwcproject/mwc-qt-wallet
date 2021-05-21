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

#ifndef MWC_QT_WALLET_FILES_H
#define MWC_QT_WALLET_FILES_H

#include <QStringList>
#include <functional>

namespace util {

// Read all lines from text file
// if openOpenErrorCallback will be called in case of open file error,
QStringList readTextFile(QString fileName, bool trimmed = true, bool cleanEMptyLines = true,  std::function<void()> openErrorCallback = [](){} );

// Write text lines into the file
bool writeTextFile(QString fileName, const QStringList & lines );

#ifdef WALLET_MOBILE

// Clear all files in temp directory
void clearTempDir();

// Return file name at tmp directory
QString genTempFileName(QString extension);

bool copyUriToFile(QString uriFN, QString fsFN);
bool copyFileToUri(QString fsFN, QString uriFN);

#endif

}



#endif //MWC_QT_WALLET_FILES_H
