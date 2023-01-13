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
//void Timer_Init();
void GPIOInit();
void I2CInit();
void I2C_writeData(uint32_t moduleInstance
                  ,uint8_t PeriphAddress
                  ,uint8_t StartReg
                  ,uint8_t *data
                  ,uint8_t len);
void I2C_readData(uint32_t moduleInstance
                 ,uint8_t PeriphAddress
                 ,uint8_t StartReg
                 ,uint8_t *data
                 ,uint8_t len);
uint16_t readCompass();
uint16_t readRanger();
// Add global variables here, as needed.
eUSCI_I2C_MasterConfig master;
uint16_t compass;
uint16_t ranger;

// Main Function
int main(void) {
    // Add local variables here, as needed.

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();

    // Place initialization code (or run-once) code here
    //Timer_Init();
    GPIOInit();
    I2CInit();
    while (1) {
        //readCompass();
        //printf("Compass: %4u\r\n",compass);
        //__delay_cycles(2.4e6); // Wait 1/10 of a second
        //readRanger();
        printf("Compass: %4u\tRanger: %4u\r\n",readCompass(),readRanger());
        __delay_cycles(2.4e6); // Wait 1/10 of a second
    }
}

// Add function declarations here as needed
void GPIOInit() {
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);
}

void I2CInit(){
    master.selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK;
    master.i2cClk = 24000000;
    master.dataRate = EUSCI_B_I2C_SET_DATA_RATE_100KBPS;
    master.byteCounterThreshold = 0;
    //master.autoSTOPGeneration =
    I2C_initMaster(EUSCI_B1_BASE, &master);
    I2C_enableModule(EUSCI_B1_BASE);
}

uint16_t readCompass(){
    uint8_t arry1[2];
    I2C_readData(EUSCI_B1_BASE,0x60,2,arry1,2);
    compass = arry1[0];
    compass = compass<<8;
    compass = compass | arry1[1];
    return compass;
}

uint16_t readRanger(){
    uint8_t arry2[2];
    I2C_readData(EUSCI_B1_BASE,0x70,2,arry2,2);
    ranger = arry2[0];
    ranger = ranger<<8;
    ranger = ranger | arry2[1];
    arry2[0] = 0x51;
    I2C_writeData(EUSCI_B1_BASE,0x70,0,arry2,1);
    return ranger;
}


void I2C_writeData(uint32_t moduleInstance
                  ,uint8_t PeriphAddress
                  ,uint8_t StartReg
                  ,uint8_t *data
                  ,uint8_t len)
{
    I2C_setSlaveAddress(moduleInstance,PeriphAddress); // Set the peripheral address
    I2C_setMode(moduleInstance,EUSCI_B_I2C_TRANSMIT_MODE); // Indicate a write operation

    I2C_masterSendMultiByteStart(moduleInstance,StartReg); // Start the communication.
                // This function does three things. It sends the START signal,
                // sends the address, and then sends the start register.

    // This code loops through all of the bytes to send.
    uint8_t ctr;
    for(ctr = 0;ctr<len;ctr++){
        I2C_masterSendMultiByteNext(moduleInstance,data[ctr]);
    }
    // Once all bytes are sent, the I2C transaction is stopped by sending the STOP signal
    I2C_masterSendMultiByteStop(moduleInstance);

    __delay_cycles(200); // A short delay to avoid starting another I2C transaction too quickly
}

void I2C_readData(uint32_t moduleInstance
                 ,uint8_t PeriphAddress
                 ,uint8_t StartReg
                 ,uint8_t *data
                 ,uint8_t len)
{
    // First write the start register to the peripheral device. This can be
    // done by using the I2C_writeData function with a length of 0.
    I2C_writeData(moduleInstance,PeriphAddress,StartReg,0,0);

    Interrupt_disableMaster(); //  Disable all interrupts to prevent timing issues

    // Then do read transaction...
    I2C_setSlaveAddress(moduleInstance,PeriphAddress); // Set the peripheral address
    I2C_setMode(moduleInstance,EUSCI_B_I2C_RECEIVE_MODE); // Indicate a read operation
    I2C_masterReceiveStart(moduleInstance); // Start the communication. This function
                // doe two things: It first sends the START signal and
                // then sends the peripheral address. Once started, the eUSCI
                // will automatically fetch bytes from the peripheral until
                // a STOP signal is requested to be sent.

    // This code loops through 1 less than all bytes to receive
    uint8_t ctr;
    for(ctr = 0;ctr<(len-1);ctr++){
        while(!(UCB1IFG&UCRXIFG0)); // Wait for a data byte to become available
        data[ctr] = I2C_masterReceiveMultiByteNext(moduleInstance); // read and store received byte
    }
    // Prior to receiving the final byte, request the STOP signal such that the
    // communication will halt after the byte is received.
    data[ctr] = I2C_masterReceiveMultiByteFinish(moduleInstance); // send STOP, read and store received byte

    Interrupt_enableMaster(); // Re-enable interrupts

    __delay_cycles(200); // A short delay to avoid starting another I2C transaction too quickly
}
