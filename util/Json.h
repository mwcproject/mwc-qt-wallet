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

#ifndef MWC_QT_WALLET_JSON_H
#define MWC_QT_WALLET_JSON_H

#include <QJsonObject>

namespace util {

// Return empty if not able to parse.
QJsonObject jsonFromString( QString str );

// Will return 'null' if not found. So you can't check if value exist and null, or doesn't exist
QJsonValue readValueFromJson(const QJsonObject & jsonObj, QString path);

// Path format: <key>.<key>. ...
QString readStringFromJson(const QJsonObject & jsonObj, QString path, const QString * const defaultValue = nullptr );


// Transaction type as intention - what we want ot do with that.
enum class FileTransactionType { RECEIVE, FINALIZE };

/////// ------------------ File Transaction processing ---------------------
struct FileTransactionInfo {
    QString fileName;
    QString transactionId;
    int64_t amount; // nanos
    int64_t fee;    // nanos
    QString message;
    int     height;
    int     lock_height;

    QString resultingFN; // cookie data

    QPair<bool, QString> parseTransaction( QString fileName, FileTransactionType type );
};


}



#endif //MWC_QT_WALLET_JSON_H
