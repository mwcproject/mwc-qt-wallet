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

#include "MwcLineEdit.h"

namespace control {

MwcLineEdit::MwcLineEdit(QWidget *parent) :
    QLineEdit(parent)
{}

MwcLineEdit::MwcLineEdit(const QString & t, QWidget *parent ) :
    QLineEdit(t, parent)
{}

MwcLineEdit::~MwcLineEdit() {}


MwcLineEditNormal::MwcLineEditNormal(QWidget *parent) :
    MwcLineEdit(parent)
{}

MwcLineEditNormal::MwcLineEditNormal(const QString & t, QWidget *parent ) :
        MwcLineEdit(t, parent)
{}

MwcLineEditNormal::~MwcLineEditNormal()
{}

}
