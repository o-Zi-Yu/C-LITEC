// ENGR-2350 Template Project
#include "engr2350_msp432.h"

void PWMInit();
void ADCInit();
void GPIOInit();
void PWM_ISR();

Timer_A_UpModeConfig TA2cfg;
Timer_A_CompareModeConfig TA2_ccr;

uint8_t timer_flag = 0;
int16_t pwm_max = 2300;  // Maximum limit on PWM output
int16_t pwm_min = 100;   // Minimum limit on PWM output
int16_t pwm_set = 1200;  // Calculated PWM output (control output)

float kp = 0.1;       // proportional control gain
float error_sum = 0;  // Integral control error summation
float ki = 10;        // integral control gain

float target = 1.65;  // Current "setpoint" voltage
float actual = 0;     // Measure voltage from ADC14

int16_t adc_out;  // variable to store the ADC output

// Main Function
int main(void) {
    SysInit();

    GPIOInit();
    PWMInit();
    ADCInit();

    printf("\r\n\nRaw ADC\t\tC Voltage\tSetpoint\tError\tPWM Out\r\n");

    while (1) {
        // If the PWM has cycled, request an ADC sample
        if (timer_flag) {
            // Add ADC conversion code here
            ADC14_toggleConversionTrigger();
            while (ADC14_isBusy()) {
            }
            adc_out = ADC14_getResult(ADC_MEM0);
            actual = adc_out / 16384.0 * 3.3;
            error_sum += target - actual;                                  // perform "integration"
            pwm_set = kp * (pwm_max - pwm_min) / target - ki * error_sum;  // PI control equation
            if (pwm_set > pwm_max) pwm_set = pwm_max;                      // Set limits on the pwm control output
            if (pwm_set < pwm_min) pwm_set = pwm_min;
            Timer_A_setCompareValue(TIMER_A2_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1, pwm_set);                     // enforce pwm control output
            printf("\r%5u\t\t   %1.3f\t  %1.3f  \t%1.3f\t%5u", adc_out, actual, target, target - actual, pwm_set);  // report
            __delay_cycles(240e3);                                                                                  // crude delay to prevent this from running too quickly
            timer_flag = 0;                                                                                         // Mark that we've performed the control loop

            // Update the target setpoint as requested. If using RSLK, change to P4.0 and P4.2
            if (!GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1)) target += 0.01;
            if (!GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN4)) target -= 0.01;
            if (target < 0.1) target = 0.1;
            if (target > 2.9) target = 2.9;
        }
    }
}

void GPIOInit() {
    // Initialize the buttons to configure the setpoint. If using RSLK, change to P4.0 and P4.2
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN1, GPIO_TERTIARY_MODULE_FUNCTION);
}

void PWMInit() {
    // Set up Timer_A2 to run at 10 kHz
    TA2cfg.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    TA2cfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_10;
    TA2cfg.timerPeriod = 2400;
    TA2cfg.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_configureUpMode(TIMER_A2_BASE, &TA2cfg);

    // Configure TA2.CCR1 for PWM generation
    TA2_ccr.compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET;
    TA2_ccr.compareValue = pwm_set;
    TA2_ccr.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    Timer_A_initCompare(TIMER_A2_BASE, &TA2_ccr);

    /* Configuring GPIOs (5.6 TA2.1) */
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P5, GPIO_PIN6,
                                                GPIO_PRIMARY_MODULE_FUNCTION);

    Timer_A_registerInterrupt(TIMER_A2_BASE, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, PWM_ISR);
    Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);
}

void PWM_ISR() {
    Timer_A_clearInterruptFlag(TIMER_A2_BASE);
    timer_flag = 1;
}

void ADCInit() {
    // Activity Stuff...
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_4, ADC_DIVIDER_1, 0);
    ADC14_setResolution(ADC_14BIT);
    ADC14_configureSingleSampleMode(ADC_MEM0, false);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A4, false);
    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);
    ADC14_enableConversion();
}
