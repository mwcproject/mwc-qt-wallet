#include "address.h"
#include <cassert>
#include <QVector>
#include <QCryptographicHash>

namespace util {

const int MWC_MQ_ADDR_LEN = 52;

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
        assert(carry == 0);
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

// mwc mq address might include the domain
static QPair<bool, ADDRESS_TYPE> checkMwcMqAddress( QString address ) {

    if ( address.length() != MWC_MQ_ADDR_LEN )
        return QPair<bool, ADDRESS_TYPE>(false, ADDRESS_TYPE::UNKNOWN);

    if (address.contains('@') ) {
        address = address.left( address.indexOf('@') );
    }
    else {
        // Checking the prefix
        if (!( address[0] == 'g' || address[0] == 'x' ))
            return QPair<bool, ADDRESS_TYPE>(false, ADDRESS_TYPE::UNKNOWN);
    }

    // check alphabet
    for ( auto ch : address ) {
        if ( Base58Alphabet.indexOf(ch) < 0 ) // invalid symbol
            return QPair<bool, ADDRESS_TYPE>(false, ADDRESS_TYPE::UNKNOWN);
    }

    // Let't validate the checksum
    QByteArray payload;
    if ( !decodeBase58( address, payload ) || payload.size()<5 )
        return QPair<bool, ADDRESS_TYPE>(false, ADDRESS_TYPE::UNKNOWN);

    int checksum_index = payload.size() - 4;
    QByteArray provided_checksum = payload.mid(checksum_index);

    payload = payload.left( payload.size()-4 ); // pure data

    payload = QCryptographicHash::hash(payload, QCryptographicHash::Algorithm::Sha256);
    payload = QCryptographicHash::hash(payload, QCryptographicHash::Algorithm::Sha256);

    for (int u=0;u<4;u++) {
        if ( payload[u] != provided_checksum[u] )
            return QPair<bool, ADDRESS_TYPE>(false, ADDRESS_TYPE::UNKNOWN);
    }

    return QPair<bool, ADDRESS_TYPE>(true, ADDRESS_TYPE::MWC_MQ);
}


QPair<bool, ADDRESS_TYPE> verifyAddress(QString address) {

    // Check for protocol name
    int scIdx = address.indexOf(':');

    if (scIdx<=0) {
        // must be mwc mq address
        // Length is 53
        return checkMwcMqAddress( address );
    }

    // Must have sintax
    // <protocol>://<address>
    int protSepIdx = address.indexOf("://");
    if ( protSepIdx != scIdx ) {
        // Not found correct separator
        return QPair<bool, ADDRESS_TYPE>(false, ADDRESS_TYPE::UNKNOWN);
    }

    Q_ASSERT(protSepIdx>0);

    QString protocol = address.left(protSepIdx);
    address = address.mid( protSepIdx + 3 );

    if ( address.length() ==0 )
        return QPair<bool, ADDRESS_TYPE>(false, ADDRESS_TYPE::UNKNOWN);

    // Checking protocol

    if ( protocol == "https")
        return QPair<bool, ADDRESS_TYPE>(true, ADDRESS_TYPE::HTTPS);
    else if ( protocol == "keybase")
        return QPair<bool, ADDRESS_TYPE>(true, ADDRESS_TYPE::KEYBASE);
    else if ( protocol == "mwcmq")
        return checkMwcMqAddress( address );
    else // Unknown protocol
        return QPair<bool, ADDRESS_TYPE>(false, ADDRESS_TYPE::UNKNOWN);
}

}
