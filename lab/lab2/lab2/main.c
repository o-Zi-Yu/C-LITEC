// ENGR-2350 Template Project
// Name: Yuhao Li
// RIN: 661982653
// This is the base project for several activities and labs throughout
// the course.  The outline provided below isn't necessarily *required*
// by a C program; however, this format is required within ENGR-2350
// to ease debugging/grading by the staff.

// We'll always add this include statement. This basically takes the
// code contained within the "engr_2350_msp432.h" file and adds it here.
#include "engr2350_msp432.h"

// Add function prototypes here, as needed.
void Timer_Init();
void GPIOInit();
void GameStart();
void RGBLogic();
void countdown();
// Add global variables here, as needed.
Timer_A_UpModeConfig A1;
uint8_t time[4];
uint32_t ssflag = 0;
uint8_t r,g,b;

// Main Function
int main(void)
{
    // Add local variables here, as needed.

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();

    // Place initialization code (or run-once) code here
    Timer_Init();
    GPIOInit();
    GameStart();
    while(1){
        RGBLogic();
    }
}

// Add function declarations here as needed
void Timer_Init(){
    A1.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    A1.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    A1.timerPeriod = 37500;
    A1.timerClear = TIMER_A_DO_CLEAR;
    Timer_A_configureUpMode(TIMER_A0_BASE,&A1);
    Timer_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);
}

void GPIOInit(){
    GPIO_setAsInputPin(GPIO_PORT_P2,GPIO_PIN3); //pb
    GPIO_setAsInputPin(GPIO_PORT_P3,GPIO_PIN2); //ss
    GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN0|GPIO_PIN1|GPIO_PIN2); //RGBLED
    //red 2.0 green 2.1 blue 2.2 yellow 2.0&2.1 purple 2.0&2.2 cyan 2.1&2.2
    GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN4|GPIO_PIN5); //BiLED
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4,GPIO_PIN0|GPIO_PIN2|GPIO_PIN3); //bp
    //red 4.0 green 4.2 blue 4.3 yellow 4.0&4.2 purple 4.0&4.3 cyan 4.2&4.3
}

void GameStart(){
    printf("This is instruction.");
    //wait for ss toggle
    while(1){
        if(ssflag == 0){
            if(GPIO_getInputPinValue(GPIO_PORT_P3,GPIO_PIN2)){
                ssflag = 1;
                break;
            }
        }
        if(ssflag == 1){
            if(!GPIO_getInputPinValue(GPIO_PORT_P3,GPIO_PIN2)){
                ssflag = 0;
                break;
            }
        }
        r = 0; g = 0; b = 0;
        countdown();
    }
    //3 seconds count down

}

void countdown(){
    uint8_t a = 3;
    while(1){
        uint16_t counter_value = 0;
        printf("%d..\r\n", a);
        // Place code that runs continuously in here
        if(counter_value==Timer_A_getCounterValue(TIMER_A0_BASE)){
            a -= 1;
            if(a == 0){break;}
        }
    }

}

void RGBLogic(){
    //Red
    if(GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0)&&!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN2)&&!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN3)){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN1);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN2);
        r = 1;
        __delay_cycles(240e3);
    }
    //Green
    if(!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0)&&GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN2)&&!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN3)){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN2);
        g = 1;
        __delay_cycles(240e3);
    }
    //Blue
    if(!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0)&&!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN2)&&GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN3)){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN1);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN0);
        b = 1;
        __delay_cycles(240e3);
    }
    /*
    //Yellow
    if(GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0)&&GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN2)&&!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN3)){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0);
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN2);
    }
    //Purple
    if(GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0)&&!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN2)&&GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN3)){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN1);
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2);
    }
    */
}


// Add interrupt functions last so they are easy to find
