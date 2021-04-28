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

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <QVector>

namespace util {

QVector<QString> parsePhrase2Words( const QString & phrase );

// convert nano items to string that represent that fraction as a double
QString nano2one( int64_t nano, bool firstcall = true );

// Trim string that represent double. 23434.32345, len 7 => 23434.32; 23434.32345, len 5 => 23434
QString trimStrAsDouble(const QString & dblStr, int maxLen);

// 1.0100000 => 1.01   or 0001.0000000 => 1
QString zeroDbl2Dbl(QString  dbl);

// convert string representing double into nano
QPair<bool,int64_t> one2nano(QString str);

// convert int64_t strign into shorter version
// abcdefgh  => abc...
QString string2shortStrR( QString str, int lenLimit );
// abcdefgh  => ab...gh
QString string2shortStrM( QString str, int lenLimit );

// 'abc' => 'abc   '
QString expandStrR(QString str, int len, QChar filler = ' ' );
// 'abc' => '   abc'
QString expandStrL(QString str, int len, QChar filler = ' ' );
// 'abc' => ' abc  '
QString expandStrM(QString str, int len, QChar filler = ' ' );

// Filter Error message: "error: Not enough funds."   =>  "Not enough funds."
QString trimErrorMessage(QString errorMsg);

// Format bunch of error messages to be ready printed one by one
QString formatErrorMessages(QStringList messages);

// Get safely substring from the string. If indexes out of range, return emoty string
QString getSubString(const QString & str, int idx1, int idx2);

// Convert mwc713 UTC time to this wallet time. Time template is different.
QString mwc713time2ThisTime(QString mwc713TimeStr);

// Convert timestamp to this wallet time.
QString timestamp2ThisTime(int64_t timestamp);

// Convert time interval in seconds into 2 sevel word description.
QString interval2String(int64_t intervalSec, bool shortUnits, int tiers);

// Validate string for acceptable symbols. mwc713 understand low ASCII synblos only. Need to verify that.
QPair <bool, QString> validateMwc713Str(QString str, bool secureStr = false);

// String to escape and prepare for mwc713 input.
// str - string to process   as sdd  =>  "as sdd";    pa@"a\s => "pa@\"a\\s"
// noSpecialCharacters - if true will clean up all characters like a new line
QString toMwc713input(QString str, bool noSpecialCharacters = true);

// Formal long number to string. Example   78,765
QString longLong2Str(int64_t n);

// Formal long number to string with length Limit. Example 1123123123, 9 => 1123.12 M
QString longLong2ShortStr(int64_t n, int length);

// Encode String into URL format. Expected that it is param or value
// https://en.wikipedia.org/wiki/Percent-encoding#Percent-encoding_in_a_URI
// All not unreserved MUST be encoded.
QString urlEncode( QString str );

// Update Event list with lines
void updateEventList( QList<QString> & events, QString str );

}

#endif // STRINGUTILS_H
