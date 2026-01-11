#include "paneltitle.h"
#include "ui_paneltitle.h"

namespace core {

static QString currentTitle;
static PanelTitle * currentWnd = nullptr;

void setCurrentTitle(const QString & title) {
    if (title.isEmpty())
        return;

    currentTitle = title;
    if (currentWnd)
        currentWnd->setTitle(currentTitle);
}


PanelTitle::PanelTitle(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::PanelTitle) {
    ui->setupUi(this);

    setTitle(currentTitle);
    currentWnd = this;
}

PanelTitle::~PanelTitle() {
    currentWnd = nullptr;
    delete ui;
}

void PanelTitle::setTitle(const QString & title) {
    ui->title->setText(title);
}


}
