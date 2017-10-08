// LED Clock v2
//
// ATmega328P @ 8MHz (int. RC osc.)
//
// fuses  hex
// low    e2
// high   d9
// ext.   07
//
// toxcat :3  08-Oct-2017
//



#include <stdio.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "tm1637.h"
#include "ds1307.h"
#include "at24c32.h"
#include "button.h"
#include "ds18b20.h"



#define TMR1_INT_HZ     100
#define TMR1_PRESCALER  256
#define TIMER1_PRELOAD  (65536-(F_CPU/TMR1_PRESCALER)/TMR1_INT_HZ)


//brightness 0-3(7)
#define DIMM_VAL   2

//correction sign, 0:"+" 1:"-"
#define TCOR_SIGN  1

//correction val., 0..9 sec.
#define TCOR1_VAL   3
#define TCOR2_VAL   3

//address in eeprom (external 24c32)
#define DIMM_VADDR   1024
#define TCOR_SADDR   1025
#define TCOR1_VADDR  1026
#define TCOR2_VADDR  1027


volatile uint8_t scrtim=0;
volatile uint8_t trmtim=0;

extern uint8_t temp[3]; //ds18b20 data

extern uint8_t rtc[8]; //ds1307 data

extern char strbuff[8];



//-----------------------------------------------------------------------------
ISR(TIMER1_OVF_vect) //Timer1 overflow interrupt
    {
    TCNT1=TIMER1_PRELOAD;

    if(scrtim) scrtim--;
    if(trmtim) trmtim--;

    button_cont();
    }


//-----------------------------------------------------------------------------
void setup(void)
    {
    CLKPR=0;
    SREG=0;

    ACSR=0b10000000; //analog comparator disable
    ADMUX=0;
    ADCSRA=0; //ADC disable

    TIMSK1=0; //timer1 OFF
    TCCR1A=0; //OC1A/OC1B disconnected
    TCCR1B=0b00000100; //prescaler 8M/256=31250
    TCCR1C=0;
    TCNT1=TIMER1_PRELOAD;
    TIMSK1=0b00000001; //Timer1 ON

    TIMSK2=0; //timer2 OFF
    TCCR2A=0;
    TCCR2B=0;
    TCNT2=0;

    sei();

    BUTTON_INIT;

    TM1637_SERIAL_INIT;

    i2c_init();
    rtc_init();

/*  //_debug
    led_print(0,"CLEE"); led_update(0); ee_clear();  //clear ext. eeprom

    if(ee_read(DIMM_VADDR)==0xff) ee_write(DIMM_VADDR, DIMM_VAL);
    if(ee_read(TCOR_SADDR)==0xff) ee_write(TCOR_SADDR, TCOR_SIGN);
    if(ee_read(TCOR1_VADDR)==0xff) ee_write(TCOR1_VADDR, TCOR1_VAL);
    if(ee_read(TCOR2_VADDR)==0xff) ee_write(TCOR2_VADDR, TCOR2_VAL);
*/
    led_dots(1);
    led_update(0);
    _delay_ms(200);

    led_print(0," -- ");
    led_update(0);
    _delay_ms(200);

    led_print(0,"----");
    led_update(0);
    _delay_ms(230);

    wait_button();
    }


