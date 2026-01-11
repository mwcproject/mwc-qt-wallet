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

#include "QrCodeWidget.h"
#include "../bridge/util_b.h"
#include "../util/QrCode.h"

#include <QPainter>
#include <QPaintEvent>
#include <QSvgRenderer>

using namespace qrcodegen;

namespace control {

QrCodeWidget::QrCodeWidget(QWidget *parent) : QWidget(parent) {
    util = new bridge::Util(this);
    show();
}

QrCodeWidget::~QrCodeWidget() {
}

void QrCodeWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QPainter painter(this);

    int w = width();
    int h = height();

    painter.fillRect(0,0,w,h, QColor(255,255,255,255));

    QSvgRenderer renderer( svgPath.toUtf8(), this );
    renderer.render(&painter, QRectF(0.0, 0.0, qreal(w) , qreal(h)) );
}

void QrCodeWidget::setContent(QString text) {
    if (text.isEmpty())
        return;

    encodedData = text;
    QVector<QString> res = util->generateQrCode(text);
    Q_ASSERT(res.size()==2);
    qrSize = res[0].toInt();
    Q_ASSERT(qrSize>0);
    svgPath = res[1];
    QWidget::repaint();
}

// Generate the image only.
QImage QrCodeWidget::generateQrImage() {
    if (qrSize<=0) {
        Q_ASSERT(false);
        return QImage(1, 1, QImage::Format_ARGB32);
    }

    int k = 5;
    while (qrSize * k < 300 )
        k++;

    // Prepare a QImage with desired characteritisc
    QImage image(qrSize * k, qrSize * k, QImage::Format_ARGB32);
    image.fill(0x00000000); // Transparent background

    QSvgRenderer renderer( svgPath.toUtf8(), this );

    // Get QPainter that paints to the image
    QPainter painter(&image);
    renderer.render(&painter);

    return image;
}

QString QrCodeWidget::generateQrImage(QString fileName) {
    QImage image = generateQrImage();

    // Save, image format based on file extension
    if (! image.save(fileName) )
        return "Unable to save image to the file " + fileName;

    // We are good, all done
    return "";
}


}
