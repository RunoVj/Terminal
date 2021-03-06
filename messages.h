#ifndef __MESSAGES_H
#define __MESSAGES_H

#include <stdint.h>
#include <QDataStream>
#include <QVector>

#define REQUEST_DELAY                       10
#define RESPONSE_DELAY                      9
#define MAX_CURRENT                         4095
#define MAX_CURRENT_A                       30
#define SENSOR_OFFSET                       0.9
#define CURRENT_COEF ((MAX_CURRENT*SENSOR_OFFSET-MAX_CURRENT/2)/MAX_CURRENT_A)

#define NORMAL_REQUEST_TYPE                 0x01
#define TERMINAL_REQUEST_TYPE               0x02
#define CONFIG_REQUEST_TYPE                 0x03
#define FIRMWARE_REQUEST_TYPE               0x04
#define DEVICE_REQUEST_TYPE                 0x05

/* STM send requests and VMA send responses */
struct Request
{
    uint8_t AA;
    uint8_t type; // 0x01
    uint8_t address;
    int8_t velocity;
    uint8_t CRC;

    friend QDataStream& operator<<(QDataStream &ds, const Request &req)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << req.AA;
        ds << req.type;
        ds << req.address;
        ds << req.velocity;
        return ds;
    }

    friend QDataStream& operator>>(QDataStream &ds, Request &req)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds >> req.AA;
        ds >> req.type;
        ds >> req.address;
        ds >> req.velocity;
        ds >> req.CRC;
        return ds;
    }

};

enum WorkingState:uint8_t {stopped, rotated, overcurrent};

struct Response
{
    uint8_t AA;
    uint8_t type;  // 0x01
    uint8_t address;
    WorkingState state;
    uint16_t current;
    uint16_t speed_period;
    uint8_t CRC;

    friend QDataStream& operator<<(QDataStream &ds, const Response &resp)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << resp.AA;
        ds << resp.type;
        ds << resp.address;
        ds << static_cast<uint8_t>(resp.state);
        ds << resp.current;
        ds << resp.speed_period;

        return ds;
    }

    friend QDataStream& operator>>(QDataStream &ds, Response &resp)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds >> resp.AA;
        ds >> resp.type;
        ds >> resp.address;
        uint8_t state;
        ds >> state;
        switch(state) {
        case stopped: resp.state = stopped; break;
        case rotated: resp.state = rotated; break;
        case overcurrent: resp.state = overcurrent; break;
        }
        ds >> resp.current;
        ds >> resp.speed_period;
        ds >> resp.CRC;

        return ds;
    }
};

struct TerminalRequest
{
    uint8_t AA;
    uint8_t type; // 0x02
    uint8_t address;
    uint8_t update_base_vector; // true or false
    uint8_t position_setting; // enabling of position_setting
    uint16_t angle; // angle - 0..359;
    int8_t velocity;
    uint8_t frequency;
    int16_t outrunning_angle;
    uint8_t update_speed_k; // if false thruster will use previous values from flash
    uint16_t speed_k;
    uint8_t CRC;

    friend QDataStream& operator<<(QDataStream &ds, const TerminalRequest &req)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << req.AA;
        ds << req.type;
        ds << req.address;
        ds << req.update_base_vector;
        ds << req.position_setting;
        ds << req.angle;
        ds << req.velocity;
        ds << req.frequency;
        ds << req.outrunning_angle;
        ds << req.update_speed_k;
        ds << req.speed_k;
        return ds;
    }

    friend QDataStream& operator>>(QDataStream &ds, TerminalRequest &req)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds >> req.AA;
        ds >> req.type;
        ds >> req.address;
        ds >> req.update_base_vector;
        ds >> req.position_setting;
        ds >> req.angle;
        ds >> req.velocity;
        ds >> req.frequency;
        ds >> req.outrunning_angle;
        ds >> req.update_speed_k;
        ds >> req.speed_k;
        ds >> req.CRC;
        return ds;
    }

};

