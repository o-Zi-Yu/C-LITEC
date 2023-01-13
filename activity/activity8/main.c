// ENGR-2350 Template Project
// Name: Ziyu Zhu
// RIN: 661978924
// This is the base project for several activities and labs throughout
// the course.  The outline provided below isn't necessarily *required*
// by a C program; however, this format is required within ENGR-2350
// to ease debugging/grading by the staff.

// We'll always add this include statement. This basically takes the
// code contained within the "engr_2350_msp432.h" file and adds it here.

#include "engr2350_msp432.h"

// Add function prototypes here, as needed.
void GPIO_Init();
void Timer_Init();
void Update_Time();
void Timer_ISR();
void Port3_ISR();

Timer_A_UpModeConfig config;
uint8_t time[4]; // [tenths of seconds,seconds,minutes,hours]
uint16_t flag = 0;

// Main Function
int main(void)
{
    SysInit();
    Timer_Init();
    GPIO_Init();
    printf("\r\n\n Real Time Clock:\r\n");

    while(1){
        // Check to see if the timer counted a full cycle
        if(flag){
            // Update and print the clock.
            flag = 0;
        }
    }
}

void GPIO_Init(){
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4,GPIO_PIN0|GPIO_PIN2);
    GPIO_registerInterrupt(GPIO_PORT_P4,Port3_ISR);
    GPIO_interruptEdgeSelect(GPIO_PORT_P4,GPIO_PIN0,GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(GPIO_PORT_P4,GPIO_PIN2,GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_enableInterrupt(GPIO_PORT_P4,GPIO_PIN0);
    GPIO_enableInterrupt(GPIO_PORT_P4,GPIO_PIN2);
}

void Port3_ISR(){
    __delay_cycles(240e3); // 10 ms delay (24 MHz clock)

        // Next: get the list of pins that may have triggered the interrupt
    uint8_t active_pins = GPIO_getEnabledInterruptStatus(GPIO_PORT_P4);

    if(active_pins & GPIO_PIN0){
        GPIO_clearInterruptFlag(GPIO_PORT_P4,GPIO_PIN0);
        if(!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN0)){
            time[3]++;
            if(time[3] == 24){
                time[3] = 0;
            }
        }
    }
    if(active_pins & GPIO_PIN2){
        GPIO_clearInterruptFlag(GPIO_PORT_P4,GPIO_PIN2);
        if(!GPIO_getInputPinValue(GPIO_PORT_P4,GPIO_PIN2)){
            time[2]++;
            if(time[2] == 60){
                time[2] = 0;
            }
        }
    }
    // Repeat as necessary for more pins
}

void Timer_Init(){
    // Set the timer configuration
    config.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    config.timerPeriod = 37500;
    config.timerClear = TIMER_A_DO_CLEAR;
    config.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    // Apply the configuration
    Timer_A_configureUpMode(TIMER_A0_BASE,&config);
    //register interrupt
    Timer_A_registerInterrupt(TIMER_A0_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,Timer_ISR);
    // Start the timer
    Timer_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);
}

void Update_Time(){
    time[0]++;
    if(time[0] == 10){
        time[0] = 0;
        time[1]++;
        if(time[1] == 60){
            time[1] = 0;
            time[2]++;
            if(time[2] == 60){
                time[2] = 0;
                time[3]++;
                if(time[3] == 24){
                    time[3] = 0;
                }
            }
        }
    }
    printf("%2u:%02u:%02u.%u\r",time[3],time[2],time[1],time[0]);
}

void Timer_ISR(){ // Name doesn't matter
    flag = 1;
    Timer_A_clearInterruptFlag(TIMER_A0_BASE);   // acknowledge the interrupt
    Update_Time();
}
