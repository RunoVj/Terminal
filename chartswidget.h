#ifndef CHARTSWIDGET_H
#define CHARTSWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <QSplineSeries>
#include <QDebug>

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
    QChart *_chart_ptr;
    QChartView *_chart_view_ptr;
    QValueAxis *_x_axis_ptr;
    QValueAxis *_y_axis_ptr;
    QLineSeries *_line_series_ptr;

signals:

public slots:
    void start_timer(bool start);
    void add_current_data(double value);
    void add_pwm_data(int8_t value);
    void add_speed_data(uint16_t value);
    void add_force_data(uint16_t value);

private slots:

};

#endif // CHARTSWIDGET_H
