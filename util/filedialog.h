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

#ifndef MWC_QT_WALLET_FILEDIALOG_H
#define MWC_QT_WALLET_FILEDIALOG_H

#include <QString>

class QWidget;

namespace util {

#ifdef WALLET_DESKTOP

// Call open save dialog with all checks
QString getSaveFileName(QString title, QString callerId, QString extentionsDlg, QString extentionFile);

// Call open file dialog with all checks
QString getOpenFileName(QString title, QString callerId, QString extentionsDlg);

#endif

}

#endif //MWC_QT_WALLET_FILEDIALOG_H
