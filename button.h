// button.h


#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#include "bitm.h"


// TIMEx10ms for 100Hz interrupt
#define DEB_TIME 5
#define HLD_TIME 60


///////////////////////////////////////////////////////////////////////////////

#define B1_BIT   PIN7
#define B1_PORT  PORTD
#define B1_DDR   DDRD
#define B1_PIN   PIND

///////////////////////////////////////////////////////////////////////////////


#define B1_CLR   (CLR_BIT(B1_PORT, B1_BIT))
#define B1_SET   (SET_BIT(B1_PORT, B1_BIT))
#define B1_OUT   (SET_BIT(B1_DDR, B1_BIT))
#define B1_INP   (CLR_BIT(B1_DDR, B1_BIT))


#define BUTTON_PRESS    (BIT_IS_CLR(B1_PIN, B1_BIT))


#define BUTTON_INIT   B1_SET; B1_INP


volatile uint8_t button1; //0-not pressed, 1-short press, 2-long press


#define BUTTON_LONG  2
#define BUTTON_SHORT 1



//-----------------------------------------------------------------------------
void wait_button(void)
    {
    while(BUTTON_PRESS);
    _delay_ms(50);
    button1=0;
    }


//-----------------------------------------------------------------------------
void button_cont(void)
    {
    static uint8_t tmp1; //flags

    static uint8_t deb1; //counters
    static uint8_t hld1;

    if(BUTTON_PRESS)
        {
        if(deb1<DEB_TIME)
            {
            if(++deb1==DEB_TIME) tmp1=1;
            }
        if(tmp1==1)
            {
            if(hld1<HLD_TIME)
                {
                hld1++;
                if(hld1==HLD_TIME)
                    {
                    tmp1=0;
                    button1=2;
                    hld1=0;
                    }
                }
            }
        }
    else
        {
        if(deb1>0)
            {
            deb1--;
            if(deb1==0 && tmp1==1)
                {
                tmp1=0;
                button1=1;
                hld1=0;
                }
            }
        }
    }
