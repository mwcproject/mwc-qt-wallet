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

enum ADDRESS_TYPE { UNKNOWN, MWC_MQ, KEYBASE, HTTPS };

QPair<bool, ADDRESS_TYPE> verifyAddress(QString address);

}

#endif // ADDRESS_H
