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
    _send_timer = new QTimer(this);

    _position_setting = 0;
    _position = 0;
    _pwm_duty = ui->verticalSliderVelocity->value();
    _period = ui->verticalSliderFrequency->value();
    _cur_mes_type = _next_mes_type = NORMAL_REQUEST_TYPE;
    _firmware.clear();


    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);

    _status = new QLabel;

    ui->statusbar->addWidget(_status);

    initActionsConnections();
}

MainWindow::~MainWindow()
{
    delete serial;
    delete settings;
    delete _send_timer;
    delete ui;
    delete settings;
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
    connect(ui->actionOpenHexFile, &QAction::triggered,
            this, &MainWindow::open_hex);

    connect(ui->verticalSliderFrequency, &QSlider::valueChanged,
            ui->spinBoxFrequency, &QSpinBox::setValue);
    connect(ui->verticalSliderVelocity, &QSlider::valueChanged,
            [this](int value){ui->spinBoxVelocity->setValue(value);});
    connect(ui->verticalSliderSpeedK, &QSlider::valueChanged,
            ui->spinBoxSpeedK, &QSpinBox::setValue);

    connect(ui->verticalSliderSpeedK, &QSlider::valueChanged,
            [this] (int value)
    {
        if (ui->verticalSliderVelocity->value() > 0) {
            ui->spinBoxClockwiseK->setValue(value);
        }
        else {
            ui->spinBoxCounterClockwiseK->setValue(value);
        }
    });

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
    connect(ui->spinBoxSpeedK,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            [this](int value){ui->verticalSliderSpeedK->setValue(value);});
    connect(ui->spinBoxPosition,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            [this](int value)
    {
        ui->dialAngle->setValue(value);
        _position = value;
    });
    connect(ui->spinBoxOutrunningAngle,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            ui->dialOutrunningAngle, &QDial::setValue);

    connect(_send_timer, &QTimer::timeout, this, &MainWindow::request);
    connect(serial,
            static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>
            (&QSerialPort::error), this, &MainWindow::handleError);

    connect(ui->radioButtonPosition, &QRadioButton::toggled,
            [this](bool permission)
    {
        _position_setting = permission;
        ui->dialAngle->setEnabled(permission);
        ui->spinBoxPosition->setEnabled(permission);

    });

    connect(ui->pushButtonSetAddress, &QPushButton::clicked,
            this, &MainWindow::config_request);

    // flashing
    connect(ui->pushButtonFlash, &QPushButton::clicked,
            this, &MainWindow::flash);

    // current thresholds
    connect(ui->radioButtonEnableCurrentThresholds, &QRadioButton::clicked,
            this, &MainWindow::disable_current_thresholds);

    // run stress test cycle
    connect(ui->pushButtonRunStressTest, &QPushButton::toggled, [this](bool checked)
    {
        if (checked) {
            _stress_test_timer = new QTimer;
            connect(_stress_test_timer, &QTimer::timeout, this, &MainWindow::stress_test_timer_timeout);
            _stress_test_timer->start(static_cast<int>(1000/ui->spinBoxStressFrequency->value()));
            stress_test_numb = ui->spinBoxStressTestDuration->value()*
                    ui->spinBoxStressFrequency->value();
        }
        else {
            _stress_test_timer->stop();
            delete _stress_test_timer;
            disconnect(_stress_test_timer, &QTimer::timeout, this, &MainWindow::stress_test_timer_timeout);
        }
    });
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
        _send_timer->start(REQUEST_DELAY);
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
    _send_timer->stop();
}

void MainWindow::writeData(const QByteArray &data)
{
    serial->write(data);
}

