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

#ifndef MWC_QT_WALLET_DICTIONARYINIT_H
#define MWC_QT_WALLET_DICTIONARYINIT_H

namespace misk {

// This method was call once to generated the dictionary data. Linker expected not to include this code into executable
// NOTE!!!! Everything is hardcoded there. Please take a look before start using it!

// I take passwords from
// https://github.com/danielmiessler/SecLists/tree/master/Passwords
// Series:  xato-net-10-million-passwords
void provisionDictionary();


}


#endif //MWC_QT_WALLET_DICTIONARYINIT_H
