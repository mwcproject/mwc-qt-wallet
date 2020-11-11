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

#include "util_b.h"
#include "util/execute.h"
#include "../util/Generator.h"
#include "../util/crypto.h"
#include "../util/address.h"
#include "../util/passwordanalyser.h"
#include "../util/Bip39.h"
#include "../util/stringutils.h"

namespace bridge {

Util::Util(QObject *parent) : QObject(parent) {}

Util::~Util() {
    if (pa) {
        delete pa;
        pa = nullptr;
    }
}

void Util::openUrlInBrowser(QString url) {
    util::openUrlInBrowser(url);
}

// Generate API secret string
QString Util::generateApiSecret(int length) {
    return util::generateSecret(length);
}

// Calculate Hash for string. Needed to validate a password against hash
QString Util::calcHSA256Hash(QString str) {
    return crypto::calcHSA256Hash(str);
}

// Return true if this String can be a hash
bool Util::isHashValid(QString str) {
    return crypto::isHashValid(str);
}

// Return true if string looks like a signature
bool Util::isSignatureValid(QString str) {
    return crypto::isSignatureValid(str);
}

// Return true is string looks like a public key
bool Util::isPublicKeyValid(QString str) {
    return crypto::isPublicKeyValid(str);
}


// Verify address and return the address type.
// Return values: "unknown", "https", "tor", "mwcmqs"
QString Util::verifyAddress(QString address) {
    QPair<bool, util::ADDRESS_TYPE> res = util::verifyAddress(address);
    if (!res.first)
        return "unknown";

    switch (res.second) {
        case util::ADDRESS_TYPE::HTTPS: return "https";
        case util::ADDRESS_TYPE::TOR: return "tor";
        case util::ADDRESS_TYPE::MWC_MQ: return "mwcmqs";
        default: return "unknown";
    }
}

// PasswordAnalyser request internal state

// Password analyzer routine, simplifyed version for GUI.
// Set word for analysys
void Util::passwordQualitySet(QString password) {
    if (pa == nullptr) {
        pa = new util::PasswordAnalyser( "#3600C9", "#CCCCCC");
    }

    passwordAnalyserWeight.clear();
    passwordAnalyserSeqWords.clear();
    passwordAnalyserDictWords.clear();

    passwordAnalyserLastRespond = pa->getPasswordQualityReport(password, passwordAnalyserWeight,
            passwordAnalyserSeqWords, passwordAnalyserDictWords);
}

// Flag is this password acceptable for a new wallet
bool Util::passwordQualityIsAcceptable() {
    return passwordAnalyserLastRespond.second;
}
// String with comments about this password. Comments most time are available
QString Util::passwordQualityComment() {
    return passwordAnalyserLastRespond.first;
}
// Release PasswordAnalyser instance. It takes memory.
void Util::releasePasswordAnalyser() {
    if (pa == nullptr) {
        delete pa;
        pa = nullptr;
    }
}

// Request a bip 39 words. There are not many of them, it is safe to get all of them
QVector<QString> Util::getBip39words() {
    return util::getBip39words();
}

// Parse input phrase into the words. Does a split with some trics
QVector<QString> Util::parsePhrase2Words( QString phrase ) {
    return util::parsePhrase2Words(phrase);
}

// Convert nano mwc into string number ready to print
QString Util::nano2one(QString mwcNano) {
    return util::nano2one(mwcNano.toLongLong());
}

// Validate string for mwc713 input
// Return empty string for OK. Otherwise return string is an error
QString Util::validateMwc713Str(QString str) {
    auto res = util::validateMwc713Str(str);
    if (res.first)
        return "";

    Q_ASSERT(!res.second.isEmpty()); // You can't return empty string for failure. Please specify a reason
    return res.second;
}


// Print nicely the number
QString Util::longLong2Str(int n) {
    return util::longLong2Str(n);
}

// Trim string that represent double. 23434.32345, len 7 => 23434.32; 23434.32345, len 5 => 23434
QString Util::trimStrAsDouble(const QString & dblStr, int maxLen) {
    return util::trimStrAsDouble(dblStr, maxLen);
}

// Convert time interval in seconds into 2 sevel word description.
QString Util::interval2String(int intervalSec, bool shortUnits, int tiers) {
    return util::interval2String(intervalSec, shortUnits, tiers);
}


}
