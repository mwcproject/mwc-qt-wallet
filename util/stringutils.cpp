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

    QString myNumber = QString::number( std::abs(nano),10);
    while (myNumber.length()<9+1)
        myNumber = "0" + myNumber;

    myNumber.insert( myNumber.length()-9, '.' );
    while( myNumber.back() == '0' ) {
        myNumber.remove(myNumber.length()-1,1);
    }

    if (myNumber.back() == '.')
        myNumber.remove(myNumber.length()-1,1);

    if (nano<0)
        myNumber.push_front("-");

    return myNumber;
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

}

