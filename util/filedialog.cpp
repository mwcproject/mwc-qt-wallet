// Copyright 2021 The MWC Developers
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

#include "filedialog.h"
#include "../bridge/config_b.h"
#include "../util/stringutils.h"
#include "../core/WndManager.h"
#include <QFileInfo>

namespace util {

// Call open file dialog
// Note parentWnd can be null, so clean up  the objects
QString getSaveFileName(QString title, QString callerId, QString extentionsDlg, QString extentionFile) {
    bridge::Config *config = new bridge::Config(nullptr);

    QString fileName = core::getWndManager()->getSaveFileName( title,
                                                               callerId.isEmpty() ? "" : config->getPathFor(callerId),
                                                               extentionsDlg);

    if (fileName.length() == 0) {
        delete config;
        return "";
    }

    auto fileOk = util::validateMwc713Str(fileName);
    if (!fileOk.first) {
        core::getWndManager()->messageTextDlg("File Path",
                                         "This file path is not acceptable.\n" + fileOk.second);
        delete config;
        return "";
    }

    if (!extentionFile.isEmpty()) {
        if (!fileName.endsWith(extentionFile))
            fileName += extentionFile;
    }

    if (!callerId.isEmpty())
        config->updatePathFor(callerId, QFileInfo(fileName).absolutePath());

    return fileName;
}

// Call open file dialog
// Note parentWnd can be null, so clean up  the objects
QString getOpenFileName(QString title, QString callerId, QString extentionsDlg) {
    bridge::Config *config = new bridge::Config(nullptr);

    QString fileName = core::getWndManager()->getOpenFileName( title,
                                                               callerId.isEmpty() ? "" : config->getPathFor(callerId),
                                                               extentionsDlg);

    if (fileName.length() == 0) {
        delete config;
        return "";
    }

    auto fileOk = util::validateMwc713Str(fileName);
    if (!fileOk.first) {
        core::getWndManager()->messageTextDlg("File Path",
                                              "This file path is not acceptable.\n" + fileOk.second);
        delete config;
        return "";
    }

    if (!callerId.isEmpty())
        config->updatePathFor(callerId, QFileInfo(fileName).absolutePath());

    return fileName;
}


}