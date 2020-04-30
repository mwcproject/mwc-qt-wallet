// Copyright 2020 The MWC Developers
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

#ifndef MWC_QT_WALLET_TESTLOGS_H
#define MWC_QT_WALLET_TESTLOGS_H

namespace test {

// Write about 1 Gb of the logs and check how it will be rotated. Note, test will be slow.
// Test expect mwczip  util.
void testLogsRotation();

}


#endif //MWC_QT_WALLET_TESTLOGS_H
