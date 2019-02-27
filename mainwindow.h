#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>

#include "messages.h"

QT_BEGIN_NAMESPACE

class QLabel;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class Console;
class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void initActionsConnections();
    void showStatusMessage(const QString &message);

    Ui::MainWindow *ui;
    QTimer *_send_timer;
    QTimer *_stress_test_timer;
    int stress_test_numb;

    QLabel *_status;
    SettingsDialog *settings;
    QSerialPort *serial;

    uint8_t _cur_mes_type;
    uint8_t _next_mes_type;

    uint16_t _firm_req_index;

    int8_t _velocity;
    uint8_t _position;
    uint8_t _period;
    bool _position_setting;

    QVector<QByteArray> _firmware;

private slots:
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();

    void disable_current_thresholds(bool enable_thresholds);

    void handleError(QSerialPort::SerialPortError error);

    void on_actionQuit_triggered();
    void on_verticalSliderVelocity_valueChanged(int value);
    void on_verticalSliderFrequency_valueChanged(int value);
    void on_verticalSliderPosition_valueChanged(int value);

    void stress_test_timer_timeout(void);
    void allow_correction(bool enable);
    void update_correction(int value);

    void request();
    void config_request();
    void open_hex();

    void flash();
    void stress_test(bool enable);
};

#endif // MAINWINDOW_H
