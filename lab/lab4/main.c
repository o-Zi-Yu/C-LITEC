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
void Encoder_ISR();
void SpeedControl();
void DirectionControl();
void EnableControl();
void Drive(uint16_t* actions, uint16_t* times);
void PWM_set();
// Add global variables here, as needed.
Timer_A_ContinuousModeConfig A0;
Timer_A_UpModeConfig A1;
Timer_A_CaptureModeConfig A2, A3;
Timer_A_CompareModeConfig A4, A5;
uint8_t HighSpeed;
uint8_t Left;
uint8_t On;
uint16_t countdown_timer;
uint16_t checkpoint;
uint16_t ti;
uint16_t ti2;
uint8_t mode;
uint8_t count1, count2;
uint16_t PWM_wheel1 = 240;
uint16_t PWM_wheel2 = 240;
uint16_t distance;

uint32_t enc_total_wheel1, enc_total_wheel2;
int32_t enc_counts_track1, enc_counts_track2;
uint32_t enc_counts_wheel1, enc_counts_wheel2;
uint32_t enc_counts_wheel1_sum, enc_counts_wheel2_sum;
uint16_t enc_flag_wheel1, enc_flag_wheel2;

uint16_t actions_R[] = {3, 1, 3, 1, 3, 1, 3, 0, 3};
uint16_t dis_R[] = {1830, 81, 860, 81, 855, 81, 865, 135, 1300};
uint16_t actions_I[] = {3, 0, 3, 1, 3, 0, 3, 1, 3};
uint16_t dis_I[] = {860, 186, 430, 84, 1830, 88, 430, 173, 860};
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
        if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0)) {
            __delay_cycles(1e6);
            Drive(actions_R, dis_R);
        } else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7)) {
            __delay_cycles(1e6);
            Drive(actions_I, dis_I);
        }
    }
}

// Add function declarations here as needed
void Timer_Init() {
    // first timer
    A0.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    A0.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    A0.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    A0.timerClear = TIMER_A_DO_CLEAR;
    Timer_A_configureContinuousMode(TIMER_A3_BASE, &A0);
    Timer_A_registerInterrupt(TIMER_A3_BASE, TIMER_A_CCR0_INTERRUPT, Encoder_ISR);
    Timer_A_registerInterrupt(TIMER_A3_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Encoder_ISR);

    A1.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    A1.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    A1.timerPeriod = 960;
    A1.timerClear = TIMER_A_DO_CLEAR;
    A1.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureUpMode(TIMER_A0_BASE, &A1);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    Timer_A_registerInterrupt(TIMER_A0_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Timer_ISR);

    A4.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
    A4.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    A4.compareValue = 240;
    A5.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;
    A5.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    A5.compareValue = 240;
    Timer_A_initCompare(TIMER_A0_BASE, &A4);
    Timer_A_initCompare(TIMER_A0_BASE, &A5);

    A2.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
    A2.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
    A2.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
    A2.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
    A2.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;

    A3.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    A3.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
    A3.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
    A3.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
    A3.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;

    Timer_A_enableCaptureCompareInterrupt(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
    Timer_A_enableCaptureCompareInterrupt(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);
    Timer_A_initCapture(TIMER_A3_BASE, &A2);
    Timer_A_initCapture(TIMER_A3_BASE, &A3);
    Timer_A_startCounter(TIMER_A3_BASE, TIMER_A_CONTINUOUS_MODE);

    // second timer
    //  A1.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    //  A1.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    //  A1.timerPeriod = 37500;
    //  A1.timerClear = TIMER_A_DO_CLEAR;
    //  A1.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    //  Timer_A_configureUpMode(TIMER_A1_BASE, &A1);
    //  Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    //  Timer_A_registerInterrupt(TIMER_A1_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Timer_ISR2);

    /*.
    A0.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    A0.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    A0.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    A0.timerClear = TIMER_A_DO_CLEAR;
    Timer_A_configureContinuousMode(TIMER_A3_BASE, &A0);
    Timer_A_registerInterrupt(TIMER_A3_BASE, TIMER_A_CCR0_INTERRUPT, Encoder_ISR);
    Timer_A_registerInterrupt(TIMER_A3_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Encoder_ISR);



    A1.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
    A1.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
    A1.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
    A1.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
    A1.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
    Timer_A_enableCaptureCompareInterrupt(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
    Timer_A_initCapture(TIMER_A3_BASE, &A1);
    Timer_A_startCounter(TIMER_A3_BASE, TIMER_A_CONTINUOUS_MODE);
    */
}

void Timer_ISR() {
    if (Timer_A_getEnabledInterruptStatus(TIMER_A0_BASE) == TIMER_A_INTERRUPT_PENDING) {
        Timer_A_clearInterruptFlag(TIMER_A0_BASE);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN6);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
    } else if (Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3)) {
        Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3);
        Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4);
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);
        GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
    }
}

