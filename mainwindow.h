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

    QLabel *_status;
    SettingsDialog *settings;
    QSerialPort *serial;

    uint8_t _cur_mes_type;
    uint8_t _next_mes_type;

    uint8_t _pwm_duty;
    uint8_t _position;
    uint8_t _period;
    bool _position_setting;

    QVector<QByteArray> _firmware;

private slots:
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();


    void handleError(QSerialPort::SerialPortError error);

    void on_actionQuit_triggered();
    void on_verticalSliderVelocity_valueChanged(int value);
    void on_verticalSliderFrequency_valueChanged(int value);
    void on_verticalSliderPosition_valueChanged(int value);

    void request();
    void config_request();
    void open_hex();

    void flash();

};

#endif // MAINWINDOW_H
