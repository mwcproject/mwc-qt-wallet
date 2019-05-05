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
