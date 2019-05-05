#include "mwcexception.h"

namespace core {

MwcException::MwcException(QString errorMsg) : error(errorMsg) {
}

MwcException::MwcException(QString errorMsg, MwcException * nested ) : error(errorMsg), next(nested) {
}

MwcException::~MwcException() {
    if (next != nullptr) {
        delete next;
        next = nullptr;
    }
}

QString MwcException::getErrorMessage() const {
    QString res = error;
    if (next)
        res += "; " + next->getErrorMessage();
    return res;
}

}
