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

#include "MwcPushButton.h"

namespace control {

// --------------  MwcPushButton  -------------------

MwcPushButton::MwcPushButton(QWidget *parent) :
    QPushButton(parent)
{}

MwcPushButton::MwcPushButton(const QString &text, QWidget *parent ) :
    QPushButton(text,parent)
{}

MwcPushButton::MwcPushButton(const QIcon &icon, const QString &text, QWidget *parent) :
    QPushButton(icon,text,parent)
{}

MwcPushButton::~MwcPushButton()
{}

// --------------  MwcPushButtonNormal  -------------------

MwcPushButtonNormal::MwcPushButtonNormal(QWidget *parent) :
        MwcPushButton(parent)
{}

MwcPushButtonNormal::MwcPushButtonNormal(const QString &text, QWidget *parent ) :
    MwcPushButton(text,parent)
{}

MwcPushButtonNormal::MwcPushButtonNormal(const QIcon &icon, const QString &text, QWidget *parent) :
    MwcPushButton(icon,text,parent)
{}

MwcPushButtonNormal::~MwcPushButtonNormal()
{}

void MwcPushButtonNormal::focusInEvent(QFocusEvent * event) {
    MwcPushButton::focusInEvent(event);
    repaint(); // update by some reasons doesn't work. Seems like qt bug
}

void MwcPushButtonNormal::focusOutEvent(QFocusEvent * event) {
    MwcPushButton::focusOutEvent(event);
    repaint(); // update by some reasons doesn't work. Seems like qt bug
}

// -----------------   MwcPushButtonSmall  -----------------------

MwcPushButtonSmall::MwcPushButtonSmall(QWidget *parent) :
        MwcPushButton(parent)
{}

MwcPushButtonSmall::MwcPushButtonSmall(const QString &text, QWidget *parent ) :
        MwcPushButton(text,parent)
{}

MwcPushButtonSmall::MwcPushButtonSmall(const QIcon &icon, const QString &text, QWidget *parent) :
        MwcPushButton(icon,text,parent)
{}

MwcPushButtonSmall::~MwcPushButtonSmall()
{}


// -----------------   MwcPushButtonTiny  -----------------------

MwcPushButtonTiny::MwcPushButtonTiny(QWidget *parent) :
        MwcPushButton(parent)
{}

MwcPushButtonTiny::MwcPushButtonTiny(const QString &text, QWidget *parent ) :
        MwcPushButton(text,parent)
{}

MwcPushButtonTiny::MwcPushButtonTiny(const QIcon &icon, const QString &text, QWidget *parent) :
        MwcPushButton(icon,text,parent)
{}

MwcPushButtonTiny::~MwcPushButtonTiny()
{}


// -----------------   MwcPushButtonTinyUnderline  -----------------------

MwcPushButtonTinyUnderline::MwcPushButtonTinyUnderline(QWidget *parent) :
        MwcPushButton(parent)
{}

MwcPushButtonTinyUnderline::MwcPushButtonTinyUnderline(const QString &text, QWidget *parent ) :
        MwcPushButton(text,parent)
{}

MwcPushButtonTinyUnderline::MwcPushButtonTinyUnderline(const QIcon &icon, const QString &text, QWidget *parent) :
        MwcPushButton(icon,text,parent)
{}

MwcPushButtonTinyUnderline::~MwcPushButtonTinyUnderline()
{}

// -----------------   MwcPushButtonRound  -----------------------


MwcPushButtonRound::MwcPushButtonRound(QWidget *parent) :
    MwcPushButton(parent)
{}
MwcPushButtonRound::MwcPushButtonRound(const QString &text, QWidget *parent) :
    MwcPushButton(text,parent)
{}

MwcPushButtonRound::MwcPushButtonRound(const QIcon &icon, const QString &text, QWidget *parent)  :
    MwcPushButton(icon,text,parent)
{}

MwcPushButtonRound::~MwcPushButtonRound() {}

// ------------------  MwcPushButtonStatusLine  ------------------------

MwcPushButtonStatusLine::MwcPushButtonStatusLine(QWidget *parent) :
        MwcPushButton(parent)
{}
MwcPushButtonStatusLine::MwcPushButtonStatusLine(const QString &text, QWidget *parent) :
        MwcPushButton(text,parent)
{}

MwcPushButtonStatusLine::MwcPushButtonStatusLine(const QIcon &icon, const QString &text, QWidget *parent)  :
        MwcPushButton(icon,text,parent)
{}

MwcPushButtonStatusLine::~MwcPushButtonStatusLine() {}


}
