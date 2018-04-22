#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QSerialPort>
#include <QTest>
#include <QDebug>
#include <qtimer.h>
#include <stdint.h>
#include <messages.h>
#include <iostream>
#include <math.h>
#include <string>
#include <cstdlib>
#include "qcustomplot.h"

#include "joystick.h"
#include "settings.h"


#define REQUEST_DELAY					25
#define REQUEST_TIMEOUT					25
#define CHANGE_TYPE_TIMEOUT             200

#define MESSAGES_ARRAY_SIZE             REQUEST_CONFIG_LENGTH



class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    ~Server();
    Joystick* j;

    uint8_t msg_to_send[MESSAGES_ARRAY_SIZE];
    QByteArray msg_in;

    uint8_t nextMessageType;
    uint8_t currentMessageType;

    QTimer *sendTimer;

    QFile file_csv_response;
    QFile file_csv_request;

    QTextStream stream_response;
    QTextStream stream_request;

    Settings* settings;

    //float temperature = 0;
    int16_t imu_roll;
    int16_t imu_pitch;
    int16_t imu_yaw;

    int16_t imu_roll_speed;
    int16_t imu_pitch_speed;
    int16_t imu_yaw_speed;

    // For plots
    int16_t imu_roll_d;
    int16_t imu_pitch_d;
    int16_t imu_yaw_d;

    int16_t imu_roll_speed_d;
    int16_t imu_pitch_speed_d;
    int16_t imu_yaw_speed_d;

    double key1;

    // _______________________________

    //int16_t imu_depth;
    uint16_t imu_pressure;

    uint8_t wf_data_type;
    uint8_t wf_tick_rate;
    uint8_t wf_voltage;
    float wf_x_angle_float;
    float wf_y_angle_float;
    int32_t wf_x_angle_int32_t;
    int32_t wf_y_angle_int32_t;

    uint8_t acoustic_state;
    int16_t leak_sensor;
    int16_t in_pressure;

    uint16_t current_HLB;
    uint16_t current_HLF;
    uint16_t current_HRB;
    uint16_t current_HRF;
    uint16_t current_VB;
    uint16_t current_VF;
    uint16_t current_VL;
    uint16_t current_VR;

    int8_t velocity_HLB;
    int8_t velocity_HLF;
    int8_t velocity_HRB;
    int8_t velocity_HRF;
    int8_t velocity_VB;
    int8_t velocity_VF;
    int8_t velocity_VL;
    int8_t velocity_VR;

    uint16_t current_light;
    uint16_t current_bottom_light;
    uint16_t current_agar;
    uint16_t current_grab;
    uint16_t current_grab_rotate;
    uint16_t current_tilt;

    uint16_t err_vma;
    uint8_t err_dev;
    uint8_t err_pc;

    uint16_t msg_received_counter;
    uint16_t msg_lost_counter;
    float msg_lost_percent;

    QString bt_data;
    QString path_csv_request;
    QString path_csv_response;

    bool emulation_mode = false;

private:
    QSerialPort *newPort;
    const QString log_folder_path = "C:\\logs\\";

    bool COMconnect(int com_name);

    void addCheckSumm16b(uint8_t * msg, uint16_t length);
    uint8_t isCheckSumm16bCorrect(uint8_t * msg, uint16_t length);

    void sendMessageNormal();
    void sendMessageDirect();
    void sendMessageConfig();
    void receiveMessage();

    void addFloat(uint8_t * msg, int position, float val);
    float getFloat(QByteArray msg, int position);
    void addSNP(uint8_t * msg);

    //float encodeTemperature(uint8_t MS, uint8_t LS);

    void writeCSV(QTextStream* stream, uint8_t * msg, uint16_t length);


signals:
    void imSleeping();
    void info(QString s);

public slots:
    void connect_fake();

    void connect_com();
    void disconnect_com();

private slots:
    void sendMessage();
};

#endif // SERVER_H
