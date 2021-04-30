#include <QtAndroidExtras>
#include <QDebug>
#include "androidinterface.h"

AndroidInterface::AndroidInterface()
{
}

QVariant AndroidInterface::getApkVersion()
{
    QAndroidJniObject activity = QtAndroid::androidActivity();
    QAndroidJniObject packageName, packageManager, packageInfo;
    QAndroidJniEnvironment env;
    QVariant info;

    packageManager = activity.callObjectMethod("getPackageManager", "()Landroid/content/pm/PackageManager;");
    packageName = activity.callObjectMethod("getPackageName", "()Ljava/lang/String;");

    packageInfo = packageManager.callObjectMethod("getPackageInfo",
                                                  "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;",
                                                  packageName.object<jstring>(),
                                                  0);
    if (!env->ExceptionCheck())
    {
        info = packageInfo.getObjectField<jstring>("versionName").toString();
    }
    else
    {
        env->ExceptionClear();
    }

    return info;
}
