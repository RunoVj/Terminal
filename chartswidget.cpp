#include "chartswidget.h"
#include "ui_chartswidget.h"

ChartsWidget::ChartsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChartsWidget)
{
    ui->setupUi(this);
}

ChartsWidget::~ChartsWidget()
{
    delete ui;
}
