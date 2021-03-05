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

#ifndef ADDRESS_H
#define ADDRESS_H

#include <QString>
#include <QPair>

namespace util {

// Verify address that mwc wallet can operate
// KEYBASE=3,  - absolete value. Index is reserved because of that
enum class ADDRESS_TYPE { UNKNOWN=1, MWC_MQ=2,  HTTPS=4, TOR=5 };

// return: <ErrorMessage, ADDRESS_TYPE>
QPair<QString, ADDRESS_TYPE> verifyAddress(QString address);

// Make an address as a full format
// type     - type og the address. Currently targeting mwc mq only
// address  - original address
QString fullFormalAddress(ADDRESS_TYPE type, QString address);

// Extract the middle part form the address. It is caller responsibility to verify the address type
// This method will remove the preffix and suffix and return what is left
QString extractPubKeyFromAddress(QString address);

}

#endif // ADDRESS_H
