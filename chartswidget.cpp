#include "chartswidget.h"
#include "ui_chartswidget.h"

ChartsWidget::ChartsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChartsWidget),
    _chart_ptr(new QChart),
    _x_axis_ptr(new QValueAxis),
    _y_axis_ptr(new QValueAxis),
    _line_series_ptr(new QSplineSeries )
{
    ui->setupUi(this);

    _chart_ptr->setAnimationOptions(QChart::AllAnimations);

    *_line_series_ptr << QPointF(0, 0);
    _chart_ptr->addSeries(_line_series_ptr);

    _x_axis_ptr->setRange(0, 20);
    _y_axis_ptr->setRange(-128, 128);

    _x_axis_ptr->setTickCount(11);
    _x_axis_ptr->setTitleText("Time, ms");

    _y_axis_ptr->setTickCount(17);
    _y_axis_ptr->setTitleText("PWM duty");

    _chart_view_ptr = new QChartView(_chart_ptr);

    _chart_view_ptr->chart()->setAxisX(_x_axis_ptr, _line_series_ptr);
    _chart_view_ptr->chart()->setAxisY(_y_axis_ptr, _line_series_ptr);

    _chart_view_ptr->setRenderHint(QPainter::Antialiasing);

    ui->chartLayout->addWidget(_chart_view_ptr);
}

ChartsWidget::~ChartsWidget()
{
    delete ui;
    delete _chart_ptr;
    delete _chart_view_ptr;
    delete _x_axis_ptr;
    delete _y_axis_ptr;
    delete _line_series_ptr;
}

void ChartsWidget::enable_plotting(bool enable)
{
    if (enable) {
        _time_ptr = new QTime;
        _time_ptr->start();
    }
    else {
        delete _time_ptr;
    }
}

void ChartsWidget::add_current_data(double value)
{

}

void ChartsWidget::add_pwm_data(int8_t value)
{
    static int prev_value, dt, count, dt_sum;
    dt = _time_ptr->elapsed()/10 - prev_value;
    prev_value = _time_ptr->elapsed()/10;
    qDebug() << "Added new pwm data: " << value
             << "\nTime: " << prev_value
             << "\ndt: " << dt;

    pwm_list.push_back(QPointF(prev_value, value));
    ++count;
    dt_sum += dt;
    if (count == 10) {
        count = 0;
        dt_sum = 0;
        _line_series_ptr->clear();
        *_line_series_ptr << pwm_list;
        pwm_list.clear();
        _chart_ptr->scroll(dt_sum, 0);
        _chart_view_ptr->update();
    }
}

void ChartsWidget::add_speed_data(uint16_t value)
{

}

void ChartsWidget::add_force_data(uint16_t value)
{

}
