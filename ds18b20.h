// 1-Wire interface for DS18B20
//
// file: ds18b20.h
//
// toxcat :3  08-Oct-2017
//



#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "bitm.h"



///////////////////////////////////////////////////////////////////////////////

#define DAT_PORT_REG  PORTD
#define DAT_DDR_REG   DDRD
#define DAT_PIN_REG   PIND
#define DAT_PIN       6

///////////////////////////////////////////////////////////////////////////////



#define DAT_INPUT   CLR_BIT(DAT_DDR_REG, DAT_PIN)
#define DAT_OUTPUT  SET_BIT(DAT_DDR_REG, DAT_PIN)
#define DAT_LOW     CLR_BIT(DAT_PORT_REG, DAT_PIN)
#define DAT_HIGH    SET_BIT(DAT_PORT_REG, DAT_PIN)

#define DAT_READ    BIT_IS_SET(DAT_PIN_REG, DAT_PIN)

#define DAT_INPUT_PULL  DAT_LOW; DAT_INPUT
#define DAT_OUTPUT_LOW  DAT_LOW; DAT_OUTPUT


#define CMD_CONVERTTEMP 0x44
#define CMD_RSCRATCHPAD 0xbe
#define CMD_SKIPROM 0xcc


uint8_t temp[3]; //ds18b20 data



//-----------------------------------------------------------------------------
uint8_t trm_reset(void)
    {
    DAT_OUTPUT_LOW;
    _delay_us(480);

    DAT_INPUT_PULL;
    _delay_us(60);

    uint8_t i = DAT_READ;
    _delay_us(420);

    return i;  //0 = OK
    }


//-----------------------------------------------------------------------------
void trm_write_bit(uint8_t bit)
    {
    cli();  //<<< disable all interrupts <<<

    DAT_OUTPUT_LOW;
    _delay_us(1);

    if(bit) { DAT_INPUT_PULL; }

    _delay_us(60);
    DAT_INPUT_PULL;
    sei();
    }


//-----------------------------------------------------------------------------
uint8_t trm_read_bit(void)
    {
    uint8_t bit=0;

    cli();  //<<< disable all interrupts <<<

    DAT_OUTPUT_LOW;
    _delay_us(1);

    DAT_INPUT_PULL;
    _delay_us(14);

    if(DAT_READ) bit=1;

    _delay_us(45);

    sei();
    return bit;
    }


//-----------------------------------------------------------------------------
void trm_read_byte(uint8_t *b) //read byte
    {
    for(uint8_t k=0; k<8; k++) *b|=(trm_read_bit()<<k);
    }


//-----------------------------------------------------------------------------
void trm_write_byte(uint8_t byte) //send byte
    {
    for(uint8_t mask=1; mask; mask<<=1) trm_write_bit(byte&mask);
    }


//-----------------------------------------------------------------------------
void trm_get_temp(uint8_t *arr)  //array[3]: 0-"+/-", 1-"XX.x", 2-"xx.X"
    {
    uint8_t reg0 = 0;
    uint8_t reg1 = 0;

    trm_reset();
    trm_write_byte(CMD_SKIPROM);
    trm_write_byte(CMD_RSCRATCHPAD);

    trm_read_byte(&reg0);
    trm_read_byte(&reg1);

    if(reg1 & 0x80)  //sign: "-"
        {
        arr[0]=1;

        uint16_t tmp = (~((reg1<<8)|reg0))+1;

        reg0 = tmp;
        reg1 = tmp>>8;
        }
    else arr[0]=0;  //"+"

    arr[1] = ((reg1 & 0b111)<<4)|(reg0>>4);  //temperature: "XX.x"
    arr[2] = (((reg0 & 0b1111)<<1)+((reg0 & 0b1111)<<3))>>4; //"xx.X"

    trm_reset();
    trm_write_byte(CMD_SKIPROM);
    trm_write_byte(CMD_CONVERTTEMP);
    }

