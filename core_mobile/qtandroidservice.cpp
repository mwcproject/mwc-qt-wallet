#include "qtandroidservice.h"

#include <QtAndroid>
#include <QAndroidIntent>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QtDebug>

QtAndroidService *QtAndroidService::m_instance = nullptr;
const int RESULT_OK = -1;

static void receivedFromAndroidService(JNIEnv *env, jobject /*thiz*/, jstring value)
{
    emit QtAndroidService::instance()->messageFromService(
        env->GetStringUTFChars(value, nullptr));
}

QtAndroidService::QtAndroidService(QObject *parent) : QObject(parent)
{
    m_instance = this;

    registerNatives();
    registerBroadcastReceiver();
    connect(this, SIGNAL(notificationChanged()), this, SLOT(updateAndroidNotification()));
}

void QtAndroidService::sendToService(QString message)
{
    QAndroidIntent serviceIntent(QtAndroid::androidActivity().object(),
                                "mw/mwc/wallet/QtAndroidService");
    serviceIntent.putExtra("message", message.toUtf8());

    QAndroidJniObject result = QtAndroid::androidActivity().callObjectMethod(
                "startService",
                "(Landroid/content/Intent;)Landroid/content/ComponentName;",
                serviceIntent.handle().object());
}

bool QtAndroidService::requestPermissions()
{
    const QVector<QString> permissions({"android.permission.WRITE_EXTERNAL_STORAGE", "android.permission.READ_EXTERNAL_STORAGE"});
    for(const QString &permission : permissions){
        auto result = QtAndroid::checkPermission(permission);
        if(result == QtAndroid::PermissionResult::Denied){
            auto resultHash = QtAndroid::requestPermissionsSync(QStringList({permission}));
            if(resultHash[permission] == QtAndroid::PermissionResult::Denied)
                return false;
        }
    }

    return true;
}

void QtAndroidService::setBarAndroid(int statusBarColor, int navigationBarColor, int statusBarWindows) {
    QtAndroid::runOnAndroidThread([=]() {
        QAndroidJniObject window = QtAndroid::androidActivity().callObjectMethod("getWindow", "()Landroid/view/Window;");
        window.callMethod<void>("addFlags", "(I)V", 0x80000000);
        window.callMethod<void>("clearFlags", "(I)V", 0x04000000);
        window.callMethod<void>("setStatusBarColor", "(I)V", statusBarColor); // Desired statusbar color
        window.callMethod<void>("setNavigationBarColor", "(I)V", navigationBarColor); // Desired statusbar color

        //QAndroidJniObject decorView = window.callObjectMethod("getDecorView", "()Landroid/view/View;");
        //int flags = 0x00000010 | 0x00002000;
        //decorView.callMethod<void>("setSystemUiVisibility", "(I)V", flags);
    });

}

void QtAndroidService::registerNatives(){
    JNINativeMethod methods[] {
        {"sendToQt", "(Ljava/lang/String;)V", reinterpret_cast<void *>(receivedFromAndroidService)}};
    QAndroidJniObject javaClass("mw/mwc/wallet/ActivityUtils");

    QAndroidJniEnvironment env;
    jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
    env->RegisterNatives(objectClass,
                         methods,
                         sizeof(methods) / sizeof(methods[0]));
    env->DeleteLocalRef(objectClass);
}

void QtAndroidService::registerBroadcastReceiver()
{
    QAndroidJniEnvironment env;
    jclass javaClass = env.findClass("mw/mwc/wallet/ActivityUtils");
    QAndroidJniObject classObject(javaClass);

    classObject.callMethod<void>("registerServiceBroadcastReceiver",
                                 "(Landroid/content/Context;)V",
                                 QtAndroid::androidContext().object());
}

// Type values:  .cfg -> text/plain  - doesn't work this way
//  text/plain
//  text/*
void QtAndroidService::openFile( QString pickerInitialUri, QString type, int eventCode ) {
    QAndroidJniObject jpickerInitialUri = QAndroidJniObject::fromString(pickerInitialUri);
    QAndroidJniObject jtype = QAndroidJniObject::fromString(type);

    QAndroidJniObject intent = QAndroidJniObject::callStaticObjectMethod("mw/mwc/wallet/QmlHelper",
                                                                         "buildOpenFileIntent",
                                                                         "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                                                                         jpickerInitialUri.object<jstring>(),
                                                                         jtype.object<jstring>());

    QtAndroid::startActivity(intent, eventCode, this);
}

void QtAndroidService::createFile( QString pickerInitialUri, QString type, QString fileName, int eventCode ) {
    QAndroidJniObject jpickerInitialUri = QAndroidJniObject::fromString(pickerInitialUri);
    QAndroidJniObject jtype = QAndroidJniObject::fromString(type);
    QAndroidJniObject jfileName = QAndroidJniObject::fromString(fileName);

    // Intent buildCreateFileIntent(String pickerInitialUri, String type, String filename )
    QAndroidJniObject intent = QAndroidJniObject::callStaticObjectMethod("mw/mwc/wallet/QmlHelper",
                                                                         "buildCreateFileIntent",
                                                                         "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                                                                         jpickerInitialUri.object<jstring>(),
                                                                         jtype.object<jstring>(),
                                                                         jfileName.object<jstring>());

    QtAndroid::startActivity(intent, eventCode, this);
}


void QtAndroidService::handleActivityResult(int receiverRequestCode, int resultCode, const QAndroidJniObject &data) {
    if ( resultCode == RESULT_OK ) {
        QAndroidJniObject uri = data.callObjectMethod("getDataString","()Ljava/lang/String;");
        emit sgnOnFileReady( receiverRequestCode, uri.toString() );
    }
    else {
        qDebug() << "QtAndroidService::handleActivityResult request " << receiverRequestCode << " failed with resultCode " << resultCode;
        emit sgnOnFileReady( receiverRequestCode, "" );
    }
}

QString QtAndroidService::getApkVersion()
{
    QAndroidJniObject activity = QtAndroid::androidActivity();
    QAndroidJniObject packageName, packageManager, packageInfo;
    QAndroidJniEnvironment env;
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
}

void QtAndroidService::setNotification(const QString &notification)
{
    if (m_notification == notification)
        return;

    m_notification = notification;
    emit notificationChanged();
}

QString QtAndroidService::notification() const
{
    return m_notification;
}

void QtAndroidService::updateAndroidNotification()
{

    QAndroidJniObject javaNotification = QAndroidJniObject::fromString(m_notification);
    QAndroidJniObject javaClass("mw/mwc/wallet/ActivityUtils");
    QAndroidJniObject classObject(javaClass);
    classObject.callMethod<void>("notify",
                                 "(Landroid/content/Context;Ljava/lang/String;)V",
                                 QtAndroid::androidContext().object(),
                                 javaNotification.object<jstring>());
}



