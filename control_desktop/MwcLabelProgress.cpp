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

static QVector<MwcLabelProgress*> processWndsToHide;

// Message box normally error and it should cancel the progress. It is a shortcut that help to avoid propagate back response signals
void onMessageBoxShown() {
    for (auto wnd : processWndsToHide)
        wnd->hide();
}

MwcLabelProgress::MwcLabelProgress(QWidget *parent, Qt::WindowFlags f, bool hideWithMessageBox) :
        QLabel(parent,f)
{
    if (hideWithMessageBox)
    processWndsToHide.push_back(this);
}

MwcLabelProgress::MwcLabelProgress(const QString &text, QWidget *parent, Qt::WindowFlags f, bool hideWithMessageBox) :
        QLabel(text,parent, f)
{
    if (hideWithMessageBox)
        processWndsToHide.push_back(this);
}

MwcLabelProgress::~MwcLabelProgress() {
    processWndsToHide.removeAll(this);
}

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
