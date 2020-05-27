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

#ifndef MWC_QT_WALLET_MWCPLAINEDITLOGS_H
#define MWC_QT_WALLET_MWCPLAINEDITLOGS_H

#include <QPlainTextEdit>

namespace control {

class MwcPlainEditLogs : public QPlainTextEdit {
Q_OBJECT
public:
    explicit MwcPlainEditLogs(QWidget *parent = Q_NULLPTR);
    explicit MwcPlainEditLogs(const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPlainEditLogs() override;
};

class MwcPlainEditDlg : public QPlainTextEdit {
Q_OBJECT
public:
    explicit MwcPlainEditDlg(QWidget *parent = Q_NULLPTR);
    explicit MwcPlainEditDlg(const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPlainEditDlg() override;
};

class MwcTextEditDlg : public QTextEdit {
Q_OBJECT
public:
    explicit MwcTextEditDlg(QWidget *parent = Q_NULLPTR);
    explicit MwcTextEditDlg(const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcTextEditDlg() override;
};

}

#endif //MWC_QT_WALLET_MWCPLAINEDITLOGS_H
