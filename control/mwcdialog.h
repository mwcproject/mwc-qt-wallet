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

#ifndef MWCDIALOG_H
#define MWCDIALOG_H

#include <QDialog>

namespace control {

class MwcDialog : public QDialog
{
    Q_OBJECT
public:
    MwcDialog( QWidget * parent );
    virtual ~MwcDialog() override;

protected:
    // We don't have caption. So let's be movable without it
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private:
    QPoint cursor;
};

}


#endif // MWCDIALOG_H
