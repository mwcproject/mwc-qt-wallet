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

#ifndef MWC_QT_WALLET_UTIL_B_H
#define MWC_QT_WALLET_UTIL_B_H

#include <QObject>
#include <QVector>
#include <QStringList>

namespace util {
class PasswordAnalyser;
}

namespace bridge {

class Util : public QObject {
Q_OBJECT
public:
    explicit Util(QObject *parent = nullptr);

    ~Util();

    // Open URL in current system browser
    Q_INVOKABLE void openUrlInBrowser(QString url);

    // Generate API secret string
    Q_INVOKABLE QString generateApiSecret(int length);

    // Calculate Hash for string. Needed to validate a password against hash
    Q_INVOKABLE QString calcHSA256Hash(QString str);

    // Return true if this String can be a hash
    Q_INVOKABLE bool isHashValid(QString str);
    // Return true if string looks like a signature
    Q_INVOKABLE bool isSignatureValid(QString str);
    // Return true is string looks like a public key
    Q_INVOKABLE bool isPublicKeyValid(QString str);

    // Verify address and return the address type.
    // Return values: "Unknown", "https", "tor", "mwcmqs"
    Q_INVOKABLE QString verifyAddress(QString address);

    // Password analyzer routine, simplifyed version for GUI.
    // Set word for analysys
    Q_INVOKABLE void passwordQualitySet(QString password);
    // Flag is this password acceptable for a new wallet
    Q_INVOKABLE bool passwordQualityIsAcceptable();
    // String with comments about this password. Comments most time are available
    // Note! String is HTML formatted. For plain string add a new API and add a support.
    Q_INVOKABLE QString passwordQualityComment();
    // Release PasswordAnalyser instance. It takes memory.
    Q_INVOKABLE void releasePasswordAnalyser();

    // Request a bip 39 words. There are not many of them, it is safe to get all of them
    Q_INVOKABLE QVector<QString> getBip39words();
    // Parse input phrase into the words. Does a split with some trics
    Q_INVOKABLE QVector<QString> parsePhrase2Words( QString phrase );

    // Convert nano mwc into string number ready to print
    Q_INVOKABLE QString nano2one(QString mwcNano);

    // Validate string for mwc713 input
    // Return empty string for OK. Otherwise return string is an error
    Q_INVOKABLE QString validateMwc713Str(QString str);

    // Print nicely the number
    Q_INVOKABLE QString longLong2Str(int n);

    // Trim string that represent double. 23434.32345, len 7 => 23434.32; 23434.32345, len 5 => 23434
    Q_INVOKABLE QString trimStrAsDouble(const QString & dblStr, int maxLen);

private:
    util::PasswordAnalyser * pa = nullptr;
    // PasswordAnalyser last respond values
    QPair<QString, bool> passwordAnalyserLastRespond;
    QVector<double> passwordAnalyserWeight;
    QStringList passwordAnalyserSeqWords;
    QStringList passwordAnalyserDictWords;
};

}

#endif //MWC_QT_WALLET_UTIL_B_H
