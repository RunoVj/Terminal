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
    QByteArray msg_buf;
    msg_buf[VMA_DEV_REQUEST_AA1] = 0xAA;
    msg_buf[VMA_DEV_REQUEST_AA2] = 0x01;
    msg_buf[VMA_DEV_REQUEST_ADDRESS] = position_setting;
    msg_buf[VMA_DEV_REQUEST_SETTING] = position;
    msg_buf[VMA_DEV_REQUEST_VELOCITY1] = pwm_duty;
    msg_buf[VMA_DEV_REQUEST_VELOCITY2] = period;

    uint8_t crc = 0;

    for(int i = 0; i < VMA_DEV_REQUEST_LENGTH - 1; i++){
        crc ^= msg_buf[i];
    }

    msg_buf[VMA_DEV_REQUEST_CHECKSUM] = crc;


    ui->plainTextEditTransmit->appendPlainText(msg_buf.toHex().toUpper());

    serial->clear(QSerialPort::Input);
    MainWindow::writeData(msg_buf);

    MainWindow::readData();
}


void MainWindow::readData()
{    
    QByteArray data;
    if (serial->waitForReadyRead(REQUEST_DELAY)){
        data = serial->readAll();
        qDebug() << "read bytes - " << data.toHex();
    }
    ui->plainTextEditReceive->appendPlainText(data.toHex().toUpper());

    static uint8_t previous_state;
    if (previous_state != (uint8_t)data[VMA_DEV_RESPONSE_CURRENT_2L]){
        previous_state = (uint8_t)data[VMA_DEV_RESPONSE_CURRENT_2L];
        ui->plainTextEditHistory->appendPlainText(
                    QString::number(previous_state));
    }

    ui->lineEditAddress->setText(
                QString::number(data[VMA_DEV_RESPONSE_ADDRESS]));
    ui->lineEditCurrent->setText(
                QString::number(
                    (uint16_t)((uint8_t)data[VMA_DEV_RESPONSE_CURRENT_1H] << 8 |
                               (uint8_t)data[VMA_DEV_RESPONSE_CURRENT_1L])));
    ui->lineEditCommutationPeriod->setText(
                QString::number(
                    (uint16_t)((uint8_t)data[VMA_DEV_RESPONSE_VELOCITY1] << 8 |
                               (uint8_t)data[VMA_DEV_RESPONSE_VELOCITY2])));
    ui->lineEditState->setText(
                QString::number(data[VMA_DEV_RESPONSE_CURRENT_2L]));

    ui->radioButtonSensorA->setDown(
                (uint8_t)data[VMA_DEV_RESPONSE_CURRENT_2H] & 0b00000001);
    ui->radioButtonSensorB->setDown(
                (uint8_t)data[VMA_DEV_RESPONSE_CURRENT_2H] & 0b00000010);
    ui->radioButtonSensorC->setDown(
                (uint8_t)data[VMA_DEV_RESPONSE_CURRENT_2H] & 0b00000100);


    ui->lineEditGrayCode->setText(
                QString::number(data[VMA_DEV_RESPONSE_CURRENT_2H]));

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
            [this](int value){ui->spinBoxVelocity->setValue(value-127);});
    connect(ui->dial, &QDial::valueChanged,
            ui->spinBoxPosition, &QSpinBox::setValue);

    connect(ui->spinBoxFrequency,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            ui->verticalSliderFrequency, &QSlider::setValue);
    connect(ui->spinBoxVelocity,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            [this](int value){ui->verticalSliderVelocity->setValue(value +
                                                                   127);});
    connect(ui->spinBoxPosition,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            [this](int value)
    {
        ui->dial->setValue(value);
        position = value;
    });

    connect(send_timer, &QTimer::timeout, this, &MainWindow::request);
    connect(serial,
            static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>
            (&QSerialPort::error), this, &MainWindow::handleError);

    connect(ui->radioButton, &QRadioButton::toggled,
            [this](bool permission)
    {
        position_setting = permission;
        ui->dial->setEnabled(permission);
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
