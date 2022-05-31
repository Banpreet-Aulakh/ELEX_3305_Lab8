
#include "msp.h"
#include "i2c.h"
#include "lcd.h"

volatile int X = 128;
#define PERIOD 37750 //200 ms*3.00Mhz/16

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
    i2cInit();

    lcdInit();
    lcdClear(BLACK);

    //initialize OPT3001
    i2cWrite(OPT3001_SLAVE_ADDRESS, OPT3001_CONFIG_REG, OPT3001_CONFIG_100);

    //initialize TIMER_A0
    TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_ID__8 | TIMER_A_CTL_MC_1;
    TIMER_A0->EX0 = TIMER_A_EX0_IDEX__2;

    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE;

    TIMER_A0->CCR[0] = PERIOD - 1;

    //enable Timer_A0_Channel_0 interrupt in the NVIC
    NVIC_EnableIRQ(TA0_0_IRQn);

    //enable global interrupt
    _enable_interrupts();

    while(1){}
}


void TA0_0_IRQHandler(void)
{
    static double tvalue;
    static int ledvalue;

    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

    X--;

    if (X < 0)
    {
        lcdClear(BLACK);
        X = LCD_MAX_X;
    }

    tvalue = (((double) i2cRead(TMP006_SLAVE_ADDRESS, TMP006_TEMPERATURE_REG)) / 10.0);

    lcdSetPixel(X, tvalue, CYAN);

    ledvalue = i2cRead(OPT3001_SLAVE_ADDRESS, OPT3001_RESULT_REG);

    ledvalue = ledvalue / 256;

    lcdSetPixel(X, ledvalue, WHITE);

}
