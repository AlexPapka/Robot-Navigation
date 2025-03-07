// Required Header
/* EEL-4742L Lab 10
 * Fall 22
 * Thursday
 * Zachary Ian Fogleman
 * Alexander James Papka
 */

// Include Files
//#include "ti/devices/msp432p4xx/inc/msp.h"
#include "ti/devices/msp432p4xx/driverlib/driverlib.h"
#include <stdint.h>

// Wheel Defines
#define PERIOD 1500
#define DUTY 25
#define CLOCKDIVIDER TIMER_A_CLOCKSOURCE_DIVIDER_48
#define LEFTCHANNEL TIMER_A_CAPTURECOMPARE_REGISTER_4
#define RIGHTCHANNEL TIMER_A_CAPTURECOMPARE_REGISTER_3
// Time Defines
#define CCPS 3000000 // clock cycles per second
#define DELAY CCPS/2 // half a second
#define TENMILLI CCPS/100 // 10ms
#define CHECKTIME 1.5*CCPS/1000 //
#define MICRO CCPS/100000 // 10us

// Global Variables
typedef enum {OFF, ON} TOGGLE;
typedef enum {NONE, FRONT, REAR, BOTH} LEDrobot;
typedef enum {POWERUP, STANDBY, TRACKING, LOST} MODE;
typedef enum {DRK, RED, GRN, BLU, YLW, CYN, MGT, WHT} COLOR;
typedef enum {LFOR, LREV, LBRK, LCST, RFOR, RREV, RBRK, RCST} WHEELS;
MODE mainState = POWERUP;
COLOR boardLED = DRK;
TOGGLE flashState = OFF;
Timer_A_PWMConfig timerPWMConfig;


// Function Headers
void setup(void);
void config432IO(void);
void configRobotIO(void);
void setLEDs(TOGGLE newState, COLOR newColor);
void toggleRobotLED(LEDrobot ToggleMe);
void bootFlash(void);
void robotControl(MODE instruction);
void bumperSwitchesHandler(void);
void sensorRead(void);
void sensorPrep1(void);
void sensorPrep2(void);
void sensorReset(void);
void sensorResolve(uint8_t left, uint8_t right);
void setWheels(WHEELS instruction);
void configPWMTimer(uint16_t clockPeriod, uint16_t clockDivider, uint16_t duty, uint16_t channel);


int main(void) {

    setup(); // run setup
    uint16_t counter = 0;

    while(1) {

        sensorRead();

        if(mainState!=LOST){counter = 0;}else{counter++;}
        if(counter >= 750){mainState = STANDBY;}

        /*LEDCONTROLS*/
        switch(mainState){
            case STANDBY:
//                if(boardLED != WHT){
                    if(flashState==ON)
                    {
                        setLEDs(ON,boardLED);
                        flashState=OFF;
                    }
                    else
                    {
                        setLEDs(OFF,boardLED);
                        flashState=ON;
                    }
//                }else{setLEDs(OFF,WHT);}
                __delay_cycles(DELAY);
                break;
            case TRACKING:
                setLEDs(ON,boardLED);
                break;
            case LOST:
                setLEDs(OFF,WHT);
                break;
        }
        /*LEDCONTROLSEND*/

        robotControl(mainState);

        __delay_cycles(TENMILLI);

    }
}

void setup(void)
{
    // Stop WDT
    MAP_WDT_A_holdTimer();

    // 432 IO init
    config432IO();

    // Robot IO init
    configRobotIO();

    // Config PWM
    configPWMTimer(PERIOD, CLOCKDIVIDER, DUTY, LEFTCHANNEL);
    configPWMTimer(PERIOD, CLOCKDIVIDER, DUTY, RIGHTCHANNEL);

    //bootflash
    bootFlash();

    // Start Timer A UP MODE
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    // Enable interrupts
    __enable_interrupts();

    // Enter Standby
    mainState = STANDBY;
}

