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

#ifndef PASSWORDANALYSER_H
#define PASSWORDANALYSER_H

#include <QString>

namespace util {

class PasswordAnalyser
{
public:
    PasswordAnalyser(const QString & password);

    // return html string that describe the quality of the password
    // Example: <font color="red">aaaa</font>
    QString getPasswordQualityStr();

    bool isPasswordOK() const;

private:
    QString pass2check;

    int len;
    int letters;
    int nonLetters;
    int capitals;
    int locase;
    int entropyLen;
};

}

#endif // PASSWORDANALYSER_H
