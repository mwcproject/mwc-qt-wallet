#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <QVector>

namespace util {

QVector<QString> parsePhrase2Words( QString phrase );

// convert nano items to dtirng that represent that fraction as a double
QString nano2one( long nano );

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

}

#endif // STRINGUTILS_H