void config432IO(void)
{

    // set pins as output
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN0); // red LED1
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN0); // red LED2
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN1); // grn LED2
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2,GPIO_PIN2); // blu LED2

    // toggle output low
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1,GPIO_PIN0); // ||
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN0); // ||
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN1); // ||
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN2); // ||

}

void configRobotIO(void)
{

    /* INPUTS */

    // Bumpers
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0);
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN2);
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN3);
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN5);
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN6);
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN7);

    // BumperIO interrupt enable
    MAP_GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN0);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN2);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN3);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN5);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN6);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN7);

    // Bumper edge
    MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P4, GPIO_PIN0,GPIO_HIGH_TO_LOW_TRANSITION);
    MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P4, GPIO_PIN2,GPIO_HIGH_TO_LOW_TRANSITION);
    MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P4, GPIO_PIN3,GPIO_HIGH_TO_LOW_TRANSITION);
    MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P4, GPIO_PIN5,GPIO_HIGH_TO_LOW_TRANSITION);
    MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P4, GPIO_PIN6,GPIO_HIGH_TO_LOW_TRANSITION);
    MAP_GPIO_interruptEdgeSelect(GPIO_PORT_P4, GPIO_PIN7,GPIO_HIGH_TO_LOW_TRANSITION);

    // Bumper flag clear
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN0);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN2);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN3);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN5);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN6);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN7);

    // Bumper interrupt function select
    MAP_GPIO_registerInterrupt(GPIO_PORT_P4, bumperSwitchesHandler);

    /* OUTPUTS */

    // Other
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P5,GPIO_PIN3); // CNTL Even
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN3); // ||
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P9,GPIO_PIN2); // CNTL Odd
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P9,GPIO_PIN2); // ||

    // LEDs
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P8,GPIO_PIN0); // FL LED
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P8,GPIO_PIN5); // FR LED
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P8,GPIO_PIN6); // RL LED
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P8,GPIO_PIN7); // RR LED

    // Left Motor
//    MAP_GPIO_setAsOutputPin(GPIO_PORT_P5,GPIO_PIN2); // Enable
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P5,GPIO_PIN4); // Direction
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P3,GPIO_PIN7); // Sleep
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,GPIO_PIN7,GPIO_PRIMARY_MODULE_FUNCTION); // PWM

    // Right Motor
//    MAP_GPIO_setAsOutputPin(GPIO_PORT_P5,GPIO_PIN0); // ||
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P5,GPIO_PIN5); // ||
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P3,GPIO_PIN6); // ||
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,GPIO_PIN6,GPIO_PRIMARY_MODULE_FUNCTION); // ||

    /* INITIALIZE */

    // LEDs
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN5);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN6);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN7);

    // Left Motor
//    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN2); // Enable
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN4); // Direction
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN7); // Sleep

    // Right Motor
//    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN0); // ||
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN5); // ||
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN6); // ||


}

void setLEDs(TOGGLE newState, COLOR newColor)
{

    if(newState == ON)
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN0); // r1 on
    else
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1,GPIO_PIN0);  // r1 off

    switch(newColor)
    {
        case DRK:
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN0);  // r2 off
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN1);  // g2 off
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN2);  // b2 off
            break;
        case RED:
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0); // r2 on
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN1);  // g2 off
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN2);  // b2 off
            break;
        case GRN:
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN0);  // r2 off
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1); // g2 on
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN2);  // b2 off
            break;
        case BLU:
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN0);  // r2 off
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN1);  // g2 off
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2); // b2 on
            break;
        case YLW:
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0); // r2 on
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1); // g2 on
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN2);  // b2 off
            break;
        case CYN:
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN0);  // r2 off
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1); // g2 on
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2); // b2 on
            break;
        case MGT:
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0); // r2 on
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN1);  // g2 off
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2); // b2 on
            break;
        case WHT:
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0); // r2 on
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1); // g2 on
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2); // b2 on
            break;
        default:
            break;
    }

}