void Timer_ISR2() {
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    ti++;
    if (ti == 10) {
        ti = 0;
        ti2++;
        // printf("%d\r\n",ti2);
    }
}

void GPIOInit() {
    // motor enable
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN7 | GPIO_PIN6);
    // motor direction
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN4 | GPIO_PIN5);
    // motor speed
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);  // TA0.4 CCR4 of 0
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);  // TA0.3 CCR3 of 0

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN7);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN3 | GPIO_PIN5);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);  // right
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);  // left
}

void SpeedControl() {
    if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3)) {
        __delay_cycles(1e6);
        HighSpeed = 0;
    } else if (!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5)) {
        __delay_cycles(1e6);
        HighSpeed = 1;
    }
    if (HighSpeed) {
        Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, PWM_wheel1 /2);
        Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, PWM_wheel2 /2);
    } else {
        Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, PWM_wheel1);
        Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, PWM_wheel2);
    }
}

void DirectionControl() {
    if (Left == 0) {
        // left
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN4);
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN5);
    } else if (Left == 1) {
        // right
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN5);
    } else if (Left == 2) {
        // backward
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN4);
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN5);
    } else if (Left == 3) {
        // forward
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN5);
    }
}

void EnableControl() {
    if (On == 0) {
        GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN7);
        GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);
    } else {
        GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN7);
        GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN6);
    }
}

void Drive(uint16_t* actions, uint16_t* times) {
    On = 1;
    HighSpeed = 0;
    EnableControl();
    // SpeedControl();
    ti = 0;
    ti2 = 0;
    count1 = 0;
    count2 = 0;
    enc_counts_wheel1_sum = 0;
    enc_counts_wheel2_sum = 0;
    checkpoint = 0;
    distance = 0;
    enc_total_wheel1 = 0;
    enc_total_wheel2 = 0;
    while (checkpoint < 9) {
        while (1) {
            PWM_set();
            SpeedControl();
            Left = actions[checkpoint];
            DirectionControl();
            if (checkpoint % 2 == 0) {
                distance = 3.14 * 70 * enc_total_wheel1 / 360;
                // printf("distance: %d\r\n", distance);
                if (distance >= times[checkpoint]) {
                    enc_total_wheel1 = 0;
                    break;
                }
            } else {
                distance = 75 * enc_total_wheel1 / 149;
                // printf("angle: %d\r\n", distance);
                if (distance >= times[checkpoint]) {
                    enc_total_wheel1 = 0;
                    break;
                }
            }
        }
        checkpoint += 1;
    }
    // uint8_t i = 0;
    // uint8_t j = 1;
    // while (checkpoint > 0) {
    //     // from 1 to 2
    //     ti = 0;
    //     countdown_timer = times[i];
    //     while (countdown_timer > 0) {
    //         Left = actions[j];
    //         DirectionControl();
    //         if (ti == 1) {
    //             printf("ti= %d..\r\n", ti2);
    //             ti = 0;
    //             printf("countdown= %d..\r\n", countdown_timer);
    //             countdown_timer -= 1;
    //         }
    //     }
    //     j++;
    //     i++;
    //     checkpoint -= 1;
    // }
    On = 0;
    EnableControl();
}

