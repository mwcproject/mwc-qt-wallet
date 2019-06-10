#include "core/mwctoolbar.h"
#include "ui_mwctoolbar.h"
#include <QPainter>

namespace core {

MwcToolbar::MwcToolbar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MwcToolbar)
{
    ui->setupUi(this);

    //setAutoFillBackground(true);
    //setStyleSheet("background-color: #6F00D6;");
}

MwcToolbar::~MwcToolbar()
{
    delete ui;
}

void MwcToolbar::paintEvent(QPaintEvent *)
 {
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

}