struct TerminalResponse
{
    uint8_t AA;
    uint8_t type;  // 0x02
    uint8_t address;
    WorkingState state;
    uint8_t position_code;
    uint16_t cur_angle;
    uint16_t current;
    uint16_t speed_period;
    uint16_t clockwise_speed_k;
    uint16_t counterclockwise_speed_k;
    uint8_t CRC;

    friend QDataStream& operator<<(QDataStream &ds, const TerminalResponse &resp)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << resp.AA;
        ds << resp.type;
        ds << resp.address;
        ds << static_cast<uint8_t>(resp.state);
        ds << resp.position_code;
        ds << resp.cur_angle;
        ds << resp.current;
        ds << resp.speed_period;
        ds << resp.clockwise_speed_k;
        ds << resp.counterclockwise_speed_k;
        return ds;
    }

    friend QDataStream& operator>>(QDataStream &ds, TerminalResponse &resp)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds >> resp.AA;
        ds >> resp.type;
        ds >> resp.address;
        uint8_t state;
        ds >> state;
        switch(state) {
        case stopped: resp.state = stopped; break;
        case rotated: resp.state = rotated; break;
        case overcurrent: resp.state = overcurrent; break;
        }
        ds >> resp.position_code;
        ds >> resp.cur_angle;
        ds >> resp.current;
        ds >> resp.speed_period;
        ds >> resp.clockwise_speed_k;
        ds >> resp.counterclockwise_speed_k;
        ds >> resp.CRC;
        return ds;
    }
};

struct ConfigRequest
{
    uint8_t AA;
    uint8_t type; // 0x03
    uint8_t update_firmware; // (bool) go to bootloader and update firmware
    uint8_t forse_setting; // (bool) set new address or update firmware even if old address doesn't equal BLDC address
    uint8_t old_address;
    uint8_t new_address;
    uint16_t high_threshold;
    uint16_t low_threshold;
    uint16_t average_threshold;
    uint8_t update_correction;
    uint16_t clockwise_speed_k;
    uint16_t counterclockwise_speed_k;
    uint8_t CRC;

    friend QDataStream& operator<<(QDataStream &ds, const ConfigRequest &req)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << req.AA;
        ds << req.type;
        ds << req.update_firmware;
        ds << req.forse_setting;
        ds << req.old_address;
        ds << req.new_address;
        ds << req.high_threshold;
        ds << req.low_threshold;
        ds << req.average_threshold;
        ds << req.update_correction;
        ds << req.clockwise_speed_k;
        ds << req.counterclockwise_speed_k;
        return ds;
    }

    friend QDataStream& operator>>(QDataStream &ds, ConfigRequest &req)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds >> req.AA;
        ds >> req.type;
        ds >> req.update_firmware;
        ds >> req.forse_setting;
        ds >> req.old_address;
        ds >> req.new_address;
        ds >> req.high_threshold;
        ds >> req.low_threshold;
        ds >> req.average_threshold;
        ds >> req.update_correction;
        ds >> req.clockwise_speed_k;
        ds >> req.counterclockwise_speed_k;
        ds >> req.CRC;
        return ds;
    }
};

struct FirmwaregRequest
{
    uint8_t AA;
    uint8_t type;  // 0x04
    uint8_t address;
    uint8_t force_update; // update even if address doesn't equal BLDC address
    uint8_t get_response; // send status
    uint16_t index;
    //hex

    struct IntelHEX
    {
        uint8_t _data_size;
        uint16_t start_address;
        uint8_t operation_type;
        QVector<uint8_t> data;
        uint8_t CRC;
    } hex;

    uint8_t CRC;

    friend QDataStream& operator<<(QDataStream &ds, const FirmwaregRequest &req)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << req.AA;
        ds << req.type;
        ds << req.address;
        ds << req.force_update;
        ds << req.get_response;
        ds << req.index;
        ds << req.hex._data_size;
        ds << req.hex.start_address;
        ds << req.hex.operation_type;
        for (auto itr : req.hex.data) {
            ds << itr;
        }
        ds << req.hex.CRC;

