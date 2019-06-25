#include "stringutils.h"

namespace util {

QVector<QString> parsePhrase2Words( QString phrase ) {

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
QString nano2one( long nano ) {
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
    while( res.length()>0 && res[0]=='0' )
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
QPair<bool,long> one2nano(QString str) {
    if (str.length()==0)
        return QPair<bool,long>(false, 0);

    bool ok = false;
    double dbl = str.toDouble(&ok);
    if (!ok)
        return QPair<bool,long>(false, 0);

    long s = 1;
    if ( dbl < 0.0 ) {
        s = -1;
        dbl = -dbl;
    }

    long nano = long(dbl * 1000000000.0 + 0.5);
    return QPair<bool,long>( true, nano*s );
}


// convert long strign into shorter version
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
        return errorMsg.mid( strlen("error:") ).trimmed();

    if ( errorMsg.startsWith("warning:", Qt::CaseInsensitive) )
        return errorMsg.mid( strlen("warning:") ).trimmed();

    if ( errorMsg.startsWith("info:", Qt::CaseInsensitive) )
        return errorMsg.mid( strlen("info:") ).trimmed();

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



}

