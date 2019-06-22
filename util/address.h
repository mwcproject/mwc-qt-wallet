#ifndef ADDRESS_H
#define ADDRESS_H

#include <QString>
#include <QPair>

namespace util {

// Verify address that mwc wallet can operate

enum ADDRESS_TYPE { UNKNOWN, MWC_MQ, KEYBASE, HTTPS };

QPair<bool, ADDRESS_TYPE> verifyAddress(QString address);

}

#endif // ADDRESS_H