void MainWindow::request()
{    
    QByteArray msg_buf;
    QDataStream stream(&msg_buf, QIODevice::Append);
    // fill request structure
    if (_next_mes_type == NORMAL_REQUEST_TYPE) {
        _cur_mes_type = NORMAL_REQUEST_TYPE;

        Request req;
        req.AA = 0xAA;
        req.type = NORMAL_REQUEST_TYPE;
        if (ui->checkBoxCircleMode->isChecked()) {
            req.address = ui->spinBoxCurrentAddress->value();
            uint8_t next_addr = (ui->spinBoxCurrentAddress->value() + 1) % 9;
            ui->spinBoxCurrentAddress->setValue(next_addr);
        }
        else {
            req.address = ui->spinBoxCurrentAddress->value();
        }
        req.update_base_vector = ui->checkBoxUpdateBaseVector->isChecked();
        req.position_setting = ui->radioButtonPosition->isChecked();
        req.angle = ui->dialAngle->value();
        req.velocity = ui->verticalSliderVelocity->value();
        req.frequency = ui->verticalSliderFrequency->value();

        req.outrunning_angle = ui->dialOutrunningAngle->value();
        req.update_base_vector = ui->checkBoxUpdateBaseVector->isChecked();
        req.speed_k = ui->verticalSliderSpeedK->value();

        // move to QByteArray
        stream << req;
    }
    else if (_next_mes_type == CONFIG_REQUEST_TYPE) {
        _cur_mes_type = CONFIG_REQUEST_TYPE;
        _next_mes_type = NORMAL_REQUEST_TYPE;

        ConfigRequest conf_req;
        conf_req.AA = 0xAA;
        conf_req.type = CONFIG_REQUEST_TYPE;
        conf_req.update_firmware = ui->checkBoxUpdateFirmware->isChecked();
        conf_req.forse_setting = ui->checkBoxForceSettingAddress->isChecked();
        conf_req.new_address = ui->spinBoxSetAddress->value();
        if (conf_req.forse_setting) {
            conf_req.old_address = 0;
        }
        else {
            conf_req.old_address = ui->spinBoxCurrentAddress->value();
        }

        conf_req.high_threshold = static_cast<uint16_t>(
                    ui->lineEditCurrentHighThreshold->text().toInt());
        conf_req.low_threshold = static_cast<uint16_t>(
                    ui->lineEditCurrentLowThreshold->text().toInt());
        conf_req.average_threshold = static_cast<uint16_t>(
                    ui->lineEditCurrentAverageThreshold->text().toInt());


        // move to QByteArray
        stream << conf_req;
    }

    else if (_next_mes_type == FIRMWARE_REQUEST_TYPE) {
        _cur_mes_type = FIRMWARE_REQUEST_TYPE;
        // move hex string line to QByteArray
        if (_firmware.isEmpty()) {
            QMessageBox::warning(this, "title", "hex file doesn't open!");
            _next_mes_type = NORMAL_REQUEST_TYPE;
            return;
        }
        QByteArray cur_str = _firmware.at(0);
        _firmware.removeFirst();

        FirmwaregRequest firmware_req;
        FirmwaregRequest::IntelHEX hex_line;
        QDataStream ds(&cur_str, QIODevice::ReadOnly);
        ds >> hex_line;

        firmware_req.AA = 0xAA;
        firmware_req.type = FIRMWARE_REQUEST_TYPE;
        firmware_req.address = ui->spinBoxCurrentAddress->value();
        firmware_req.force_update = ui->checkBoxForceSettingAddress->isChecked();
        firmware_req.get_response = ui->checkBoxSendResponse->isChecked();
        firmware_req.hex = hex_line;

        if (_firmware.isEmpty()) {
            QMessageBox::information(this, "title",
                                     "Firmware upgrade complited!");
            _next_mes_type = NORMAL_REQUEST_TYPE;
        }

        stream << firmware_req;
    }

    // calculate CRC
    uint8_t crc = 0;

    // 0xAA doesn't include in CRC calculation
    for (int i = 1; i < msg_buf.size(); i++){
        crc ^= msg_buf[i];
    }
    stream << crc;

    ui->plainTextEditTransmit->appendPlainText(msg_buf.toHex().toUpper());
    serial->clear(QSerialPort::Input);
    writeData(msg_buf);
    readData();
}

void MainWindow::config_request()
{
    _next_mes_type = CONFIG_REQUEST_TYPE;
}