void PWM_set() {
    // right
    if (HighSpeed) {
        if (enc_flag_wheel1) {    // Check to see if capture occurred
            enc_flag_wheel1 = 0;  // reset capture flag
            count1 += 1;
            enc_counts_wheel1_sum += enc_counts_wheel1;
            if (count1 == 6) {
                if (enc_counts_wheel1_sum / 6 > 65000*2) {
                    PWM_wheel1 += 2;
                }
                if (enc_counts_wheel1_sum / 6 < 65000*2) {
                    PWM_wheel1 -= 2;
                }
                // printf("1: %d, %d", enc_counts_wheel1_sum / 6, PWM_wheel1);
                count1 = 0;
                enc_counts_wheel1_sum = 0;
                // Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, PWM_wheel1);
            }
        }                         // left
        if (enc_flag_wheel2) {    // Check to see if capture occurred
            enc_flag_wheel2 = 0;  // reset capture flag
            count2 += 1;
            enc_counts_wheel2_sum += enc_counts_wheel2;
            if (count2 == 6) {
                if (enc_counts_wheel2_sum / 6 > 65500*2) {
                    PWM_wheel2 += 2;
                }
                if (enc_counts_wheel2_sum / 6 < 65500*2) {
                    PWM_wheel2 -= 2;
                }
                // printf("   2: %d, %d\r\n", enc_counts_wheel2_sum / 6, PWM_wheel2);
                count2 = 0;
                enc_counts_wheel2_sum = 0;
                // Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, PWM_wheel2);
            }
        }
    } else {
        if (enc_flag_wheel1) {    // Check to see if capture occurred
            enc_flag_wheel1 = 0;  // reset capture flag
            count1 += 1;
            enc_counts_wheel1_sum += enc_counts_wheel1;
            if (count1 == 6) {
                if (enc_counts_wheel1_sum / 6 > 65000) {
                    PWM_wheel1 += 1;
                }
                if (enc_counts_wheel1_sum / 6 < 65000) {
                    PWM_wheel1 -= 1;
                }
                // printf("1: %d, %d", enc_counts_wheel1_sum / 6, PWM_wheel1);
                count1 = 0;
                enc_counts_wheel1_sum = 0;
                // Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, PWM_wheel1);
            }
        }                         // left
        if (enc_flag_wheel2) {    // Check to see if capture occurred
            enc_flag_wheel2 = 0;  // reset capture flag
            count2 += 1;
            enc_counts_wheel2_sum += enc_counts_wheel2;
            if (count2 == 6) {
                if (enc_counts_wheel2_sum / 6 > 65500) {
                    PWM_wheel2 += 1;
                }
                if (enc_counts_wheel2_sum / 6 < 65500) {
                    PWM_wheel2 -= 1;
                }
                // printf("   2: %d, %d\r\n", enc_counts_wheel2_sum / 6, PWM_wheel2);
                count2 = 0;
                enc_counts_wheel2_sum = 0;
                // Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, PWM_wheel2);
            }
        }
    }
    printf("1: %d, 2: %d\r\n", PWM_wheel1, PWM_wheel2);
}

void Encoder_ISR() {
    /*Check to see if timer reset/overflow has occurred. If yes:
    Clear the overflow interrupt flag
    Add 65536 to enc_counts_track
Check to see if a capture event ocurred. If yes:
    Clear the capture event interrupt flag
    Increment the enc_total (total number of capture events that have occurred)
    Calculate the enc_counts as enc_counts_track + capture value
    Set enc_counts_track to -(capture value)
    Set enc_flag to 1*/
    if (Timer_A_getEnabledInterruptStatus(TIMER_A3_BASE) == TIMER_A_INTERRUPT_PENDING) {
        Timer_A_clearInterruptFlag(TIMER_A3_BASE);
        enc_counts_track1 += 65536;
        enc_counts_track2 += 65536;
    }
    // right
    if (Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0) == TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG) {
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
        enc_total_wheel1++;
        enc_counts_wheel1 = enc_counts_track1 + Timer_A_getCaptureCompareCount(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
        enc_counts_track1 = 0 - Timer_A_getCaptureCompareCount(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
        enc_flag_wheel1 = 1;
    }
    // left
    if (Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1) == TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG) {
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);
        enc_total_wheel2++;
        enc_counts_wheel2 = enc_counts_track2 + Timer_A_getCaptureCompareCount(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);
        enc_counts_track2 = 0 - Timer_A_getCaptureCompareCount(TIMER_A3_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1);
        enc_flag_wheel2 = 1;
    }
}
