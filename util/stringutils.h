#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <QVector>

namespace util {

QVector<QString> parsePhrase2Words( QString phrase );

// convert nano items to dtirng that represent that fraction as a double
QString nano2one( long nano );

// 1.0100000 => 1.01   or 0001.0000000 => 1
QString zeroDbl2Dbl(QString  dbl);

// convert string representing double into nano
QPair<bool,long> one2nano(QString str);

// convert long strign into shorter version
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

}

#endif // STRINGUTILS_H