        return ds;
    }

    friend QDataStream& operator>>(QDataStream &ds, FirmwaregRequest &req)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds >> req.AA;
        ds >> req.type;
        ds >> req.address;
        ds >> req.force_update;
        ds >> req.get_response;
        ds >> req.index;
        ds >> req.hex._data_size;
        ds >> req.hex.start_address;
        ds >> req.hex.operation_type;
        for (int i = 0; i < req.hex._data_size; ++i) {
            uint8_t tmp;
            ds >> tmp;
            req.hex.data.push_back(tmp);
        }
        ds >> req.hex.CRC;
        ds >> req.CRC;

        return ds;
    }

    friend QDataStream& operator>>(QDataStream &ds, FirmwaregRequest::IntelHEX &hex)
    {
        ds.setByteOrder(QDataStream::BigEndian);
        ds >> hex._data_size;
        ds >> hex.start_address;
        ds >> hex.operation_type;
        for (int i = 0; i < hex._data_size; ++i) {
            uint8_t tmp;
            ds >> tmp;
            hex.data.push_back(tmp);
        }
        ds >> hex.CRC;

        return ds;
    }
};

struct FirmwareResponse
{
    uint8_t AA;
    uint8_t type; // 0x04
    uint8_t address;
    uint8_t status;
    uint16_t index;
    uint8_t CRC;

    friend QDataStream& operator<<(QDataStream &ds, const FirmwareResponse &resp)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << resp.AA;
        ds << resp.type;
        ds << resp.address;
        ds << resp.status;
        ds << resp.index;
        return ds;
    }

    friend QDataStream& operator>>(QDataStream &ds, FirmwareResponse &resp)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds >> resp.AA;
        ds >> resp.type;
        ds >> resp.address;
        ds >> resp.status;
        ds >> resp.index;
        ds >> resp.CRC;
        return ds;
    }
};

struct DevicesRequest
{
    uint8_t AA1;
    uint8_t AA2;
    uint8_t address;
    uint8_t setting;
    uint8_t velocity1;
    uint8_t velocity2;
    uint8_t CRC;

    friend QDataStream& operator<<(QDataStream &ds, const DevicesRequest &req)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << req.AA1;
        ds << req.AA2;
        ds << req.address;
        ds << req.setting;
        ds << req.velocity1;
        ds << req.velocity2;
        return ds;
    }

    friend QDataStream& operator>>(QDataStream &ds, DevicesRequest &req)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds >> req.AA1;
        ds >> req.AA2;
        ds >> req.address;
        ds >> req.setting;
        ds >> req.velocity1;
        ds >> req.velocity2;
        ds >> req.CRC;
        return ds;
    }
};

struct DevicesResponse
{
    uint8_t AA;
    uint8_t address;
    uint8_t errors;
    uint16_t current1;
    uint16_t current2;
    uint8_t velocity1;
    uint8_t velocity2;
    uint8_t CRC;

    friend QDataStream& operator<<(QDataStream &ds, const DevicesResponse &resp)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds << resp.AA;
        ds << resp.address;
        ds << resp.errors;
        ds << resp.current1;
        ds << resp.current2;
        ds << resp.velocity1;
        ds << resp.velocity2;
        return ds;
    }

    friend QDataStream& operator>>(QDataStream &ds, DevicesResponse &resp)
    {
        ds.setByteOrder(QDataStream::LittleEndian);
        ds >> resp.AA;
        ds >> resp.address;
        ds >> resp.errors;
        ds >> resp.current1;
        ds >> resp.current2;
        ds >> resp.velocity1;
        ds >> resp.velocity2;
        ds >> resp.CRC;
        return ds;
    }
};


#endif //__MESSAGES_H
