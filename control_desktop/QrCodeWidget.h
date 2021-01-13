// Copyright 2021 The MWC Developers
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

#ifndef QRCODEWIDGET_H
#define QRCODEWIDGET_H

#include <QColor>
#include <QWidget>
#include <QByteArray>

class QPaintEvent;

namespace bridge {
class Util;
}

namespace control {

class QrCodeWidget : public QWidget {
Q_OBJECT
public:
    explicit QrCodeWidget(QWidget *parent = nullptr);
    ~QrCodeWidget();

    void setContent(QString text);
    // generate image and save it into the path. Return empty String on success. Otherwise String has error message
    // Image fileName must have valid extention. Expected png.
    QString generateQrImage(QString fileName);

    // Generate the image only.
    QImage generateQrImage();

protected:
    void paintEvent(QPaintEvent *event);

private:
    bridge::Util * util;
    QString encodedData;
    int     qrSize = 0;
    QString svgPath;
};

}

#endif // QRCODEWIDGET_H