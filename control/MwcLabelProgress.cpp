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

#include "MwcLabelProgress.h"
#include <QMovie>

namespace control {

MwcLabelProgress::MwcLabelProgress(QWidget *parent, Qt::WindowFlags f) :
        QLabel(parent,f)
{
}

MwcLabelProgress::MwcLabelProgress(const QString &text, QWidget *parent, Qt::WindowFlags f) :
        QLabel(text,parent, f)
{
}

MwcLabelProgress::~MwcLabelProgress() {}

void MwcLabelProgress::initLoader(bool visible) {
    QMovie *movie = new QMovie(":/img/loading.gif", QByteArray(), this);
    setMovie(movie);
    setScaledContents(true);
    movie->start();

    if (visible)
        show();
    else
        hide();
}

}
