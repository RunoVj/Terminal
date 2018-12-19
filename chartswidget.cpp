#include "chartswidget.h"
#include "ui_chartswidget.h"

ChartsWidget::ChartsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChartsWidget),
    _chart_ptr(new QChart),
    _chart_view_ptr(new QChartView),
    _x_axis_ptr(new QValueAxis),
    _y_axis_ptr(new QValueAxis),
    _line_series_ptr(new QLineSeries)
{
    ui->setupUi(this);
    _chart_ptr->setAnimationOptions(QChart::AllAnimations);
    _line_series_ptr->append(3, 4);
    _line_series_ptr->append(4, 6);

    _chart_ptr->addSeries(_line_series_ptr);

    _x_axis_ptr->setRange(0, 100);
    _y_axis_ptr->setRange(-10, 10);
    _x_axis_ptr->setTickCount(11);
    _x_axis_ptr->setTitleText("Aaaaa, a");

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

void ChartsWidget::start_timer(bool start)
{

}
void ChartsWidget::add_current_data(double value)
{

}
void ChartsWidget::add_pwm_data(int8_t value)
{

}
void ChartsWidget::add_speed_data(uint16_t value)
{

}
void ChartsWidget::add_force_data(uint16_t value)
{

}
