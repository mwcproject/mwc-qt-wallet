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

#include "MwcNodeConfig.h"
#include "../util/ioutils.h"
#include <QDir>
#include "../util/Files.h"
#include "../util/ConfigReader.h"
#include <QtGlobal>
#include <QTime>
#include "../core/global.h"
#include <QCoreApplication>
#include "../core/WndManager.h"
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRandomGenerator>
#endif

namespace node {

QPair<bool,QString> getMwcNodePath( const QString & nodeDataPath,  const QString & network) {
    QString nwPath = network.toLower().contains("main") ? "main" : "floo";
    QPair<bool,QString> appPath = ioutils::getAppDataPath(nodeDataPath);
    if (!appPath.first)
        return appPath;
    return QPair<bool,QString>(true, appPath.second + QDir::separator() + nwPath + QDir::separator());
}

}
