// Software Two-Wire Interface (I2C)
//
// file: i2c.h
//
// toxcat :3  24-Jul-2017
//


#ifndef I2C_H_
#define I2C_H_



#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#include "bitm.h"



#define I2C_DELAY  _delay_us(5)

#define ACK  0
#define NACK 1


///////////////////////////////////////////////////////////////////////////////

#define I2C_DDR_REG   DDRD
#define I2C_PORT_REG  PORTD
#define I2C_PIN_REG   PIND

#define I2C_SDA_PIN   4
#define I2C_SCL_PIN   5

///////////////////////////////////////////////////////////////////////////////


#define SDA_INP  CLR_BIT(I2C_DDR_REG, I2C_SDA_PIN)
#define SDA_OUT  SET_BIT(I2C_DDR_REG, I2C_SDA_PIN)
#define SDA_SET  SET_BIT(I2C_PORT_REG, I2C_SDA_PIN)
#define SDA_CLR  CLR_BIT(I2C_PORT_REG, I2C_SDA_PIN)

#define SDA_PULL  SDA_INP; SDA_SET
#define SDA_NULL  SDA_OUT; SDA_CLR

#define SDA_IS_SET  BIT_IS_SET(I2C_PIN_REG, I2C_SDA_PIN)

#define SCL_INP  CLR_BIT(I2C_DDR_REG, I2C_SCL_PIN)
#define SCL_OUT  SET_BIT(I2C_DDR_REG, I2C_SCL_PIN)
#define SCL_SET  SET_BIT(I2C_PORT_REG, I2C_SCL_PIN)
#define SCL_CLR  CLR_BIT(I2C_PORT_REG, I2C_SCL_PIN)

#define SCL_PULL  SCL_INP; SCL_SET
#define SCL_NULL  SCL_OUT; SCL_CLR



//-----------------------------------------------------------------------------
void i2c_init(void)
    {
    SDA_PULL;
    SCL_PULL;
    }


//-----------------------------------------------------------------------------
void i2c_start(void)
    {
    SDA_PULL;
    I2C_DELAY;
    SCL_PULL;
    I2C_DELAY;

    SDA_NULL;
    I2C_DELAY;
    SCL_NULL;
    I2C_DELAY;
    }


//-----------------------------------------------------------------------------
void i2c_stop(void)
    {
    SCL_NULL;
    I2C_DELAY;
    SDA_NULL;
    I2C_DELAY;

    SCL_PULL;
    I2C_DELAY;
    SDA_PULL;
    I2C_DELAY;
    }


//-----------------------------------------------------------------------------
void i2c_send_byte(uint8_t data)
    {
    for(uint8_t mask=0x80; mask; mask>>=1)
        {
        if(data & mask) { SDA_PULL; }
        else { SDA_NULL; }

        I2C_DELAY;
        SCL_PULL;
        I2C_DELAY;
        SCL_NULL;
        }

    SDA_PULL;
    I2C_DELAY;
    SCL_PULL;
    I2C_DELAY;
    SCL_NULL;
    }


//-----------------------------------------------------------------------------
uint8_t i2c_read_byte(uint8_t ask)
    {
    uint8_t data=0;

    SDA_PULL;

    for(uint8_t i=0; i<8; i++)
        {
        data<<=1;
        SCL_PULL;
        I2C_DELAY;
        if(SDA_IS_SET) data |= 0x01;
        SCL_NULL;
        I2C_DELAY;
        }

    if(ask==ACK) { SDA_NULL; }
    else { SDA_PULL; }

    I2C_DELAY;
    SCL_PULL;
    I2C_DELAY;
    SCL_NULL;
    I2C_DELAY;
    SDA_PULL;

    return data;
    }


#endif  //end of I2C_H_

