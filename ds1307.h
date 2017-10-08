// RTC DS1307 (TinyRTC Module)
//
// file: ds1307.h
//
// toxcat :3  08-Oct-2017
//



#include <stdint.h>

#include "i2c.h"



//DS1307 I2C address
#define DS1307 0b11010000
#define WRITE  0b00000000
#define READ   0b00000001


#define SECONDS_REG  0
#define MINUTES_REG  1
#define HOURS_REG    2
#define DAY_REG      3
#define DATE_REG     4
#define MONTH_REG    5
#define YEAR_REG     6
#define CONTROL_REG  7

#define SUNDAY  7

uint8_t rtc[8];  //for ds1307 registers data



//-----------------------------------------------------------------------------
void _rtc_conv(uint8_t *d, uint8_t m)  //d-data //m-mask
    {
    *d=(((((*d & m)>>4)<<1)+(((*d & m)>>4)<<3))+(*d & 0x0F));
    }


//-----------------------------------------------------------------------------
void rtc_read(uint8_t *data)
    {
    i2c_start();
    i2c_send_byte(DS1307 | WRITE);

    i2c_send_byte(0x00);

    i2c_start();

    i2c_send_byte(DS1307 | READ);

    for(uint8_t i=0; i<7; i++) data[i] = i2c_read_byte(ACK);

    data[7] = i2c_read_byte(NACK);

    i2c_stop();

    _rtc_conv(&data[0], 0x70);
    _rtc_conv(&data[1], 0x70);
    _rtc_conv(&data[2], 0x30);
    _rtc_conv(&data[4], 0x30);
    _rtc_conv(&data[5], 0x10);
    _rtc_conv(&data[6], 0xF0);
    }


//-----------------------------------------------------------------------------
void rtc_write_reg(uint8_t addr, uint8_t val)
    {
    i2c_start();
    i2c_send_byte(DS1307 | WRITE);

    i2c_send_byte(addr);
    i2c_send_byte(val);

    i2c_stop();
    }


//-----------------------------------------------------------------------------
uint8_t rtc_read_reg(uint8_t addr)
    {
    i2c_start();
    i2c_send_byte(DS1307 | WRITE);

    i2c_send_byte(addr);

    i2c_start();
    i2c_send_byte(DS1307 | READ);

    uint8_t data = i2c_read_byte(NACK);

    i2c_stop();

    return data;
    }


//-----------------------------------------------------------------------------
inline void rtc_init(void)
    {
    uint8_t temp=0;

    temp = rtc_read_reg(0x00);
    rtc_write_reg(0x00, temp & 0b01111111); //rtc start

    temp = rtc_read_reg(0x02);
    rtc_write_reg(0x02, temp & 0b10111111); //set 24-hour mode

    rtc_write_reg(0x07, 0b10000000);
    }


/*
//-----------------------------------------------------------------------------
void _rtc_conv2(uint8_t *d, uint8_t m)  //d-data //m-mask
    {
    *d=(((*d/10)<<4)+(*d%10)) & m;
    }
*/


//-----------------------------------------------------------------------------
inline void rtc_set_sec(uint8_t val)
    {
    rtc_write_reg(0x00, (((val/10)<<4)+(val%10)) & 0b01111111);
    }


//-----------------------------------------------------------------------------
inline void rtc_set_min(uint8_t val)
    {
    rtc_write_reg(0x01, (((val/10)<<4)+(val%10)) & 0b01111111);
    }


//-----------------------------------------------------------------------------
inline void rtc_set_hrs(uint8_t val)
    {
    rtc_write_reg(0x02, (((val/10)<<4)+(val%10)) & 0b10111111);
    }


//-----------------------------------------------------------------------------
inline void rtc_set_day(uint8_t val)
    {
    rtc_write_reg(0x03, (val & 0b00000111));
    }


//-----------------------------------------------------------------------------
inline void rtc_set_dat(uint8_t val)
    {
    rtc_write_reg(0x04, (((val/10)<<4)+(val%10)) & 0b00111111);
    }


//-----------------------------------------------------------------------------
inline void rtc_set_mon(uint8_t val)
    {
    rtc_write_reg(0x05, (((val/10)<<4)+(val%10)) & 0b00011111);
    }


//-----------------------------------------------------------------------------
inline void rtc_set_year(uint8_t val)
    {
    rtc_write_reg(0x06, ((val/10)<<4)+(val%10));
    }