void toggleRobotLED(LEDrobot ToggleMe)
{
    switch(ToggleMe)
    {
        case NONE:
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN0);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN5);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN6);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN7);
            break;
        case FRONT:
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P8,GPIO_PIN0);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P8,GPIO_PIN5);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN6);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN7);
            break;
        case REAR:
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN0);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P8,GPIO_PIN5);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P8,GPIO_PIN6);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P8,GPIO_PIN7);
            break;
        case BOTH:
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P8,GPIO_PIN0);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P8,GPIO_PIN5);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P8,GPIO_PIN6);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P8,GPIO_PIN7);
            break;
        default:
            break;
    }
}

void bootFlash(void)
{
    setLEDs(ON, WHT);
    toggleRobotLED(BOTH);
    __delay_cycles(DELAY);
    setLEDs(OFF, DRK);
    toggleRobotLED(NONE);
    __delay_cycles(DELAY);
}

void setWheels(WHEELS instruction)
{
    switch(instruction)
    {
    case LFOR:
//        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN2); // Enable
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN4); // Direction
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN7); // Sleep
        break;
    case LREV:
//        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN2);
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN4);
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN7);
        break;
    case LBRK:
//        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN2);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN4);
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN7);
        break;
    case LCST:
//        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN2);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN4);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN7);
        break;
    case RFOR:
//        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN0); // Enable
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN5); // Direction
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN6); // Sleep
        break;
    case RREV:
//        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN0);
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN5);
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN6);
        break;
    case RBRK:
//        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN0);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN5);
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN6);
        break;
    case RCST:
//        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN0);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN5);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN6);
        break;
    default:
        break;
    }
}

void configPWMTimer(uint16_t clockPeriod, uint16_t clockDivider, uint16_t duty, uint16_t channel)
{
    const uint32_t TIMER=TIMER_A0_BASE;
    uint16_t dutyCycle = duty*clockPeriod/100;
    timerPWMConfig.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timerPWMConfig.clockSourceDivider = clockDivider;
    timerPWMConfig.timerPeriod = clockPeriod;
    timerPWMConfig.compareOutputMode = TIMER_A_OUTPUTMODE_TOGGLE_SET;
    timerPWMConfig.compareRegister = channel;
    timerPWMConfig.dutyCycle = dutyCycle;
    MAP_Timer_A_generatePWM(TIMER, &timerPWMConfig);
    MAP_Timer_A_stopTimer(TIMER);
}

void robotControl(MODE instruction)
{
    switch(instruction){
        case POWERUP:
            toggleRobotLED(BOTH);
            break;
        case STANDBY:
            setWheels(LCST);
            setWheels(RCST);
            toggleRobotLED(NONE);
            break;
        case TRACKING:
            switch(boardLED){
                case RED:
                    setWheels(LREV);
                    setWheels(RFOR);
                    break;
                case YLW:
                    setWheels(RFOR);
                    setWheels(LCST);
                    break;
                case GRN:
                    setWheels(LFOR);
                    setWheels(RFOR);
                    break;
                case CYN:
                    setWheels(RCST);
                    setWheels(LFOR);
                    break;
                case BLU:
                    setWheels(RREV);
                    setWheels(LFOR);
                    break;
            }
            toggleRobotLED(FRONT);
            break;
        case LOST:
            setWheels(RREV);
            setWheels(LREV);
            toggleRobotLED(REAR);
            __delay_cycles(TENMILLI*2);
            break;
        default:
            break;
    }
}

void bumperSwitchesHandler(void)
{
    uint16_t status;
    _delay_cycles(TENMILLI); // debounce switch
    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P4);
    switch(status)
    {
        case GPIO_PIN0: // BS0
            if(mainState==STANDBY)
            {
                mainState = TRACKING;
                break;
            }
        case GPIO_PIN2: // BS1
        case GPIO_PIN3: // BS2
        case GPIO_PIN5: // BS3
        case GPIO_PIN6: // BS4
        case GPIO_PIN7: // BS5
        default:
            if(mainState == TRACKING || mainState == LOST)
                mainState = STANDBY;
            break;
    }
}

