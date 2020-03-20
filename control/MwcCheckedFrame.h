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

#ifndef MWC_QT_WALLET_MWCCHECKEDFRAME_H
#define MWC_QT_WALLET_MWCCHECKEDFRAME_H

#include "MwcFrameWithBorder.h"

namespace control {

class MwcCheckedFrame : public MwcFrameWithBorder {
    Q_OBJECT
public:
    MwcCheckedFrame( QWidget* parent, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~MwcCheckedFrame() override;

    void setId(int id) { ctrlId = id; }

    void setChecked(bool checked);

    bool isChecked() const {return checked;}

private: signals:
    void onChecked( int id );

protected:
    virtual void focusInEvent(QFocusEvent * event) override;
    virtual void focusOutEvent(QFocusEvent * event) override;
    virtual void mousePressEvent(QMouseEvent * event) override;
    virtual void keyPressEvent(QKeyEvent * event) override;

    virtual void leaveEvent(QEvent * event) override;
    virtual void mouseMoveEvent(QMouseEvent * event) override;

private:
    void updateCheckState();

private:
    int ctrlId = -1;
    bool checked = false;
    bool hover = false;
};

}

#endif //MWC_QT_WALLET_MWCCHECKEDFRAME_H
