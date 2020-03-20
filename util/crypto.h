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

#ifndef MWC_QT_WALLET_CRYPTO_H
#define MWC_QT_WALLET_CRYPTO_H

#include <QByteArray>

// bunch of crypto related utils
namespace crypto {

    // Hex string -> bytes
    // return empty array in case of error
    QByteArray str2hex(const QString & hexStr);
    // bytes -> hex string
    QString hex2str(const QByteArray & data);

    // Apply hash to the data once
    // return data is 256 bits
    QByteArray HSA256(const QByteArray & data);

};


#endif //MWC_QT_WALLET_CRYPTO_H
