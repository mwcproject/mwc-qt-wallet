#ifndef QTANDROIDSERVICE_H
#define QTANDROIDSERVICE_H

#include <QObject>
#include <QtAndroid>
#include <QAndroidIntent>
#include <QAndroidActivityResultReceiver>

class QtAndroidService : public QObject, protected QAndroidActivityResultReceiver
{
    Q_OBJECT

public:
    QtAndroidService(QObject *parent = nullptr);

    static QtAndroidService *instance() { return m_instance; }
    Q_INVOKABLE void sendToService(QString message);
    Q_INVOKABLE bool requestPermissions();

    // Type values:  .cfg -> text/plain  - doesn't work
    //  text/plain
    //  text/*
    Q_INVOKABLE void openFile( QString pickerInitialUri, QString type, int eventCode );
protected:
    virtual void handleActivityResult(int receiverRequestCode, int resultCode, const QAndroidJniObject &data) override;

signals:
    void messageFromService(const QString &message);

    // Empty file - rejection case
    void sgnOnFileOpen( int eventCode, QString fileUri );
private:
    void registerNatives();
    void registerBroadcastReceiver();

    static QtAndroidService *m_instance;
};


#endif // QTANDROIDSERVICE_H
