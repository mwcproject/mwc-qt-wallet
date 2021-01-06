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

#include <string>
#include "util_b.h"
#include "util/execute.h"
#include "../util/Generator.h"
#include "../util/crypto.h"
#include "../util/address.h"
#include "../util/passwordanalyser.h"
#include "../util/Bip39.h"
#include "../util/stringutils.h"
#include "../util/QrCode.h"
#include "../util/Json.h"
#include "../util/filedialog.h"
#include "../util/Files.h"

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

using namespace qrcodegen;

// Ganarating a QR image for the text content
// Result is 2 element array:
// res[0]  - size of the QR code
// res[1]  - SVG string that can draw the QR code. Seems like both Desctop and QML can draw the SVG path.
QVector<QString> Util::generateQrCode(QString content) {
    const QrCode qrCodeResult = QrCode::encodeText(content.toStdString().c_str(), QrCode::Ecc::MEDIUM);

    int size = qrCodeResult.getSize();
    QString svgContent = QString::fromStdString( qrCodeResult.toSvgString(1) );
    return { QString::number(size), svgContent };
}

// Parsing slate as a Json. Respond back with error or with Slate details
// slateContent  - slate sjon string to parse.
// fileTransactionType  - value of FileTransactionType
// Error case:
// res[0] - error message
// Normal case:
// res[0] = transactionId
// res[1] = amount
QVector<QString> Util::parseSlateContent( QString slateContent, int fileTransactionType, QString slateSenderAddress ) {
    util::FileTransactionInfo fileInfo;

    QPair<bool, QString> res = fileInfo.parseSlateContent( slateContent, util::FileTransactionType(fileTransactionType), slateSenderAddress );
    if (!res.first) {
        // error
        return { res.second };
    }

    return { fileInfo.transactionId, QString::number(fileInfo.amount) };
}

// Open QFileDialog::getSaveFileName with all standard verificaitons that we normally have
// Return file name or empty value is request was cancelled or error happens
QString Util::getSaveFileName(QString title, QString callerId, QString extentionsDlg, QString extentionFile) {
    return util::getSaveFileName(title, callerId, extentionsDlg, extentionFile);
}

// Open QFileDialog::getOpenFileName with all standard verificaitons that we normally have
// Return file name or empty value is request was cancelled or error happens
QString Util::getOpenFileName(QString title, QString callerId, QString extentionsDlg) {
    return util::getOpenFileName(title, callerId, extentionsDlg);
}

// Write some text into the file
bool Util::writeTextFile(QString fileName, QStringList lines ) {
    return util::writeTextFile(fileName, lines);
}

}
