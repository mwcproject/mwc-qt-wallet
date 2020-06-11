// Copyright 2020 The MWC Developers
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

#ifndef CLIPBOARD_B_H
#define CLIPBOARD_B_H

#include <QObject>

class ClipboardProxy : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ dataText WRITE setDataText NOTIFY dataChanged)
    Q_PROPERTY(QString selectionText READ selectionText WRITE setSelectionText NOTIFY selectionChanged)
public:
    explicit ClipboardProxy(QObject *parent = 0);

    void setDataText(const QString &text);
    QString dataText() const;

    void setSelectionText(const QString &text);
    QString selectionText() const;

signals:
    void dataChanged();
    void selectionChanged();
};

#endif // CLIPBOARD_B_H
