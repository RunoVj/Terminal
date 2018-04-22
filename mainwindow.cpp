#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
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
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
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
    msg_buf[VMA_DEV_REQUEST_CHECKSUM] = 0xFF;


    serial->clear(QSerialPort::Input);
    MainWindow::writeData(msg_buf);

    MainWindow::readData();
}

void MainWindow::readData()
{    
    //Если получен ответ в течение зад.времени
    QByteArray responseData;
    if (serial->waitForReadyRead(10)){
        responseData = serial->readAll();
        while (serial->waitForReadyRead(10)){
            responseData += serial->readAll();
        }
    }
    qDebug() << "read bytes - " << responseData.toHex();
    ui->plainTextEditReceive->appendPlainText(responseData.toHex());

    //Если получен ответ в течение зад.времени
//    QByteArray responseData;
//    if (serial->waitForReadyRead(5)){
//        responseData = serial->readAll();
////        while (serial->waitForReadyRead(5)){
////            responseData += serial->readAll();
////        }
//    }
//    qDebug() << "read bytes size - " << responseData.size() << "data - " <<  responseData.toHex();
//    qDebug() << "response data - " << responseData;
//    ui->plainTextEditReceive->appendPlainText(responseData.toHex());

//    QByteArray responseData = serial->readAll();
//    qDebug() << responseData;
//    ui->plainTextEditReceive->appendPlainText(responseData.toHex());
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionConfigure, &QAction::triggered, settings, &SettingsDialog::show);
    connect(ui->actionClear, &QAction::triggered, [this](void){ui->plainTextEditReceive->clear();});
    connect(ui->actionClear, &QAction::triggered, [this](void){ui->plainTextEditTransmit->clear();});

    connect(send_timer, &QTimer::timeout, this, &MainWindow::request);
    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleError);
//    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(ui->verticalSliderVelocity, &QSlider::valueChanged,
            [this](int value){ui->spinBoxVelocity->setValue(value-127);});
    connect(ui->radioButton, &QRadioButton::toggled, [this](bool permission){position_setting = permission;});
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
