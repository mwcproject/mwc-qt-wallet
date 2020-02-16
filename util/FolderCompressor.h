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


#ifndef MWC_QT_WALLET_FOLDERCOMPRESSOR_H
#define MWC_QT_WALLET_FOLDERCOMPRESSOR_H

#include <QPair>

namespace compress {

//A recursive function that scans all files inside the source folder
//and serializes all files in a row of file names and compressed
//binary data in a single file
// archiveTag - will be written into archive
// return: <success, Error Message>
QPair<bool, QString> compressFolder(QString sourceFolder, QString destinationFile, const QString & archiveTag, bool callProcessEvents = true);

//A function that deserializes data from the compressed file and
//creates any needed subfolders before saving the file
// archiveTag - expected archive tag. Example: network. This tag will be checked.
// callProcessEvents - if true - will periodically call QCoreApplication::processEvents();
// return: <success, Error Message>
QPair<bool, QString> decompressFolder(QString sourceFile, QString destinationFolder, const QString & archiveTag, bool callProcessEvents = true);


}

#endif //MWC_QT_WALLET_FOLDERCOMPRESSOR_H
