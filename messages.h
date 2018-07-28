#ifndef __MESSAGES_H
#define __MESSAGES_H

#include <stdint.h>
#include <QDataStream>

#define REQUEST_DELAY                       10
#define RESPONSE_DELAY                      8
#define REQUEST_SIZE                        12

/* STM send requests and VMA send responses */
struct Request
{
    uint8_t AA;
    uint8_t type; // 0x01
    uint8_t address;
    uint8_t update_base_vector; // true or false
    uint8_t position_setting; // enabling of position_setting
    uint16_t angle; // angle - 0..359;
    int8_t velocity;
    uint8_t frequency;
    int16_t outrunning_angle;
    uint8_t CRC;

    friend QDataStream& operator<<(QDataStream &ds, const Request &req)
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
        return ds;
    }

    friend QDataStream& operator>>(QDataStream &ds, Request &req)
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
        ds >> req.CRC;
        return ds;
    }

} ;

struct ConfigRequest
{
    uint8_t AA;
    uint8_t type; // 0x02
    uint8_t new_address;
    uint8_t CRC;
} ;

enum WorkingState:uint8_t {stopped, rotated, overcurrent};

struct Response
{
    uint8_t AA;
    uint8_t type;
    uint8_t address;
    WorkingState state;
    uint8_t position_code;
    uint16_t cur_angle;
    uint16_t current;
    uint8_t CRC;

    friend QDataStream& operator<<(QDataStream &ds, const Response &resp)
    {
        ds << resp.AA;
        ds << resp.type;
        ds << resp.address;
        ds << static_cast<uint8_t>(resp.state);
        ds << resp.position_code;
        ds << resp.cur_angle;
        ds << resp.current;

        return ds;
    }

    friend QDataStream& operator>>(QDataStream &ds, Response &resp)
    {
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
        return ds;
    }
};



#endif //__MESSAGES_H
