#include "address.h"

namespace util {

const int MWC_MQ_ADDR_LEN = 52;

static QPair<bool, ADDRESS_TYPE> checkMwcMqAddress( QString address ) {
    if ( address.length() != MWC_MQ_ADDR_LEN )
        return QPair<bool, ADDRESS_TYPE>(false, ADDRESS_TYPE::UNKNOWN);

    // Checking the prefix
    if (!( address[0] == 'g' || address[0] == 'x' ))
        return QPair<bool, ADDRESS_TYPE>(false, ADDRESS_TYPE::UNKNOWN);

    // Checking the symbols
    for ( auto ch : address ) {
        if ( ! ch.isLetterOrNumber() )
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
