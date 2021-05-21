// Copyright 2019 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Files.h"
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QDebug>

#ifdef WALLET_MOBILE
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#include <QAndroidIntent>
#endif

namespace util {

QStringList readTextFile( QString fileName, bool trimmed, bool cleanEMptyLines, std::function<void()> openOpenErrorCallback ) {
    QFile file(fileName);
    QStringList res;
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        openOpenErrorCallback();
        return res;
    }

    QTextStream in(&file);

    while(!in.atEnd()) {
        QString line = in.readLine();
        if (trimmed)
            line = line.trimmed();
        if (cleanEMptyLines && line.isEmpty())
            continue;

        res << line;
    }
    file.close();
    return res;
}

// Write text lines into the file
bool writeTextFile(QString fileName, const QStringList & lines ) {

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        Q_ASSERT(false);
        return false;
    }

    QTextStream out(&file);

    for (const auto & ln : lines) {
        out << ln << endl;
    }

    file.close();
    return true;
}

#ifdef WALLET_MOBILE

// Clear all files in temp directory
void clearTempDir() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir dir( path );

    dir.setFilter( QDir::NoDotAndDotDot | QDir::Files );
    foreach( QString dirItem, dir.entryList() )
            dir.remove( dirItem );
}

// Return file name at tmp directory
QString genTempFileName(QString extension) {
    QString tmpPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDateTime now;
    QString fileName = tmpPath + "/" + now.currentDateTime().toString("MMMM-d-yyyy-hh-mm-ss") + extension;
    return fileName;
}

bool copyUriToFile(QString uriFN, QString fsFN) {
    QAndroidJniObject activity = QtAndroid::androidActivity();
    QAndroidJniObject juriFN = QAndroidJniObject::fromString(uriFN);
    QAndroidJniObject jfsFN = QAndroidJniObject::fromString(fsFN);

    bool result = QAndroidJniObject::callStaticMethod<jboolean>("mw/mwc/wallet/QmlHelper",
                                                                         "copyUriToFile",
                                                                         "(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;)Z",
                                                                         activity.object(),
                                                                         juriFN.object<jstring>(),
                                                                         jfsFN.object<jstring>());

    qDebug() << "copyUriToFile with " + uriFN + ", " + fsFN + " return " << result;
    return result;
}

bool copyFileToUri(QString fsFN, QString uriFN) {
    QAndroidJniObject activity = QtAndroid::androidActivity();
    QAndroidJniObject jfsFN = QAndroidJniObject::fromString(fsFN);
    QAndroidJniObject juriFN = QAndroidJniObject::fromString(uriFN);

    bool result = QAndroidJniObject::callStaticMethod<jboolean>("mw/mwc/wallet/QmlHelper",
                                                                         "copyFileToUri",
                                                                         "(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;)Z",
                                                                         activity.object(),
                                                                         jfsFN.object<jstring>(),
                                                                         juriFN.object<jstring>());

    qDebug() << "copyFileToUri with " + fsFN + ", " + uriFN + " return " << result;
    return result;
}

#endif



}
