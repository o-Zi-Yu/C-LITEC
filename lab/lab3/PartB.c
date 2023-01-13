// ENGR-2350 Template Project
// Name: Yuhao Li
//      Ziyu Zhu
// RIN: 661982653
//      661978924
// This is the base project for several activities and labs throughout
// the course.  The outline provided below isn't necessarily *required*
// by a C program; however, this format is required within ENGR-2350
// to ease debugging/grading by the staff.

// We'll always add this include statement. This basically takes the
// code contained within the "engr_2350_msp432.h" file and adds it here.
#include "engr2350_msp432.h"
//#include "stdio.h"

// Add function prototypes here, as needed.
void Timer_Init();
void GPIOInit();
void Timer_ISR();
void Timer_ISR2();
void SpeedControl();
void DirectionControl();
void EnableControl();
void Drive(uint16_t * actions,uint16_t * times);
// Add global variables here, as needed.
Timer_A_UpModeConfig A0,A1;
Timer_A_CompareModeConfig A3,A4;
uint8_t HighSpeed;
uint8_t Left;
uint8_t On;
uint16_t countdown_timer;
uint16_t checkpoint;
uint16_t ti;
uint16_t ti2;
uint8_t mode;

uint16_t actions_R[] = {9,3,1,3,1,3,1,3,0,3};
uint16_t times_R[] = {90,6,42,6,42,6,41,8,60};
uint16_t actions_I[] = {11,3,0,3,1,3,0,3,0,3,1,3};
uint16_t times_I[] = {40,12,20,7,45,1,45,6,20,12,41};
// Main Function
int main(void) {
    // Add local variables here, as needed.

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();

    // Place initialization code (or run-once) code here
    Timer_Init();
    GPIOInit();
    while (1) {
        if(!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0)){
            __delay_cycles(1e6);
            Drive(actions_R, times_R);
        }else if(!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN7)){
            __delay_cycles(1e6);
            Drive(actions_I, times_I);
        }

    }
}

// Add function declarations here as needed
void Timer_Init() {
    //first timer
    A0.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    A0.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    A0.timerPeriod = 960;
    A0.timerClear = TIMER_A_DO_CLEAR;
    A0.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureUpMode(TIMER_A0_BASE, &A0);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    Timer_A_registerInterrupt(TIMER_A0_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Timer_ISR);

    A3.compareRegister=TIMER_A_CAPTURECOMPARE_REGISTER_3;
    A3.compareOutputMode=TIMER_A_OUTPUTMODE_RESET_SET;
    A3.compareValue=258;
    A4.compareRegister=TIMER_A_CAPTURECOMPARE_REGISTER_4;
    A4.compareOutputMode=TIMER_A_OUTPUTMODE_RESET_SET;
    A4.compareValue=240;
    Timer_A_initCompare(TIMER_A0_BASE, &A3);
    Timer_A_initCompare(TIMER_A0_BASE, &A4);

    //second timer
    A1.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    A1.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    A1.timerPeriod = 37500;
    A1.timerClear = TIMER_A_DO_CLEAR;
    A1.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureUpMode(TIMER_A1_BASE, &A1);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    Timer_A_registerInterrupt(TIMER_A1_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Timer_ISR2);

}

void Timer_ISR(){
    if(Timer_A_getEnabledInterruptStatus(TIMER_A0_BASE) == TIMER_A_INTERRUPT_PENDING){
        Timer_A_clearInterruptFlag(TIMER_A0_BASE);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN6);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN7);
    }else if(Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3)){
        Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3);
        Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4);
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
    }
}

void Timer_ISR2() {
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    ti++;
    if(ti == 10){
        ti = 0;
        ti2++;
        //printf("%d\r\n",ti2);
    }
}


void GPIOInit() {
    //motor enable
    GPIO_setAsOutputPin(GPIO_PORT_P3,GPIO_PIN7|GPIO_PIN6);
    //motor direction
    GPIO_setAsOutputPin(GPIO_PORT_P5,GPIO_PIN4|GPIO_PIN5);
    //motor speed
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION); //TA0.4 CCR4 of 0
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION); //TA0.3 CCR3 of 0

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4,GPIO_PIN0|GPIO_PIN7);
}

void SpeedControl(){
    if (HighSpeed == 0){
        Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, 251);
        Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, 240);
    }else{
        Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, 720);
        Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, 720);
    }
}

void DirectionControl(){
    if (Left == 0){
        //left
        GPIO_setOutputHighOnPin( GPIO_PORT_P5, GPIO_PIN4);
        GPIO_setOutputLowOnPin( GPIO_PORT_P5, GPIO_PIN5);
    }else if (Left == 1){
        //right
        GPIO_setOutputLowOnPin( GPIO_PORT_P5, GPIO_PIN4);
        GPIO_setOutputHighOnPin( GPIO_PORT_P5, GPIO_PIN5);
    }else if (Left == 2){
        //backward
        GPIO_setOutputHighOnPin( GPIO_PORT_P5, GPIO_PIN4);
        GPIO_setOutputHighOnPin( GPIO_PORT_P5, GPIO_PIN5);
    }else if (Left == 3){
        //forward
        GPIO_setOutputLowOnPin( GPIO_PORT_P5, GPIO_PIN4);
        GPIO_setOutputLowOnPin( GPIO_PORT_P5, GPIO_PIN5);
    }
}

void EnableControl(){
    if (On == 0){
        GPIO_setOutputLowOnPin( GPIO_PORT_P3, GPIO_PIN7);
        GPIO_setOutputLowOnPin( GPIO_PORT_P3, GPIO_PIN6);
    }else{
        GPIO_setOutputHighOnPin( GPIO_PORT_P3, GPIO_PIN7);
        GPIO_setOutputHighOnPin( GPIO_PORT_P3, GPIO_PIN6);
    }
}

void Drive(uint16_t * actions,uint16_t * times){
    On = 1;
    HighSpeed = 0;
    EnableControl();
    SpeedControl();
    ti = 0;
    ti2 = 0;
    checkpoint = actions[0];
    uint8_t i = 0;
    uint8_t j = 1;
while (checkpoint > 0){
    //from 1 to 2
    ti = 0;
    countdown_timer = times[i];
    while(countdown_timer > 0)
    {
        Left = actions[j];
        DirectionControl();
        if (ti == 1){
            printf("ti= %d..\r\n",ti2);
            ti = 0;
            printf("countdown= %d..\r\n",countdown_timer);
            countdown_timer -= 1;
        }
    }
    j++;
    i++;
    checkpoint -= 1;
}
On = 0;
EnableControl();
}



