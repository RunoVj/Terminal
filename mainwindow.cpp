#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

#include <QLabel>
#include <QMessageBox>
#include <QtSerialPort/QSerialPort>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serial = new QSerialPort(this);
    settings = new SettingsDialog;
    send_timer = new QTimer(this);

    position_setting = 0;
    position = 0;
    pwm_duty = ui->verticalSliderVelocity->value();
    period = ui->verticalSliderFrequency->value();


    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);

    status = new QLabel;

    ui->statusbar->addWidget(status);

    initActionsConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete settings;
}

void MainWindow::openSerialPort()
{
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    if (serial->open(QIODevice::ReadWrite)) {
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate)
                          .arg(p.stringDataBits).arg(p.stringParity)
                          .arg(p.stringStopBits).arg(p.stringFlowControl));
        send_timer->start(REQUEST_DELAY);
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        showStatusMessage(tr("Open error"));
    }
}

void MainWindow::closeSerialPort()
{
    if (serial->isOpen())
        serial->close();
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
    send_timer->stop();
}

void MainWindow::writeData(const QByteArray &data)
{
    serial->write(data);
}

void MainWindow::request()
{    
    // fill request structure
    Request req;
    req.AA = 0xAA;
    req.type = 0x01;
    req.address = 0x01;
    req.update_base_vector = ui->radioButtonCorrection->isChecked();
    req.position_setting = ui->radioButtonPosition->isChecked();
    req.angle = ui->dialAngle->value();
    req.velocity = ui->verticalSliderVelocity->value();
    req.frequency = ui->verticalSliderFrequency->value();

    req.outrunning_angle = ui->dialOutrunningAngle->value();
    req.update_base_vector = ui->radioButtonCorrection->isChecked();

    // move to QByteArray
    QByteArray msg_buf;
    QDataStream stream(&msg_buf, QIODevice::Append);
    stream << req;

    uint8_t crc = 0;

    for (int i = 0; i < REQUEST_SIZE - 1; i++){
        crc ^= msg_buf[i];
    }

    stream << crc;

    ui->plainTextEditTransmit->appendPlainText(msg_buf.toHex().toUpper());

    serial->clear(QSerialPort::Input);

    writeData(msg_buf);

    readData();
}


void MainWindow::readData()
{    
    qDebug() << "read data";
    QByteArray data;
    if (serial->waitForReadyRead(RESPONSE_DELAY)){
        data = serial->readAll();
        qDebug() << "read bytes - " << data.toHex();
    }
    ui->plainTextEditReceive->appendPlainText(data.toHex().toUpper());

    struct Response resp;
    QDataStream stream(&data, QIODevice::ReadOnly);

    stream >> resp;


    ui->lineEditAddress->setText(QString::number(resp.address));
    ui->lineEditCurrent->setText(QString::number(resp.current));

    ui->lineEditState->setText(QString::number(resp.state));

    ui->radioButtonSensorA->setDown(resp.position_code & 0b00000001);
    ui->radioButtonSensorB->setDown(resp.position_code & 0b00000010);
    ui->radioButtonSensorC->setDown(resp.position_code & 0b00000100);


}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"),
                              serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, &QAction::triggered,
            this, &MainWindow::openSerialPort);
    connect(ui->actionDisconnect, &QAction::triggered,
            this, &MainWindow::closeSerialPort);
    connect(ui->actionQuit, &QAction::triggered,
            this, &MainWindow::close);
    connect(ui->actionConfigure, &QAction::triggered,
            settings, &SettingsDialog::show);
    connect(ui->actionClear, &QAction::triggered,
            [this](void){ui->plainTextEditReceive->clear();});
    connect(ui->actionClear, &QAction::triggered,
            [this](void){ui->plainTextEditTransmit->clear();});

    connect(ui->verticalSliderFrequency, &QSlider::valueChanged,
            ui->spinBoxFrequency, &QSpinBox::setValue);
    connect(ui->verticalSliderVelocity, &QSlider::valueChanged,
            [this](int value){ui->spinBoxVelocity->setValue(value);});
    connect(ui->dialAngle, &QDial::valueChanged,
            ui->spinBoxPosition, &QSpinBox::setValue);
    connect(ui->dialOutrunningAngle, &QDial::valueChanged,
            ui->spinBoxOutrunningAngle, &QSpinBox::setValue);

    connect(ui->spinBoxFrequency,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            ui->verticalSliderFrequency, &QSlider::setValue);
    connect(ui->spinBoxVelocity,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            [this](int value){ui->verticalSliderVelocity->setValue(value);});
    connect(ui->spinBoxPosition,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            [this](int value)
    {
        ui->dialAngle->setValue(value);
        position = value;
    });
    connect(ui->spinBoxOutrunningAngle,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            ui->dialOutrunningAngle, &QDial::setValue);

    connect(send_timer, &QTimer::timeout, this, &MainWindow::request);
    connect(serial,
            static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>
            (&QSerialPort::error), this, &MainWindow::handleError);

    connect(ui->radioButtonPosition, &QRadioButton::toggled,
            [this](bool permission)
    {
        position_setting = permission;
        ui->dialAngle->setEnabled(permission);
        ui->spinBoxPosition->setEnabled(permission);

    });
}

void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}

void MainWindow::on_verticalSliderVelocity_valueChanged(int value)
{
    pwm_duty = value;
    qDebug() << "pwm duty = " << pwm_duty << "\n";
}

void MainWindow::on_verticalSliderPosition_valueChanged(int value)
{
    position = value;
    qDebug() << "position = " << position << "\n";
}

void MainWindow::on_verticalSliderFrequency_valueChanged(int value)
{
    period = value;
    qDebug() << "period = " << period << "\n";
}
