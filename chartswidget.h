#ifndef CHARTSWIDGET_H
#define CHARTSWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <QLineSeries>
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
    QSplineSeries  *_line_series_ptr;

    QTime *_time_ptr;
    QList<QPointF> pwm_list;

signals:

public slots:
    void enable_plotting(bool enable);
    void add_current_data(double value);
    void add_pwm_data(int8_t value);
    void add_speed_data(uint16_t value);
    void add_force_data(uint16_t value);

private slots:

};

#endif // CHARTSWIDGET_H
