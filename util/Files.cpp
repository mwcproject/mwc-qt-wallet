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

// QT copy doesn't support overwrite, but it is exactly what we need with Android.
// This routine copy and overwrite the file
bool copyFiles(QString srcFile, QString dstFile) {
    qDebug() << "calling copyFiles " << srcFile << " to " << dstFile;
    QFile src(srcFile);
    if (!src.open(QFile::ReadOnly)) {
        qDebug() << "failed to open " << srcFile;
        Q_ASSERT(false);
        return false;
    }

    QByteArray blob = src.readAll();
    src.close();

    qDebug() << "read bytes: " << blob.size();

    if (blob.size()==0) {
        qDebug() << "No data red from " << srcFile;
        return false;
    }

    QFile dst(dstFile);
    if (!dst.open(QFile::WriteOnly | QFile::Truncate)) {
        qDebug() << "failed to open " << dstFile;
        Q_ASSERT(false);
        return false;
    }

    if ( dst.write(blob) != blob.size()) {
        qDebug() << "Unable to save all data to " << dstFile;
        return false;
    }

    qDebug() << "copyFiles done with success " << dstFile;

    dst.close();
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

// mobile devices file names access is really wierd and depend on manufacturer.
// Because of that we will try all possible cases.
QStringList calculateAlternativeFileNames( QString uriFN, QString uriDecodedFn ) {
    QStringList result;
    result << uriFN;
    if (!uriDecodedFn.isEmpty()) {
        result << uriDecodedFn;
        int idx = uriDecodedFn.indexOf("raw:/");
        if (idx>0) {
            result << uriDecodedFn.mid(idx+std::strlen("raw:"));
        }
    }

    qDebug() << "calculateAlternativeFileNames result:";
    for (auto & r : result ) {
        qDebug() << r;
    }

    return result;
}


#endif



}
