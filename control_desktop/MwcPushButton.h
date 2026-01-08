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

#ifndef GUI_WALLET_MWCPUSHBUTTON_H
#define GUI_WALLET_MWCPUSHBUTTON_H

#include <QPushButton>

namespace control {

class MwcPushButton : public QPushButton {
Q_OBJECT
public:
    explicit MwcPushButton(QWidget *parent = Q_NULLPTR);
    explicit MwcPushButton(const QString &text, QWidget *parent = Q_NULLPTR);
    MwcPushButton(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPushButton() override;
};

class MwcPushButtonNormal : public MwcPushButton {
Q_OBJECT
public:
    explicit MwcPushButtonNormal(QWidget *parent = Q_NULLPTR);
    explicit MwcPushButtonNormal(const QString &text, QWidget *parent = Q_NULLPTR);
    MwcPushButtonNormal(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPushButtonNormal() override;
protected:
    // to handle focus
    virtual void focusInEvent(QFocusEvent * event) override;
    virtual void focusOutEvent(QFocusEvent * event) override;
};

class MwcPushButtonSmall : public MwcPushButton {
Q_OBJECT
public:
    explicit MwcPushButtonSmall(QWidget *parent = Q_NULLPTR);
    explicit MwcPushButtonSmall(const QString &text, QWidget *parent = Q_NULLPTR);
    MwcPushButtonSmall(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPushButtonSmall() override;
};

class MwcPushButtonTiny : public MwcPushButton {
Q_OBJECT
public:
    explicit MwcPushButtonTiny(QWidget *parent = Q_NULLPTR);
    explicit MwcPushButtonTiny(const QString &text, QWidget *parent = Q_NULLPTR);
    MwcPushButtonTiny(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPushButtonTiny() override;
};

class MwcPushButtonTinyUnderline : public MwcPushButton {
Q_OBJECT
public:
    explicit MwcPushButtonTinyUnderline(QWidget *parent = Q_NULLPTR);
    explicit MwcPushButtonTinyUnderline(const QString &text, QWidget *parent = Q_NULLPTR);
    MwcPushButtonTinyUnderline(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPushButtonTinyUnderline() override;
};


class MwcPushButtonRound : public MwcPushButton {
Q_OBJECT
public:
    explicit MwcPushButtonRound(QWidget *parent = Q_NULLPTR);
    explicit MwcPushButtonRound(const QString &text, QWidget *parent = Q_NULLPTR);
    MwcPushButtonRound(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPushButtonRound() override;
};


class MwcPushButtonStatusLine : public MwcPushButton {
Q_OBJECT
public:
    explicit MwcPushButtonStatusLine(QWidget *parent = Q_NULLPTR);
    explicit MwcPushButtonStatusLine(const QString &text, QWidget *parent = Q_NULLPTR);
    MwcPushButtonStatusLine(const QIcon &icon, const QString &text, QWidget *parent = Q_NULLPTR);
    virtual ~MwcPushButtonStatusLine() override;
};


}

#endif //GUI_WALLET_MWCPUSHBUTTON_H
