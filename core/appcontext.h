#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QMap>
#include "../state/state.h"
#include "../state/sendcoins.h"


namespace core {

#define COOKIE_PASSWORD "pswd"
#define ACTIVE_WND "activeWnd"

// State that applicable to all application.
class AppContext
{
public:
    AppContext();
    ~AppContext();

    // add new key/value
    template <class T>
    void pushCookie(QString key, T value);

    // remove and return
    template <class T>
    T pullCookie(QString key);

    // get is exist, don't clean up
    template <class T>
    T getCookie(QString key);

//    void setPassHash(const QString & pass);
//    bool checkPassHash(const QString & pass) const;

    // Set network need to be done from the command line
    QString getNetwork() const {return network;}

    state::STATE getActiveWndState() const {return activeWndState;}
    void setActiveWndState(state::STATE  state) {activeWndState=state;}

    // Send coins params.
    state::SendCoinsParams getSendCoinsParams() const {return sendCoinsParams;}
    void setSendCoinsParams(state::SendCoinsParams params) { sendCoinsParams=params; }

    // Get last path state. Default: Home dir
    QString getPathFor( QString name ) const;
    // update path state
    void updatePathFor( QString name, QString path );

    // IO for Int vectors
    QVector<int> getIntVectorFor( QString name ) const;
    void updateIntVectorFor( QString name, const QVector<int> & data );

    QString getCurrentAccount() const {return currentAccount;}
    void setCurrentAccount(QString account) {currentAccount = account;}

private:
    bool loadData();
    void saveData() const;

private:
    // 16 bit hash from the password. Can be used for the password verification
    // Don't use many bits because we don't want it be much usable for attacks.
//    int passHash = -1;

    // Network. Normally it is 'main'. Need to be changable from arg list
    QString network = "floonet";  // Let's keep floonet for testing. We are not ready for main

    QString currentAccount = ""; // Selected account

    // Active window that is visible
    state::STATE activeWndState = state::STATE::ACCOUNTS;

    // Send coins params.
    state::SendCoinsParams sendCoinsParams;

    // Current Path dirs
    QMap<QString,QString> pathStates;
    QMap<QString,QVector<int> > intVectorStates;
};

template <class T>
QMap<QString, T> & getCookieMap() {
    static QMap<QString, T> cookieMap;
    return cookieMap;
}

// add new key/value
template <class T>
void AppContext::pushCookie(QString key, T value) {
    getCookieMap<T>()[key] = value;
}

// remove and return
template <class T>
T AppContext::pullCookie(QString key) {
    return getCookieMap<T>().take(key);
}

// get is exist, don't clean up
template <class T>
T AppContext::getCookie(QString key) {
    return getCookieMap<T>().value(key);
}



}

#endif // APPCONTEXT_H
