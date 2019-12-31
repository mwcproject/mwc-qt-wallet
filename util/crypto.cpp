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

#include "crypto.h"
#include <QString>
#include <QCryptographicHash>

// bunch of crypto related utils
namespace crypto {

    // Hex string -> bytes
    // return empty array in case of error
    QByteArray str2hex(const QString & hexStr) {
        QByteArray result;

        if (hexStr.length()==0)
            return result;

        QString inStr = hexStr;
        if (inStr.length()%2==1)
            inStr = "0" + inStr;

        for (int i=1; i<inStr.length(); i+=2) {
            bool ok = false;
            uint n = inStr.mid(i-1,2).toUInt( &ok, 16 );
            if (!ok)
                return QByteArray();

            result.push_back( char(n) );
        }
        return result;
    }

    // bytes -> hex string
    QString hex2str(const QByteArray & data) {
        QString result;

        int sz = data.length();
        const uint8_t * ui8data = reinterpret_cast<const uint8_t * >(data.constData());

        for ( int i=0; i<sz; i++ ) {
            QString ns = QString::number( ui8data[i], 16 );
            while(ns.length()<2)
                ns = "0"+ns;

            result += ns;
        }

        return result;
    }

    // Appy hash to the data once
    // return data is 256 bits
    QByteArray HSA256(const QByteArray & data) {
        QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);

        hash.addData(data);
        return hash.result();
    }

};
