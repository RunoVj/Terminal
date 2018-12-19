#ifndef CHARTSWIDGET_H
#define CHARTSWIDGET_H

#include <QWidget>

namespace Ui {
class ChartsWidget;
}

class ChartsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChartsWidget(QWidget *parent = nullptr);
    ~ChartsWidget();

private:
    Ui::ChartsWidget *ui;
};

#endif // CHARTSWIDGET_H