void MainWindow::readData()
{    
    qDebug() << "read data";
    QByteArray data;

    if (serial->waitForReadyRead(RESPONSE_DELAY)){
        data = serial->readAll();
        qDebug() << "read bytes - " << data.toHex();
    }
    if (!data.isEmpty()) {
        ui->plainTextEditReceive->appendPlainText(data.toHex().toUpper());
    }

    QDataStream stream(&data, QIODevice::ReadOnly);;

    if (_cur_mes_type == FIRMWARE_REQUEST_TYPE) {
        struct FirmwareResponse resp;
    }
    else {
        struct Response resp;

        stream >> resp;

        ui->lineEditAddress->setText(QString::number(resp.address));
        float current_in_amp = (resp.current - MAX_CURRENT/2)/CURRENT_COEF;
        ui->lineEditCurrent->setText(QString::number(current_in_amp) + " A (" +
                                     QString::number(resp.current) + " ADC)");

        ui->lineEditState->setText(QString::number(resp.state));

        ui->radioButtonSensorA->setDown(resp.position_code & 0b00000001);
        ui->radioButtonSensorB->setDown(resp.position_code & 0b00000010);
        ui->radioButtonSensorC->setDown(resp.position_code & 0b00000100);

        ui->lineEditSpeedPeriod->setText(QString::number(resp.speed_period));
    }

}

void MainWindow::open_hex()
{
    QString file_name = QFileDialog::getOpenFileName(
                this, "Open hex file",
                "../brushless_motor_control/MDK-ARM/brushless_motor/",
                "hex file (*.hex)");

    QFile hex_file(file_name);
    if (!hex_file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "title", "file not open");
    }
    QTextStream hex(&hex_file);
    ui->plainTextEditHex->setPlainText(hex.readAll());

}

void MainWindow::flash()
{
    QString hex = ui->plainTextEditHex->toPlainText();
    _firmware.clear();
    for (auto itr : hex.split("\n")) {
        if (itr == "") {
            continue;
        }
        _firmware.push_back(QByteArray::fromHex(itr.toUtf8()));
    }
    _next_mes_type = FIRMWARE_REQUEST_TYPE;
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"),
                              serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::showStatusMessage(const QString &message)
{
    _status->setText(message);
}

void MainWindow::on_verticalSliderVelocity_valueChanged(int value)
{
    _pwm_duty = value;
    qDebug() << "pwm duty = " << _pwm_duty << "\n";
}

void MainWindow::on_verticalSliderPosition_valueChanged(int value)
{
    _position = value;
    qDebug() << "position = " << _position << "\n";
}

void MainWindow::on_verticalSliderFrequency_valueChanged(int value)
{
    _period = value;
    qDebug() << "period = " << _period << "\n";
}

void MainWindow::disable_current_thresholds(bool enable_thresholds)
{
    if (enable_thresholds) {
        ui->lineEditCurrentHighThreshold->setReadOnly(false);
        ui->lineEditCurrentAverageThreshold->setReadOnly(false);
        ui->lineEditCurrentLowThreshold->setReadOnly(false);
    }
    else {
        ui->lineEditCurrentHighThreshold->setText(QString::number(4095));
        ui->lineEditCurrentAverageThreshold->setText(QString::number(4095));
        ui->lineEditCurrentLowThreshold->setText(QString::number(0));
        ui->lineEditCurrentHighThreshold->setReadOnly(true);
        ui->lineEditCurrentAverageThreshold->setReadOnly(true);
        ui->lineEditCurrentLowThreshold->setReadOnly(true);
    }
}

void MainWindow::stress_test_timer_timeout()
{
    ui->verticalSliderVelocity->setValue(-ui->verticalSliderVelocity->value());
    --stress_test_numb;
    if (stress_test_numb > 0) {
        _stress_test_timer->start(static_cast<int>(1000/ui->spinBoxStressFrequency->value()));
    }
    else {
        _stress_test_timer->stop();
    }
}
