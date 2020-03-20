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

#ifndef GUI_WALLET_MWCLABEL_H
#define GUI_WALLET_MWCLABEL_H

#include <QLabel>

namespace control {

class MwcLabelTiny : public QLabel {
    Q_OBJECT
public:
    explicit MwcLabelTiny(QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    explicit MwcLabelTiny(const QString &text, QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    virtual ~MwcLabelTiny() override;
};

class MwcLabelSmall : public QLabel {
    Q_OBJECT
public:
    explicit MwcLabelSmall(QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    explicit MwcLabelSmall(const QString &text, QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    virtual ~MwcLabelSmall() override;
};

class MwcLabelNormal : public QLabel {
Q_OBJECT
public:
    explicit MwcLabelNormal(QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    explicit MwcLabelNormal(const QString &text, QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    virtual ~MwcLabelNormal() override;
};

class MwcLabelLarge : public QLabel {
Q_OBJECT
public:
    explicit MwcLabelLarge(QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    explicit MwcLabelLarge(const QString &text, QWidget *parent=Q_NULLPTR, Qt::WindowFlags f=Qt::WindowFlags());
    virtual ~MwcLabelLarge() override;
};

}

#endif //GUI_WALLET_MWCLABEL_H
