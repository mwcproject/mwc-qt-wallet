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

#include "address.h"
#include <QVector>
#include <QCryptographicHash>
#include "../core/Config.h"

namespace util {

const int MWC_MQ_ADDR_LEN = 52;
const int TOR_ADDR_LEN = 56;

const QString Base58Alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static const int8_t mapBase58[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6,  7, 8,-1,-1,-1,-1,-1,-1,
    -1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,
    22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,
    -1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
    47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
};

// Bitcoin base 58 implementation. We don't skip spaces becuase we did that before.
static bool decodeBase58( QString sz, QByteArray & vch)
{
    int length = 0;

    // Allocate enough space in big-endian base256 representation.
    int size = sz.size() * 733 /1000 + 1; // log(58) / log(256), rounded up.
    std::vector<unsigned char> b256(size);
    // Process the characters.

    std::string s = sz.toStdString();
    const char * psz = s.c_str();

    while (*psz) {
        // Decode base58 character
        int carry = mapBase58[(uint8_t)*psz];
        if (carry == -1)  // Invalid b58 character
            return false;
        int i = 0;
        for (std::vector<unsigned char>::reverse_iterator it = b256.rbegin(); (carry != 0 || i < length) && (it != b256.rend()); ++it, ++i) {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }
        Q_ASSERT(carry == 0);
        length = i;
        psz++;
    }
    // Skip trailing spaces.
    if (*psz != 0)
        return false;
    // Skip leading zeroes in b256.
    std::vector<unsigned char>::iterator it = b256.begin() + (size - length);
    while (it != b256.end() && *it == 0)
        it++;
    // Copy result into output vector.
    while (it != b256.end())
        vch.push_back(*(it++));
    return true;
}

// Check if it is tor address
static QPair<QString, ADDRESS_TYPE> checkTorAddress( QString address ) {
    QString pk = extractPubKeyFromAddress(address);
    if (pk.length() != TOR_ADDR_LEN) {
        return QPair<QString, ADDRESS_TYPE>("Not a TOR address", ADDRESS_TYPE::UNKNOWN);
    }

    // Check optional suffix '.onion'
    int idx = address.lastIndexOf('.');
    if (idx>=0) {
        // Check for bad extention
        if (address.mid(idx).compare(".onion", Qt::CaseInsensitive) != 0)
            return QPair<QString, ADDRESS_TYPE>("Expecting onion suffix for TOR address", ADDRESS_TYPE::UNKNOWN);
    }

    return QPair<QString, ADDRESS_TYPE>("", ADDRESS_TYPE::TOR);
}

// mwc mq address might include the domain
static QPair<QString, ADDRESS_TYPE> checkMwcMqAddress( QString address ) {

    if (address.contains('@') ) {
        address = address.left( address.indexOf('@') );
    }
    else {
        // Checking the prefix
        if (!( address[0] == 'q' || address[0] == 'x' ))
            return QPair<QString, ADDRESS_TYPE>("Not found expected prefix for MQS address", ADDRESS_TYPE::UNKNOWN);
    }

    if ( address.length() != MWC_MQ_ADDR_LEN )
        return QPair<QString, ADDRESS_TYPE>("Incorrect length for MQS address", ADDRESS_TYPE::UNKNOWN);

    // check alphabet
    for ( auto ch : address ) {
        if ( Base58Alphabet.indexOf(ch) < 0 ) // invalid symbol
            return QPair<QString, ADDRESS_TYPE>("MSQ address has invalid symbols", ADDRESS_TYPE::UNKNOWN);
    }

    // Let't validate the checksum
    QByteArray payload;
    if ( !decodeBase58( address, payload ) || payload.size()<5 )
        return QPair<QString, ADDRESS_TYPE>("Unable to decode MQS address", ADDRESS_TYPE::UNKNOWN);

    int checksum_index = payload.size() - 4;
    QByteArray provided_checksum = payload.mid(checksum_index);

    payload = payload.left( payload.size()-4 ); // pure data

    payload = QCryptographicHash::hash(payload, QCryptographicHash::Algorithm::Sha256);
    payload = QCryptographicHash::hash(payload, QCryptographicHash::Algorithm::Sha256);

    for (int u=0;u<4;u++) {
        if ( payload[u] != provided_checksum[u] )
            return QPair<QString, ADDRESS_TYPE>("MQS address has wrong checksum", ADDRESS_TYPE::UNKNOWN);
    }

    return QPair<QString, ADDRESS_TYPE>("", ADDRESS_TYPE::MWC_MQ);
}

// return: protocol, address.
// Empty if failed to parse
static QPair<QString, QString> split2ProtocolAddress(QString address) {
    // Check for protocol name
    int scIdx = address.indexOf(':');

    if (scIdx<=0) {
        // No protocol defned...
        return QPair<QString, QString>( "", address );
    }

    // Must have sintax
    // <protocol>://<address>
    int protSepIdx = address.indexOf("://");
    if ( protSepIdx != scIdx ) {
        // Not found correct separator
        return QPair<QString, QString>("",""); // Incorrect format
    }

    Q_ASSERT(protSepIdx>0);

    return  QPair<QString, QString>( address.left(protSepIdx).toLower(), address.mid( protSepIdx + 3 ) );
}

// return: <ErrorMessage, ADDRESS_TYPE>
QPair<QString, ADDRESS_TYPE> verifyAddress(QString address) {
    // protocol, address
    QPair<QString, QString>  protAddr = split2ProtocolAddress(address);
    if (protAddr.second.isEmpty())
        return QPair<QString, ADDRESS_TYPE>("Address has incorrect syntax", ADDRESS_TYPE::UNKNOWN);

    if (protAddr.first.isEmpty()) {
        QPair<QString, ADDRESS_TYPE> mqsRes = checkMwcMqAddress(protAddr.second);
        if (mqsRes.first.isEmpty())
            return mqsRes;

        return checkTorAddress(protAddr.second);
    }

    QString protocol = protAddr.first;
    address = protAddr.second;

    QPair<QString, ADDRESS_TYPE> tor_res = checkTorAddress(protAddr.second);
    if (tor_res.first.isEmpty()) {
        if (protocol.isEmpty() || protocol=="http") {
            return QPair<QString, ADDRESS_TYPE>("", ADDRESS_TYPE::TOR);
        }
        return QPair<QString, ADDRESS_TYPE>("TOR address support 'http' prefix only", ADDRESS_TYPE::UNKNOWN);
    }

    // both http and https are required
    if ( protocol == "https" || protocol == "http" ) {
        return QPair<QString, ADDRESS_TYPE>("", ADDRESS_TYPE::HTTPS);
    }
    else if ( protocol == "mwcmq" || protocol == "mwcmqs" )
        return checkMwcMqAddress( address );
    else // Unknown protocol
        return QPair<QString, ADDRESS_TYPE>("Address doesn't match any of expected patterns", ADDRESS_TYPE::UNKNOWN);
}

// Make an address as a full format
// type     - type og the address. Currently targeting mwc mq only
// address  - original address
QString fullFormalAddress(ADDRESS_TYPE type, QString address) {
    switch (type) {
        case ADDRESS_TYPE::MWC_MQ: {
            // Updating prefix, depend on active transport
            // protocol, address
            QPair<QString, QString>  protAddr = split2ProtocolAddress(address);
            return "mwcmqs://" + protAddr.second;
        }
        case ADDRESS_TYPE::HTTPS:
        case ADDRESS_TYPE::TOR:
        {
            return address;
        }
        case ADDRESS_TYPE::UNKNOWN:
        default: {
            Q_ASSERT(false);
            return address;
        }
    }
}

QString extractPubKeyFromAddress(QString address) {
    int idx1 = address.indexOf("//");
    int idx2 = address.indexOf("@");
    if (idx1>=0)
        idx1+=2;
    if (idx2>=0)
        idx1+=1;

    int sIdx = std::max(idx1, idx2);
    int lidx = address.lastIndexOf(".");

    if (sIdx<0)
        sIdx = 0;

    if (lidx<0)
        lidx=address.length();

    if (sIdx<lidx)
        return address.mid(sIdx, lidx-sIdx);
    else
        return "";
}

}
