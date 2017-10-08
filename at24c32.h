// AT24C32 4096*8 EEPROM (TinyRTC Module)
//
// file: at24c32.h
//
// toxcat :3  24-Jul-2017
//



#include <stdint.h>
#include <util/delay.h>

#include "i2c.h"



#define EE_W 0b10100000
#define EE_R 0b10100001



//-----------------------------------------------------------------------------
void ee_write(uint16_t addr, uint8_t data)
    {
    i2c_start();
    i2c_send_byte(EE_W); //device address | write
    i2c_send_byte(addr>>8); //word address
    i2c_send_byte(addr);
    i2c_send_byte(data); //write data word
    i2c_stop();

    _delay_ms(10);  //wait
    }


//-----------------------------------------------------------------------------
uint8_t ee_read(uint16_t addr)
    {
    uint8_t data = 0;

    i2c_start();
    i2c_send_byte(EE_W); //device address | write
    i2c_send_byte(addr>>8); //word address
    i2c_send_byte(addr);
    i2c_start();
    i2c_send_byte(EE_R); //device address | read
    data = i2c_read_byte(NACK);
    i2c_stop();

    return data;
    }


//-----------------------------------------------------------------------------
inline void ee_clear(void)
    {
    for(uint16_t k=0; k<4096; k++) ee_write(k, 0xff);
    }






