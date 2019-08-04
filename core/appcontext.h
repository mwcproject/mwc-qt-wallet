#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include <QMap>
#include "../state/state.h"
#include "../state/m_airdrop.h"

class QAction;

namespace core {

struct SendCoinsParams {
    int inputConfirmationNumber;
    int changeOutputs;

    // Expected to be deafult values
    SendCoinsParams() :
            inputConfirmationNumber(10), changeOutputs(1) {}

    SendCoinsParams( int _inputConfirmationNumber, int _changeOutputs) :
            inputConfirmationNumber(_inputConfirmationNumber), changeOutputs(_changeOutputs) {}

    void setData(int _inputConfirmationNumber, int _changeOutputs) {
        inputConfirmationNumber = _inputConfirmationNumber;
        changeOutputs = _changeOutputs;
    }

    SendCoinsParams(const SendCoinsParams & o) = default;
    SendCoinsParams & operator = (const SendCoinsParams & o) = default;

    bool operator == (const SendCoinsParams & o ) {return inputConfirmationNumber==o.inputConfirmationNumber && changeOutputs==o.changeOutputs;}

    void saveData(QDataStream & out) const;
    bool loadData(QDataStream & in);
};

struct ContactRecord {
    QString name;
    QString address;

    bool operator ==(const ContactRecord & o) {return name == o.name && address == o.address;}

    void setData(QString name,
                 QString address);

    void saveData( QDataStream & out) const;
    bool loadData( QDataStream & in);
};


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

    state::STATE getActiveWndState() const {return activeWndState;}
    void setActiveWndState(state::STATE  state) {activeWndState=state;}

    // Send coins params.
    SendCoinsParams getSendCoinsParams() const {return sendCoinsParams;}
    void setSendCoinsParams(SendCoinsParams params) { sendCoinsParams=params; }

    // Get last path state. Default: Home dir
    QString getPathFor( QString name ) const;
    // update path state
    void updatePathFor( QString name, QString path );

    // IO for Int vectors
    QVector<int> getIntVectorFor( QString name ) const;
    void updateIntVectorFor( QString name, const QVector<int> & data );

    QString getReceiveAccount() const {return recieveAccount;}
    void setReceiveAccount(QString account) {recieveAccount = account;}

    QString getCurrentAccountName() const {return currentAccountName;};
    void setCurrentAccountName(QString currentAccount) {currentAccountName = currentAccount;}

    // AirdropRequests will handle differently
    void saveAirdropRequests( const QVector<state::AirdropRequests> & data );
    QVector<state::AirdropRequests> loadAirdropRequests() const;

    // -------------- Contacts
    // Get the contacts
    QVector<ContactRecord> getContacts() const {return contactList;}
    // Add s new contact
    QPair<bool, QString> addContact( const ContactRecord & contact );
    // Remove contact. return false if not found
    QPair<bool, QString> deleteContact( const ContactRecord & contact );
    // Update contact
    QPair<bool, QString> updateContact( const ContactRecord & prevValue, const ContactRecord & newValue );


private:
    bool loadData();
    void saveData() const;

private:
    // 16 bit hash from the password. Can be used for the password verification
    // Don't use many bits because we don't want it be much usable for attacks.
//    int passHash = -1;

    QString recieveAccount = "default"; // Selected account
    QString currentAccountName = "default"; // Current account

    // Active window that is visible
    state::STATE activeWndState = state::STATE::LISTENING;

    // Send coins params.
    SendCoinsParams sendCoinsParams;

    // Current Path dirs
    QMap<QString,QString> pathStates;
    QMap<QString,QVector<int> > intVectorStates;

    // Contact list
    QVector<ContactRecord> contactList;
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
