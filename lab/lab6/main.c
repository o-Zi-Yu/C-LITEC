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
#include "stdio.h"
#include "stdlib.h"

// Add function prototypes here, as needed.
// void Timer_Init();
void GPIOInit();
void I2CInit();
void Timer_Init();
void Timer_ISR();
void Encoder_ISR();
void PWM_ISR();
void PWM_Calc();
void I2C_writeData(uint32_t moduleInstance, uint8_t PeriphAddress, uint8_t StartReg, uint8_t *data, uint8_t len);
void I2C_readData(uint32_t moduleInstance, uint8_t PeriphAddress, uint8_t StartReg, uint8_t *data, uint8_t len);
uint16_t readRanger();
void DirectionControl();
void EnableControl();
uint16_t readCompass();
// Add global variables here, as needed.
// I2C
eUSCI_I2C_MasterConfig master;
uint16_t ranger;
uint16_t compass;

// Calculation
int16_t desired_heading;
int16_t desired_speed, desired_speed_left, desired_speed_right;
uint16_t measured_heading;
float kp = 0.04;
float ki = 0.003;
int32_t measured_speed1, measured_speed2;
int16_t speed_error1, speed_error2;
int16_t heading_error;
int16_t error_sum1 = 0;
int16_t error_sum2 = 0;
int32_t correct_speed1, correct_speed2;
int32_t correct_speed_compare1, correct_speed_compare2;

// Timer
Timer_A_ContinuousModeConfig A0;
Timer_A_UpModeConfig A1, A6;
Timer_A_CaptureModeConfig A2, A3;
Timer_A_CompareModeConfig A4, A5;

uint32_t enc_total_wheel1, enc_total_wheel2;
int32_t enc_counts_track1, enc_counts_track2;
uint32_t enc_counts_wheel1, enc_counts_wheel2;
uint32_t enc_counts_wheel1_sum, enc_counts_wheel2_sum;
uint16_t enc_flag_wheel1, enc_flag_wheel2;

uint32_t PWM_Calc_avg1, PWM_Calc_avg2;

uint16_t ti;
uint16_t ti2;
uint16_t countdown;
uint16_t wait5s;

uint16_t count1, count2;

uint8_t On, Left;

// Main Function
int main(void) {
    // Add local variables here, as needed.

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();

    // Place initialization code (or run-once) code here
    // Timer_Init();
    GPIOInit();
    Timer_Init();
    I2CInit();
    count1 = 0;
    count2 = 0;
    ti = 0;
    ti2=0;
    desired_speed = 25;

    desired_heading = rand()%3600;
    __delay_cycles(2.4e7);
    printf("start\r\n");
    while (1) {

        if (ti2 == 5){
            ti2 = 0;
            desired_heading = rand()%3600;
            //printf("desired_heading: %d\r\n",desired_heading);
            error_sum1 = 0;
            error_sum2 = 0;
        }

        printf("desired_heading: %d\r\n",desired_heading);
        //ti = 0;
        /*
        desired_heading = rand() % 3600;
        printf("desired_heading: %d\r\n",desired_heading);

        wait5s = 50;
        while (wait5s > 0) {
            // wait 100ms
            //__delay_cycles(2.4e6);

            ti = 0;
            countdown = 1;
            while (countdown > 0) {
                if (ti == 1) {
                    ti = 0;
                    countdown -= 1;
                    wait5s -= 1;
                }
            }
            */

            // printf("100ms passed.\r\n");

            // Distance Control
            measured_heading = readCompass();
            printf("measured_heading: %d\r\n",measured_heading);
            __delay_cycles(2.4e5);  // Wait 1/10 of a second
            heading_error = desired_heading - measured_heading;
            if (heading_error > 1800) {
                heading_error -= 3600;
            }
            if (heading_error < -1800) {
                heading_error += 3600;
            }
            printf("headingerror:%d\r\n",heading_error);
            heading_error = kp * heading_error;
            heading_error = heading_error/10;

            desired_speed_left = desired_speed - heading_error;
            desired_speed_right = desired_speed + heading_error;

            if (abs(desired_speed_left) < 10) {
                desired_speed_left = 0;
            }
            if (abs(desired_speed_left) > 50) {
                if (desired_speed_left > 0) {
                    desired_speed_left = 50;
                } else {
                    desired_speed_left = -50;
                }
            }

            printf("desired_speed_right:%d\r\n",desired_speed_right);
            if (abs(desired_speed_right) < 10) {
                desired_speed_right = 0;
            }
            if (abs(desired_speed_right) > 50) {
                if (desired_speed_right > 0) {
                    desired_speed_right = 50;
                } else {
                    desired_speed_right = -50;
                }
            }

            // Wheel Speed Control
            if (desired_speed_left == 0 && desired_speed_right == 0) {
                On = 0;
                EnableControl();
                // Enforce a maximum encoder output
                enc_counts_wheel1 = 200000;
                enc_counts_wheel2 = 200000;
            } else {
                // set motor direction depending on desired speed
                if (desired_speed_left < 0) {
                    // printf("yes\r\n");
                    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN4);
                } else {
                    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
                }
                if (desired_speed_right < 0) {
                    // printf("yes\r\n");
                    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN5);
                } else {
                    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN5);
                }
                // desired_speed = desired_speed * kp;
                // Left wheel
                //  printf("encoder count=%d\r\n",PWM_Calc_avg1);
                PWM_Calc();
                measured_speed1 = 1500000 / PWM_Calc_avg2;
                desired_speed_left = abs(desired_speed_left);
                //printf("measured_speed_left:%d\r\n", measured_speed1);
                speed_error1 = desired_speed_left - measured_speed1;
                error_sum1 += speed_error1;

                //printf("errorsum=%d\r\n",error_sum1);
                correct_speed1 = desired_speed_left + (ki*error_sum1);
                // correct_speed_compare = correct_speed*24000000/100;
                if (correct_speed1 < 10) {
                    correct_speed1 = 10;
                } else if (correct_speed1 > 90) {
                    correct_speed1 = 90;
                }
                correct_speed_compare1 = correct_speed1 * 960 / 100;
                Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_4, correct_speed_compare1);
                printf("desired_speed: %d\r\n",desired_speed);
                //printf("desired_speed_left: %d\r\n",desired_speed_left);
                //printf("correct_speed_left: %d\r\n", correct_speed1);

                // Right wheel
                measured_speed2 = 1500000 / PWM_Calc_avg1;
                desired_speed_right = abs(desired_speed_right);
                printf("measured_speed_right:%d\r\n", measured_speed2);
                speed_error2 = desired_speed_right - measured_speed2;
                error_sum2 += speed_error2;
                correct_speed2 = desired_speed_right + (ki*error_sum2);
                // correct_speed_compare = correct_speed*24000000/100;
                if (correct_speed2 < 10) {
                    correct_speed2 = 10;
                } else if (correct_speed2 > 90) {
                    correct_speed2 = 90;
                }
                correct_speed_compare2 = correct_speed2 * 960 / 100;
                printf("correct_speed_right: %d\r\n", correct_speed2);
                Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, correct_speed_compare2);
                printf("desired_speed_right: %d\r\n\n",desired_speed_right);

                // turn on motor
                On = 1;
                EnableControl();
            }
        }
    }