//-----------------------------------------------------------------------------
int main(void)
    {
    uint8_t mod=0;
    uint8_t set=0;
    uint8_t dat=0;
    uint8_t sys=0;

    uint8_t tmp=0;

    uint8_t wascor=0;  //flag
    uint8_t corprep=0;  //flag
    uint8_t upd=0;

    setup();

    uint8_t dimm = ee_read(DIMM_VADDR);
    uint8_t cors = ee_read(TCOR_SADDR);
    uint8_t cor1 = ee_read(TCOR1_VADDR);
    uint8_t cor2 = ee_read(TCOR2_VADDR);

    while(1)  // main loop
        {
        if(trmtim==0)
            {
            trm_get_temp(temp);
            trmtim=100;
            }

        if(scrtim==0)
            {
            rtc_read(rtc);

            /*************************************  correction  ***************************************/

            if(rtc[2]==3 && rtc[1]==30 && rtc[0]==29 && wascor==0 && set==0 && corprep==0) corprep=1;

            if(corprep==1 && rtc[SECONDS_REG]==30)
                {
                tmp=rtc[0];

                if(cors) tmp-=cor1;
                else tmp+=cor1;
                if(cor1) upd=1;

                if(rtc[DAY_REG]==SUNDAY)
                    {
                    if(cors) tmp-=cor2;
                    else tmp+=cor2;
                    if(cor2) upd=1;
                    }

                if(upd==1) { upd=0; rtc_set_sec(tmp); }

                corprep=0;
                wascor=1;
                }

            if(wascor==1 && rtc[1]==31) wascor=0;

            /**********************************  end of correction  ***********************************/


            /*************************************  display  ******************************************/
            led_dots(0);

            switch(mod)
                {
                case 0:
                    switch(set)
                        {
                        case 0: sprintf(strbuff,"%02u%02u",rtc[2],rtc[1]); led_dots(1); break; //hrs:min
                        case 1: sprintf(strbuff,"__%02u",rtc[0]); break; //sec
                        case 2: sprintf(strbuff,"%02u  ",rtc[2]); led_dots(1); break; //hrs
                        case 3: sprintf(strbuff,"  %02u",rtc[1]); led_dots(1); break; //min
                        case 4: sprintf(strbuff,"du%2u",rtc[3]); break; //day of week
                        case 5: sprintf(strbuff,"dn%2u",rtc[4]); break; //day of month
                        case 6: sprintf(strbuff,"no%2u",rtc[5]); break; //month
                        case 7: sprintf(strbuff,"ur%02u",rtc[6]); break; //year
                        }
                    break;
                case 1:
                    switch(dat)
                        {
                        case 0: sprintf(strbuff,"  %02u",rtc[0]); led_dots(1); break;  //:sec
                        case 1: sprintf(strbuff,"d%3u",rtc[3]); break;    //day of week
                        case 2: sprintf(strbuff,"%2u%2u",rtc[4],rtc[5]); break;  //date/month
                        case 3: sprintf(strbuff,"20%02u",rtc[6]); break;  //year
                        }
                    break;
                case 2:
                    switch(sys)
                        {
                        case 0: sprintf(strbuff,"%3u",temp[1]); led_dat(3,0b01100011); break;  //temperature
                        case 1:  //correction 1
                            if(cors) { sprintf(strbuff,"c1-%1u", cor1); }
                            else { sprintf(strbuff,"c1P%1u", cor1); }
                            break;
                        case 2:  //correction 2
                            if(cors) { sprintf(strbuff,"c2-%1u", cor2); }
                            else { sprintf(strbuff,"c2P%1u", cor2); }
                            break;
                        case 3: sprintf(strbuff,"L%3u", dimm); break;  //brightness
                        }
                    break;
                }

            led_print(0,strbuff);

            led_update(dimm);

            /**********************************  end of display  **************************************/


            if((mod==0 && set==1) || (mod==1 && dat==0) || corprep==1) scrtim=10;  //set timer
            else scrtim=50;
            }

        switch(button1)
            {
            case BUTTON_SHORT:  /*>>>>>>>>>>>>>>>>>>>>>> BUTTON_SHORT >>>>>>>>>>>>>>>>>>>>>>>>>>*/
                switch(mod)
                    {
                    case 0:
                        switch(set)  //change settings
                            {
                            case 0: if(++mod>2) mod=0; break; //change screen mode
                            case 1: rtc_set_sec(0); break; //reset sec
                            case 2: tmp=rtc[2]; if(++tmp>59) tmp=0; rtc_set_hrs(tmp); break; //set hrs
                            case 3: tmp=rtc[1]; if(++tmp>59) tmp=0; rtc_set_min(tmp); break; //set min
                            case 4: tmp=rtc[3]; if(++tmp>7) tmp=1; rtc_set_day(tmp); break; //set day
                            case 5: tmp=rtc[4]; if(++tmp>31) tmp=1; rtc_set_dat(tmp); break; //set date
                            case 6: tmp=rtc[5]; if(++tmp>12) tmp=1; rtc_set_mon(tmp); break; //set month
                            case 7: tmp=rtc[6]; if(++tmp>99) tmp=0; rtc_set_year(tmp); break; //set year
                            }
                        break;
                    case 1:
                        if(++mod>2) mod=0; //change screen mode
                        break;
                    case 2:
                        switch(sys)
                            {
                            case 0: if(++mod>2) mod=0; break; //change screen mode
                            case 1: //set corr 1
                                if(++cor1>9)
                                    {
                                    cor1=0;
                                    if(cors) cors=0;
                                    else cors=1;
                                    ee_write(TCOR_SADDR, cors);
                                    }
                                ee_write(TCOR1_VADDR, cor1);
                                break;
                            case 2: //set corr 2
                                if(++cor2>9) cor2=0;
                                ee_write(TCOR2_VADDR, cor2);
                                break;
                            case 3: //set brightness
                                if(++dimm>3) dimm=0;
                                ee_write(DIMM_VADDR, dimm);
                                break;
                            }
                        break;
                    }
                button1=0;
                scrtim=0;
                break;  //end of BUTTON_SHORT

            case BUTTON_LONG:  /*>>>>>>>>>>>>>>>>>>>>>>> BUTTON_LONG >>>>>>>>>>>>>>>>>>>>>>>>>>>*/
                switch(mod)
                    {
                    case 0: if(++set>7) set=0; break;
                    case 1: if(++dat>3) dat=0; break;
                    case 2: if(++sys>3) sys=0; break;
                    }
                button1=0;
                scrtim=0;
                break;
            }  //end of button
        }  //end of main loop
    }


