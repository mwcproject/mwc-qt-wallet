#include "Files.h"
#include <QFile>
#include <QTextStream>

namespace util {

QStringList readTextFile( QString fileName, std::function<void()> openOpenErrorCallback ) {
    QFile file(fileName);
    QStringList res;
    if (!file.open(QFile::ReadOnly)) {

        openOpenErrorCallback();
        return res;
    }

    QTextStream in(&file);

    while(!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        res << line;
    }
    file.close();
    return res;
}

// Write text lines into the file
bool writeTextFile(QString fileName, QStringList lines ) {

    QFile file(fileName);
    QStringList res;
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    QTextStream out(&file);

    for (const auto & ln : lines) {
        out << ln << endl;
    }

    file.close();
    return true;
}


}