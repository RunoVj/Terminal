#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QtSerialPort/QSerialPort>

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

private slots:
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();

    void handleError(QSerialPort::SerialPortError error);

    void on_actionQuit_triggered();
    void on_actionConfigure_triggered();

    void on_verticalSliderVelocity_valueChanged(int value);

    void on_verticalSliderFrequency_valueChanged(int value);

    void on_verticalSliderPosition_valueChanged(int value);

private:
    void initActionsConnections();
    void showStatusMessage(const QString &message);

    Ui::MainWindow *ui;

    QLabel *status;
    SettingsDialog *settings;
    QSerialPort *serial;
};

#endif // MAINWINDOW_H
