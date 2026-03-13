#ifndef QTANDROIDSERVICE_H
#define QTANDROIDSERVICE_H

#include <QObject>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>
#include <QJniObject>
using AndroidJniObject = QJniObject;
#endif

#ifdef Q_OS_ANDROID
class QtAndroidService : public QObject, protected QAndroidActivityResultReceiver
#else
class QtAndroidService : public QObject
#endif
{
    Q_OBJECT

public:
    QtAndroidService(QObject *parent = nullptr);

    Q_INVOKABLE bool requestPermissions();

    Q_INVOKABLE QString getApkVersion();

    // Type values:  .cfg -> text/plain  - doesn't work
    //  text/plain
    //  text/*
    Q_INVOKABLE void openFile( QString pickerInitialUri, QString type, int eventCode );
    Q_INVOKABLE void createFile( QString pickerInitialUri, QString type, QString fileName, int eventCode );

#ifdef Q_OS_ANDROID
protected:
    virtual void handleActivityResult(int receiverRequestCode, int resultCode, const AndroidJniObject &data) override;
#endif

signals:
    // Empty file - rejection case
    void sgnOnFileReady( int eventCode, QString fileUri );
};


#endif // QTANDROIDSERVICE_H
