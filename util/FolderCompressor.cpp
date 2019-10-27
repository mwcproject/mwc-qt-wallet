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

#include "FolderCompressor.h"
#include <QFile>
#include <QDir>
#include <QDataStream>
#include <QCoreApplication>

namespace compress {

const int ARCH_VERSION = 0x9265DB;
const int ARCH_DIR_VER      = 0x587634;
const int ARCH_FILE_VER     = 0x823AD1;

// return: <success, Error Message>
static QPair<bool, QString> compress(QDataStream & dataStream, // target
                QString sourceFolder, QString prefix, bool callProcessEvents) {
    QDir dir(sourceFolder);
    if (!dir.exists())
        return QPair<bool, QString>(false, "Unable to compress directory " + sourceFolder);

    // Forders can be empty, we want to store them as well
    dataStream << int(ARCH_DIR_VER);
    dataStream << QString(prefix);

               //1 - list all folders inside the current folder
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
    QFileInfoList foldersList = dir.entryInfoList();

    if (callProcessEvents)
        QCoreApplication::processEvents();

    //2 - For each folder in list: call the same function with folders' paths
    for (int i = 0; i < foldersList.length(); i++) {
        QString folderName = foldersList.at(i).fileName();
        QString folderPath = dir.absolutePath() + "/" + folderName;
        QString newPrefex = prefix + "/" + folderName;

        compress(dataStream, folderPath, newPrefex, callProcessEvents);
    }

    //3 - List all files inside the current folder
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Files);
    QFileInfoList filesList = dir.entryInfoList();

    //4- For each file in list: add file path and compressed binary data
    for (int i = 0; i < filesList.length(); i++)
    {
        if (callProcessEvents)
            QCoreApplication::processEvents();

        QFile file(dir.absolutePath() + "/" + filesList.at(i).fileName());
        if (!file.open(QIODevice::ReadOnly))//couldn't open file
        {
            return QPair<bool, QString>(false, "Unable to open file " + QFileInfo(file).absoluteFilePath() );
        }

        dataStream << int(ARCH_FILE_VER);
        dataStream << QString(prefix + "/" + filesList.at(i).fileName());
        dataStream << qCompress(file.readAll());

        file.close();
    }

    return QPair<bool, QString>( true, "" );
}


//A recursive function that scans all files inside the source folder
//and serializes all files in a row of file names and compressed
//binary data in a single file
// return: <success, Error Message>
QPair<bool, QString> compressFolder(QString sourceFolder, QString destinationFile, const QString & archiveTag, bool callProcessEvents) {
    QDir src(sourceFolder);
    if(!src.exists())
        return QPair<bool, QString>(false, "Not found source folder " + sourceFolder);

    QFile file;
    file.setFileName(destinationFile);
    if(!file.open(QIODevice::WriteOnly))
        return QPair<bool, QString>(false, "Unable to create archive file " + destinationFile);

    QDataStream dataStream;
    dataStream.setDevice(&file);

    // File version
    dataStream << int(ARCH_VERSION);
    dataStream << archiveTag;

    QPair<bool, QString> compResult = compress(dataStream, sourceFolder, "", callProcessEvents);
    file.close();

    return compResult;
}

//A function that deserializes data from the compressed file and
//creates any needed subfolders before saving the file
// return: <success, Error Message>
QPair<bool, QString> decompressFolder(QString sourceFile, QString destinationFolder, const QString & archiveTag, bool callProcessEvents) {

    //validation
    QFile src(sourceFile);
    if (!src.exists()) {//file not found, to handle later
        return QPair<bool, QString>( false, "Not found file to extract the data from: " + sourceFile );
    }

    QFile file;
    file.setFileName(sourceFile);
    if (!file.open(QIODevice::ReadOnly))
        return QPair<bool, QString>( false, "Unable to open file to extract the data from: " + sourceFile );

    QDataStream dataStream;
    dataStream.setDevice(&file);

    int version;
    dataStream >> version;

    if (version!=ARCH_VERSION)
        return QPair<bool, QString>( false, "File " + sourceFile + " has wrong format. Unable to process this data." );

    QString archTag;
    dataStream >> archTag;

    if (archTag != archiveTag) {
        return QPair<bool, QString>( false, "File " + sourceFile + " has was expected for '" + archTag + "', but expected '" + archTag + "'" );
    }

    // Cleaning up destination dir first

    QDir dir(destinationFolder);
    if (!dir.removeRecursively() ) {
        return QPair<bool, QString>( false, "Unable to clean up destination directory " + destinationFolder );
    }
    if (!dir.mkpath(destinationFolder)) {//could not create folder
        return QPair<bool, QString>( false, "Unable to create destination directory " + destinationFolder );
    }

    while (!dataStream.atEnd()) {
        if (callProcessEvents)
            QCoreApplication::processEvents();

        int dataId;
        dataStream >> dataId;

        if (dataId == ARCH_DIR_VER) {
            QString dirName;
            dataStream >> dirName;

            if (!dirName.isEmpty()) {
                if (!dir.mkpath(destinationFolder + "/" + dirName) )
                    return QPair<bool, QString>( false, "Unable to create directory " + destinationFolder );
            }
        }
        else if ( dataId == ARCH_FILE_VER ) {
            QString fileName;
            QByteArray data;

            dataStream >> fileName >> data;

            QFile outFile(destinationFolder + "/" + fileName);
            if (!outFile.open(QIODevice::WriteOnly)) {
                file.close();
                return QPair<bool, QString>( false, "Unable to create resulting file " + QFileInfo(file).absoluteFilePath() );
            }
            outFile.write(qUncompress(data));
            outFile.close();
        }
        else {
            return QPair<bool, QString>( false, "File " + sourceFile + " corrupted or has wrong format. Unable to finish extraction." );
        }

    }

    return QPair<bool, QString>( true,"");
}

}