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

#include "MwcLabel.h"

namespace control {

MwcLabelTiny::MwcLabelTiny(QWidget *parent, Qt::WindowFlags f) :
   QLabel(parent,f)
{}

MwcLabelTiny::MwcLabelTiny(const QString &text, QWidget *parent, Qt::WindowFlags f) :
    QLabel(text,parent, f)
{}

MwcLabelTiny::~MwcLabelTiny() {}


MwcLabelSmall::MwcLabelSmall(QWidget *parent, Qt::WindowFlags f) :
    QLabel(parent,f)
{}

MwcLabelSmall::MwcLabelSmall(const QString &text, QWidget *parent, Qt::WindowFlags f) :
    QLabel(text,parent, f)
{}

MwcLabelSmall::~MwcLabelSmall() {}


MwcLabelNormal::MwcLabelNormal(QWidget *parent, Qt::WindowFlags f) :
    QLabel(parent,f)
{}

MwcLabelNormal::MwcLabelNormal(const QString &text, QWidget *parent, Qt::WindowFlags f) :
        QLabel(text,parent, f)
{}

MwcLabelNormal::~MwcLabelNormal()
{}

MwcLabelLarge::MwcLabelLarge(QWidget *parent, Qt::WindowFlags f) :
    QLabel(parent,f)
{}

MwcLabelLarge::MwcLabelLarge(const QString &text, QWidget *parent, Qt::WindowFlags f) :
    QLabel(text,parent, f)
{}

MwcLabelLarge::~MwcLabelLarge()
{}

}
