#include "qtandroidservice.h"

#include <QCoreApplication>
#include <QtDebug>
#include <QMetaObject>

#ifdef Q_OS_ANDROID
#include <QJniEnvironment>
#include <QtCore/qcoreapplication_platform.h>

const int RESULT_OK = -1;
#endif

QtAndroidService::QtAndroidService(QObject *parent) : QObject(parent)
{
}

bool QtAndroidService::requestPermissions()
{
#ifdef Q_OS_ANDROID
    const QVector<QString> permissions({"android.permission.WRITE_EXTERNAL_STORAGE", "android.permission.READ_EXTERNAL_STORAGE"});
    for(const QString &permission : permissions){
        auto result = QtAndroidPrivate::checkPermission(permission).result();
        if(result == QtAndroidPrivate::PermissionResult::Denied){
            auto requestResult = QtAndroidPrivate::requestPermission(permission).result();
            if(requestResult == QtAndroidPrivate::PermissionResult::Denied)
                return false;
        }
    }

    return true;
#else
    return true;
#endif
}
// Type values:  .cfg -> text/plain  - doesn't work this way
//  text/plain
//  text/*
void QtAndroidService::openFile( QString pickerInitialUri, QString type, int eventCode ) {
#ifdef Q_OS_ANDROID
    AndroidJniObject jpickerInitialUri = AndroidJniObject::fromString(pickerInitialUri);
    AndroidJniObject jtype = AndroidJniObject::fromString(type);

    AndroidJniObject intent = AndroidJniObject::callStaticObjectMethod("mw/mwc/wallet/QmlHelper",
                                                                         "buildOpenFileIntent",
                                                                         "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                                                                         jpickerInitialUri.object<jstring>(),
                                                                         jtype.object<jstring>());

    QtAndroidPrivate::startActivity(intent, eventCode, this);
#else
    Q_UNUSED(pickerInitialUri)
    Q_UNUSED(type)
    qWarning() << "QtAndroidService::openFile is not implemented on this platform";
    QMetaObject::invokeMethod(this, [this, eventCode]() { emit sgnOnFileReady(eventCode, QString()); },
                              Qt::QueuedConnection);
#endif
}

void QtAndroidService::createFile( QString pickerInitialUri, QString type, QString fileName, int eventCode ) {
#ifdef Q_OS_ANDROID
    AndroidJniObject jpickerInitialUri = AndroidJniObject::fromString(pickerInitialUri);
    AndroidJniObject jtype = AndroidJniObject::fromString(type);
    AndroidJniObject jfileName = AndroidJniObject::fromString(fileName);

    // Intent buildCreateFileIntent(String pickerInitialUri, String type, String filename )
    AndroidJniObject intent = AndroidJniObject::callStaticObjectMethod("mw/mwc/wallet/QmlHelper",
                                                                         "buildCreateFileIntent",
                                                                         "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                                                                         jpickerInitialUri.object<jstring>(),
                                                                         jtype.object<jstring>(),
                                                                         jfileName.object<jstring>());

    QtAndroidPrivate::startActivity(intent, eventCode, this);
#else
    Q_UNUSED(pickerInitialUri)
    Q_UNUSED(type)
    Q_UNUSED(fileName)
    qWarning() << "QtAndroidService::createFile is not implemented on this platform";
    QMetaObject::invokeMethod(this, [this, eventCode]() { emit sgnOnFileReady(eventCode, QString()); },
                              Qt::QueuedConnection);
#endif
}

#ifdef Q_OS_ANDROID
void QtAndroidService::handleActivityResult(int receiverRequestCode, int resultCode, const AndroidJniObject &data) {
    if ( resultCode == RESULT_OK ) {
        AndroidJniObject uri = data.callObjectMethod("getDataString","()Ljava/lang/String;");
        emit sgnOnFileReady( receiverRequestCode, uri.toString() );
    }
    else {
        qDebug() << "QtAndroidService::handleActivityResult request " << receiverRequestCode << " failed with resultCode " << resultCode;
        emit sgnOnFileReady( receiverRequestCode, "" );
    }
}
#endif

QString QtAndroidService::getApkVersion()
{
#ifdef Q_OS_ANDROID
    QJniObject activity(QNativeInterface::QAndroidApplication::context().object());
    QJniObject packageName, packageManager, packageInfo;
    QJniEnvironment env;
    QString info;

    packageManager = activity.callObjectMethod("getPackageManager", "()Landroid/content/pm/PackageManager;");
    packageName = activity.callObjectMethod("getPackageName", "()Ljava/lang/String;");

    packageInfo = packageManager.callObjectMethod("getPackageInfo",
                                                  "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;",
                                                  packageName.object<jstring>(),
                                                  0);

    qDebug() << "Requesting Package info";

    if (!env->ExceptionCheck())
    {
        info = packageInfo.getObjectField<jstring>("versionName").toString();
        qDebug() << "info: " << info;
    }
    else
    {
        env->ExceptionClear();
        qDebug() << "Got an error..." << info;
    }

    return info;
#else
    return QCoreApplication::applicationVersion();
#endif
}