// Add function declarations here as needed
void GPIOInit() {
    // I2C
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);

    // motor enable
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN7 | GPIO_PIN6);
    // motor direction
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN4 | GPIO_PIN5);
    // motor speed
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);  // TA0.4 CCR4 of 0
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);  // TA0.3 CCR3 of 0

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);  // right
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);  // left
}

void I2CInit() {
    master.selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK;
    master.i2cClk = 24000000;
    master.dataRate = EUSCI_B_I2C_SET_DATA_RATE_100KBPS;
    master.byteCounterThreshold = 0;
    // master.autoSTOPGeneration =
    I2C_initMaster(EUSCI_B1_BASE, &master);
    I2C_enableModule(EUSCI_B1_BASE);
}

void Timer_Init() {
    // Encoder timer
    A0.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    A0.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    A0.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    A0.timerClear = TIMER_A_DO_CLEAR;
    Timer_A_configureContinuousMode(TIMER_A3_BASE, &A0);
    Timer_A_registerInterrupt(TIMER_A3_BASE, TIMER_A_CCR0_INTERRUPT, Encoder_ISR);
    Timer_A_registerInterrupt(TIMER_A3_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Encoder_ISR);

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

    // PWM timer
    A1.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    A1.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    A1.timerPeriod = 960;
    A1.timerClear = TIMER_A_DO_CLEAR;
    A1.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureUpMode(TIMER_A0_BASE, &A1);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    Timer_A_registerInterrupt(TIMER_A0_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, PWM_ISR);

    A4.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
    A4.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    A4.compareValue = 240;
    A5.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;
    A5.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    A5.compareValue = 240;
    Timer_A_initCompare(TIMER_A0_BASE, &A4);
    Timer_A_initCompare(TIMER_A0_BASE, &A5);

    // Countdown timer
    A6.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    A6.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    A6.timerPeriod = 37500;
    A6.timerClear = TIMER_A_DO_CLEAR;
    A6.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureUpMode(TIMER_A1_BASE, &A6);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    Timer_A_registerInterrupt(TIMER_A1_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Timer_ISR);
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

void PWM_Calc() {
    // printf("PWM_Calc\r\n");
    // printf("enc_flag=%d\r\n",enc_flag_wheel1);
    //  right
    if (enc_flag_wheel1) {    // Check to see if capture occurred
        enc_flag_wheel1 = 0;  // reset capture flag
        count1 += 1;
        enc_counts_wheel1_sum += enc_counts_wheel1;
        if (count1 == 6) {
            // printf("encoder count=%d\r\n",PWM_Calc_avg1);
            PWM_Calc_avg1 = enc_counts_wheel1_sum / 6;
            count1 = 0;
            enc_counts_wheel1_sum = 0;
        }
    }
    // left
    if (enc_flag_wheel2) {    // Check to see if capture occurred
        enc_flag_wheel2 = 0;  // reset capture flag
        count2 += 1;
        enc_counts_wheel2_sum += enc_counts_wheel2;
        if (count2 == 6) {
            PWM_Calc_avg2 = enc_counts_wheel2_sum / 6;
            count2 = 0;
            enc_counts_wheel2_sum = 0;
        }
    }
}

void PWM_ISR() {
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

void Timer_ISR() {
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    ti++;
    if (ti == 10) {
        ti = 0;
        ti2++;
        printf("%d\r\n",ti2);
    }
    /*
    if (ti2 == 5){
        ti2 = 0;
        desired_heading = rand() % 3600;
        printf("desired_heading: %d\r\n",desired_heading);
    }
    */
}

uint16_t readRanger() {
    uint8_t arry2[2];
    I2C_readData(EUSCI_B1_BASE, 0x70, 2, arry2, 2);
    ranger = arry2[0];
    ranger = ranger << 8;
    ranger = ranger | arry2[1];
    arry2[0] = 0x51;
    I2C_writeData(EUSCI_B1_BASE, 0x70, 0, arry2, 1);
    return ranger;
}

uint16_t readCompass() {
    uint8_t arry1[2];
    I2C_readData(EUSCI_B1_BASE, 0x60, 2, arry1, 2);
    compass = arry1[0];
    compass = compass << 8;
    compass = compass | arry1[1];
    return compass;
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

void I2C_writeData(uint32_t moduleInstance, uint8_t PeriphAddress, uint8_t StartReg, uint8_t *data, uint8_t len) {
    I2C_setSlaveAddress(moduleInstance, PeriphAddress);      // Set the peripheral address
    I2C_setMode(moduleInstance, EUSCI_B_I2C_TRANSMIT_MODE);  // Indicate a write operation

    I2C_masterSendMultiByteStart(moduleInstance, StartReg);  // Start the communication.
                                                             // This function does three things. It sends the START signal,
                                                             // sends the address, and then sends the start register.

    // This code loops through all of the bytes to send.
    uint8_t ctr;
    for (ctr = 0; ctr < len; ctr++) {
        I2C_masterSendMultiByteNext(moduleInstance, data[ctr]);
    }
    // Once all bytes are sent, the I2C transaction is stopped by sending the STOP signal
    I2C_masterSendMultiByteStop(moduleInstance);

    __delay_cycles(200);  // A short delay to avoid starting another I2C transaction too quickly
}

void I2C_readData(uint32_t moduleInstance, uint8_t PeriphAddress, uint8_t StartReg, uint8_t *data, uint8_t len) {
    // First write the start register to the peripheral device. This can be
    // done by using the I2C_writeData function with a length of 0.
    I2C_writeData(moduleInstance, PeriphAddress, StartReg, 0, 0);

    Interrupt_disableMaster();  //  Disable all interrupts to prevent timing issues

    // Then do read transaction...
    I2C_setSlaveAddress(moduleInstance, PeriphAddress);     // Set the peripheral address
    I2C_setMode(moduleInstance, EUSCI_B_I2C_RECEIVE_MODE);  // Indicate a read operation
    I2C_masterReceiveStart(moduleInstance);                 // Start the communication. This function
                                                            // doe two things: It first sends the START signal and
                                                            // then sends the peripheral address. Once started, the eUSCI
                                                            // will automatically fetch bytes from the peripheral until
                                                            // a STOP signal is requested to be sent.

    // This code loops through 1 less than all bytes to receive
    uint8_t ctr;
    for (ctr = 0; ctr < (len - 1); ctr++) {
        while (!(UCB1IFG & UCRXIFG0))
            ;                                                        // Wait for a data byte to become available
        data[ctr] = I2C_masterReceiveMultiByteNext(moduleInstance);  // read and store received byte
    }
    // Prior to receiving the final byte, request the STOP signal such that the
    // communication will halt after the byte is received.
    data[ctr] = I2C_masterReceiveMultiByteFinish(moduleInstance);  // send STOP, read and store received byte

    Interrupt_enableMaster();  // Re-enable interrupts

    __delay_cycles(200);  // A short delay to avoid starting another I2C transaction too quickly
}
