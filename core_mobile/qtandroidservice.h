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

    Q_INVOKABLE void setNotification(const QString &notification);
    Q_INVOKABLE QString notification() const;

    Q_INVOKABLE void sendToService(QString message);
    Q_INVOKABLE bool requestPermissions();

    Q_INVOKABLE QString getApkVersion();

    // Type values:  .cfg -> text/plain  - doesn't work
    //  text/plain
    //  text/*
    Q_INVOKABLE void openFile( QString pickerInitialUri, QString type, int eventCode );
    Q_INVOKABLE void createFile( QString pickerInitialUri, QString type, QString fileName, int eventCode );
    Q_INVOKABLE void setBarAndroid( int statusBarColor, int navigationBarColor, int statusBarWindows );
protected:
    virtual void handleActivityResult(int receiverRequestCode, int resultCode, const QAndroidJniObject &data) override;

signals:
    void messageFromService(const QString &message);
    void notificationChanged();

    // Empty file - rejection case
    void sgnOnFileReady( int eventCode, QString fileUri );

private slots:
    void updateAndroidNotification();

private:
    void registerNatives();
    void registerBroadcastReceiver();

    QString m_notification;
    static QtAndroidService *m_instance;
};

#endif // QTANDROIDSERVICE_H
