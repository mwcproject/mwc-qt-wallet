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

#include "../core/global.h"
#include "stringutils.h"
#include <QDateTime>

namespace util {

QVector<QString> parsePhrase2Words( const QString & phrase ) {

    // input seed in low cases
    QVector<QString> words;
    words.push_back("");

    for (QChar ch : phrase) {
        if  (ch.isLetter()) { // letter goes to the last word
            words.back()+=ch;
        }
        else {
            // Start new word
            if (words.back().length()==0)
                continue;
            words.push_back("");
        }
    }

    // Clean up the last seprator case
    if (words.back().length()==0)
        words.pop_back();

    return words;
}


// convert nano items to dtirng that represent that fraction as a double
QString nano2one( int64_t nano ) {
    if (nano == 0)
        return "0";

    if (nano<0)
        return "-" + nano2one(-nano);

    QString myNumber = QString::number( std::abs(nano),10);
    while (myNumber.length()<9+1)
        myNumber = "0" + myNumber;

    myNumber.insert( myNumber.length()-9, '.' );
    while( myNumber[myNumber.length()-1] == '0' ) {
        myNumber.remove(myNumber.length()-1,1);
    }

    if (myNumber[myNumber.length()-1] == '.')
        myNumber.remove(myNumber.length()-1,1);

    if (nano<0)
        myNumber.push_front("-");

    return myNumber;
}

// 1.0100000 => 1.01   or 0001.0000000 => 1
QString zeroDbl2Dbl(QString  dbl) {
    QString res = dbl.trimmed();
    // remove leading chars
    while( res.length()>1 && res[0]=='0' && res[1]!='.' )
        res = res.mid(1);

    int ptIdx = res.indexOf('.');
    if (ptIdx>0) {
        while( res.length()>ptIdx && res[res.length()-1]=='0' )
            res = res.left(res.length()-1);

        if (res[res.length()-1]=='.')
            res = res.left(res.length()-1);
    }

    return res;
}



// convert string representing double into nano
QPair<bool,int64_t> one2nano(QString str) {
    if (str.length()==0)
        return QPair<bool,int64_t>(false, 0);

    bool ok = false;
    double dbl = str.toDouble(&ok);
    if (!ok)
        return QPair<bool,int64_t>(false, 0);

    int64_t s = 1;
    if ( dbl < 0.0 ) {
        s = -1;
        dbl = -dbl;
    }

    int64_t nano = int64_t(dbl * 1000000000.0 + 0.5);
    return QPair<bool,int64_t>( true, nano*s );
}

// Trim string that represent double. 23434.32345, len 7 => 23434.32; 23434.32345, len 5 => 23434
QString trimStrAsDouble(const QString & dblStr, int maxLen) {
    if (dblStr.size() < maxLen)
        return dblStr;

    QString res = dblStr;
    int ptIdx = res.indexOf('.');
    if (ptIdx<=0)
        return res;

    res = res.left( std::max(ptIdx, maxLen) );
    if ( res[res.size()-1] == '.' ) {
        res = res.left( res.size()-1 );
    }

    return res;
}


// convert int64_t strign into shorter version
// abcdefgh  => abc...
QString string2shortStrR( QString str, int lenLimit ) {
    if ( str.length() < lenLimit )
        return str;

    int pts = std::min(3,lenLimit-1);
    str.chop( (str.length() - lenLimit)-pts );

    for (int i=0;i<pts;i++)
        str.push_back('.');

    return str;
}

// 'abc' => 'abc   '
QString expandStrR(QString str, int len, QChar filler ) {
    while(str.length() < len)
        str.push_back(filler);
    return str;
}
// 'abc' => '   abc'
QString expandStrL(QString str, int len, QChar filler ) {
    while(str.length() < len)
        str.push_front(filler);
    return str;

}
// 'abc' => ' abc  '
QString expandStrM(QString str, int len, QChar filler ) {
    while(str.length() < len) {
        str.push_front(filler);
        if (str.length() < len)
            str.push_back(filler);
    }
    return str;
}


// Filter Error message: "error: Not enough funds."   =>  "Not enough funds."
QString trimErrorMessage(QString errorMsg) {
    errorMsg = errorMsg.trimmed();
    if ( errorMsg.startsWith("error:", Qt::CaseInsensitive) )
        return errorMsg.mid( (int)strlen("error:") ).trimmed();

    if ( errorMsg.startsWith("warning:", Qt::CaseInsensitive) )
        return errorMsg.mid( (int)strlen("warning:") ).trimmed();

    if ( errorMsg.startsWith("info:", Qt::CaseInsensitive) )
        return errorMsg.mid( (int)strlen("info:") ).trimmed();

    return errorMsg;
}

// Format bunch of error messages to be ready pronted one by one
QString formatErrorMessages(QStringList messages) {
    QString errMsg;
    for (auto & err : messages) {
        if (errMsg.size()>0)
            errMsg += "\n";

        errMsg += util::trimErrorMessage(err);
    }
    return errMsg;
}

// Get safely substring from the string. If indexes out of range, return emoty string
QString getSubString(const QString & str, int idx1, int idx2) {
    idx2 = std::min(idx2, str.length());

    if (idx2<=idx1 || idx1>=str.length())
        return "";

    return str.mid(idx1, idx2-idx1).trimmed();
}

static int calcOffsetFromUTC() {
     return QDateTime::currentDateTime().offsetFromUtc();
}

// Convert mwc713 UTC time to this wallet time. Time template is different.
QString mwc713time2ThisTime(QString mwc713TimeStr) {
    if (mwc713TimeStr.isEmpty())
        return mwc713TimeStr;

    static int offsetFromUTC = calcOffsetFromUTC();

    QDateTime time = QDateTime::fromString(mwc713TimeStr, mwc::DATETIME_TEMPLATE_MWC713 );
    time = time.addSecs(offsetFromUTC);

    QString res = time.toString( mwc::DATETIME_TEMPLATE_THIS);

    // Return original string if time wan't converted. It is a usecase for Json data conversion
    if (res.isEmpty() && !mwc713TimeStr.isEmpty())
        return mwc713TimeStr;

    return res;
}

QPair <bool, QString> validateMwc713Str(QString str, bool secureStr) {
    QString nonAsciiChars;

    for ( QChar ch : str ) {
        if ( ch.isLowSurrogate() || ch.isNonCharacter() || ch.isSurrogate() )
            return QPair<bool, QString>( false, (secureStr ? "Input string " : "String '" + str + "' ") + "contains not acceptable unicode characters" );

        // https://ascii.cl/
        ushort code = ch.unicode();
        if ( code>=32 && code<127 )
            continue; // Acceptable symbols

        if ( !nonAsciiChars.contains(ch) )
            nonAsciiChars += ch;
    }

    if (nonAsciiChars.isEmpty())   // success
        return QPair<bool, QString>(true, "");

    // generating the report about inputs
    QString reportStr = (secureStr ? "Input string " : "String '" + str + "' ") + "contains not acceptable symbols: ";
    for (QChar ch : nonAsciiChars)
        reportStr += QString("'") + ch + "',";

    return QPair<bool, QString>( false, reportStr.left(reportStr.size()-1) + "." );
}


// String to escape and prepare for mwc713 input.
// str - string to process   as sdd  =>  "as sdd";    pa@"a\s => "pa@\"a\\s"
// noSpecialCharacters - if true will clean up all characters like a new line
QString toMwc713input(QString str, bool noSpecialCharacters ) {
    QString res = "\"";
    for (QChar ch : str) {
        if (noSpecialCharacters && ch.unicode() < 32 ) // skipping all special chars
            continue;

        if (ch == '"')
            res += "\\\"";
        else if (ch == '\\')
            res += "\\\\";
        else
            res += ch;
    }
    res += "\"";
    return res;
}

// Formal long number to string. Example   78,765
QString longLong2Str(int64_t n) {
    QString res = QString::number(n);

    if (res.size()<=3)
        return res;

    // add commas every 3 digits
    int pos = res.length() - 3;

    while ( pos>0 && res[pos-1].isDigit() ) {
        res.insert(pos, ',');
        pos -= 3;
    }
    return res;
}

// Formal long number to string with length Limit. Example 1123123123, 9 => 1123.12 M
QString longLong2ShortStr(int64_t n, int lengthLimit) {
    QString numberStr = longLong2Str(n);

    if (numberStr.length()<=lengthLimit)
        return numberStr;

    // Need to shorten the string
    // We can do
    // M - millions, 6+2 digits
    // B - billion,  9+3 Digits
    // T - trillion, 12+4 Digits
    // Q - quadrillions, 15+5  Digits

    // Letter will take 2 digits
    int scale = numberStr.length() - lengthLimit + 2;
    QString decimals;
    QString letter;
    QString number;
    if (scale <= 6+2) {
        letter = "M";
        scale = 6+2;
    }
    else if (scale <= 9+3) {
        letter = "B";
        scale = 9+3;
    }
    else if (scale <= 12+4) {
        letter = "T";
        scale = 12+4;
    }
    else {
        letter = "Q";
        scale = 15+5;
    }

    number = numberStr.left( numberStr.size() - scale );
    decimals = numberStr.right( scale ).remove(',');
    if (decimals.length()>3)
        decimals = decimals.left(3); // we really don't want print many decimals. It doesn't look good

    if (number.length()+2 <= lengthLimit-2 ) {
        number += "." + decimals;
        number = number.left(lengthLimit-2);

        // Clean up .00
        while ( number.size()>0 && number[number.size()-1] == '0' )
            number = number.left(number.size()-1);

        if ( number.size()>0 && number[number.size()-1] == '.' )
            number = number.left(number.size()-1);
    }

    return number + " " + letter;
}

static bool isunreserved(unsigned char in)
{
    switch(in) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '-': case '.': case '_': case '~':
            return true;
        default:
            break;
    }
    return false;
}

static const QString hexEncStr = "0123456789ABCDEF";

// Encode String into URL format. Expected that it is param or value
// https://en.wikipedia.org/wiki/Percent-encoding#Percent-encoding_in_a_URI
// All not unreserved MUST be encoded.
QString urlEncode( QString str ) {
    QString result;

    QByteArray chars = str.toUtf8();
    // utf 8 fits isunreserved pretty well. Only 1 byte can be mapped to the symbol
    for (unsigned char ch : chars ) {
        if (isunreserved(ch)) {
            result += ch;
        }
        else {
            // Percent encoding is required
            uint chI = ch;
            result += '%';
            result += hexEncStr[chI/16];
            result += hexEncStr[chI%16];
        }
    }
    return result;
}

// Update Event list with lines
void updateEventList( QList<QString> & events, QString str ) {
    auto lns = str.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);

    for (auto ln : lns) {
        if (ln.isEmpty())
            continue;

        events.push_back(ln);
    }
}


}

