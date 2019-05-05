#ifndef MWCEXCEPTION_H
#define MWCEXCEPTION_H

#include <QException>

namespace core {

class MwcException : public QException
{
public:
    MwcException(QString errorMsg);
    MwcException(QString errorMsg, MwcException * nested );
    virtual ~MwcException();

    void raise() const override { throw *this; }
    virtual MwcException *clone() const override { return new MwcException(*this); }

    QString getErrorMessage() const;
protected:
    QString error;
    MwcException * next = nullptr;
};

}

#endif // MWCEXCEPTION_H