void sensorRead(void)
{
    volatile uint8_t S[8] = {0};
    volatile uint8_t left = 0, right = 0, i = 0;
    sensorPrep1();
    __delay_cycles(MICRO);
    sensorPrep2();
    __delay_cycles(CHECKTIME);

    S[0] = MAP_GPIO_getInputPinValue(GPIO_PORT_P7, GPIO_PIN0);
    S[1] = MAP_GPIO_getInputPinValue(GPIO_PORT_P7, GPIO_PIN1);
    S[2] = MAP_GPIO_getInputPinValue(GPIO_PORT_P7, GPIO_PIN2);
    S[3] = MAP_GPIO_getInputPinValue(GPIO_PORT_P7, GPIO_PIN3);
    S[4] = MAP_GPIO_getInputPinValue(GPIO_PORT_P7, GPIO_PIN4);
    S[5] = MAP_GPIO_getInputPinValue(GPIO_PORT_P7, GPIO_PIN5);
    S[6] = MAP_GPIO_getInputPinValue(GPIO_PORT_P7, GPIO_PIN6);
    S[7] = MAP_GPIO_getInputPinValue(GPIO_PORT_P7, GPIO_PIN7);

    for(i=0;i<4;i++)
        right += S[i];
    for(;i<8;i++)
        left += S[i];

    sensorResolve(left, right);

    sensorReset();
}

void sensorPrep1(void)
{
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P5,GPIO_PIN3);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P9,GPIO_PIN2);

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P7,GPIO_PIN0);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P7,GPIO_PIN1);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P7,GPIO_PIN2);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P7,GPIO_PIN3);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P7,GPIO_PIN4);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P7,GPIO_PIN5);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P7,GPIO_PIN6);
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P7,GPIO_PIN7);

    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P7,GPIO_PIN0);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P7,GPIO_PIN1);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P7,GPIO_PIN2);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P7,GPIO_PIN3);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P7,GPIO_PIN4);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P7,GPIO_PIN5);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P7,GPIO_PIN6);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P7,GPIO_PIN7);

}

void sensorPrep2(void)
{
    MAP_GPIO_setAsInputPin(GPIO_PORT_P7,GPIO_PIN0);
    MAP_GPIO_setAsInputPin(GPIO_PORT_P7,GPIO_PIN1);
    MAP_GPIO_setAsInputPin(GPIO_PORT_P7,GPIO_PIN2);
    MAP_GPIO_setAsInputPin(GPIO_PORT_P7,GPIO_PIN3);
    MAP_GPIO_setAsInputPin(GPIO_PORT_P7,GPIO_PIN4);
    MAP_GPIO_setAsInputPin(GPIO_PORT_P7,GPIO_PIN5);
    MAP_GPIO_setAsInputPin(GPIO_PORT_P7,GPIO_PIN6);
    MAP_GPIO_setAsInputPin(GPIO_PORT_P7,GPIO_PIN7);
}

void sensorReset(void)
{
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN3);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P9,GPIO_PIN2);
}

void sensorResolve(uint8_t left, uint8_t right)
{

    if(mainState == LOST) mainState = TRACKING;

    if(left == right && left == 0)
    {
        boardLED = WHT;
        if(mainState == TRACKING)
        {
            mainState = LOST;
        }
    }
    else if ((left>0&&right==0)||left > (right+1))
    {
        if(right==0)boardLED = RED;
        else boardLED = YLW;
    }
    else if ((right>0&&left==0)||right > (left+1))
    {
        if(left==0) boardLED = BLU;
        else boardLED = CYN;
    }
    else boardLED = GRN;
}
