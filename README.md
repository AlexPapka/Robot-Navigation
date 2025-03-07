# Robot-Navigation

EEL-4742L Advanced Microprocessor Based System Design Lab Report





Section No:
0001


Lab Instructor:
Mr. Ayodeji Ogundana


Lab No:
10


Lab Title:
Robot Navigation


Name:
Alex Papka


Partner’s Name:
Zachary Fogleman


Date Performed:
11/17/22


Date Delivered:
12/1/2022





Contents
EEL-4742LAdvanced Microcontroller Based Systems Design Lab Report	1
1.	Introduction	3
2.	Design Requirements	3
3.	Theoretical Design	3
4.	Synthesized Design	4
5.	Experimental Results	4
6.	Summary	4
7.	Lessons Learned	5


Introduction
In this lab, students were tasked with using what the learned in the previous labs, to navigate their Roomba through a figure 8 track, within the time limit of 70 seconds.  

Design Requirements
Step 1, turn on design
All LED toggle once
Device enters standby mode.
In standby mode, the LED1 flashes, and the LED2 display’s color based on the number of sensors active. The table below shows LED colors based on the number of sensors active.
If BMP0 is pressed, the device goes to tracking mode.
In tracking mode, LED1 is solid, and LED2 display’s color based on the number of sensors active. The table below shows LED colors based on the number of sensors active.
The wheels move based on the number of sensors active, in the same pattern as the LED2.
If in Tracking mode, the device loses the Line, the design will go to Lost mode, and LED1 will turn off, and LED2 will turn white. If the line is re-found, the device will return to Tracking mode
If any BMP is pressed while in tracking mode or lost mode, the device will return to  standby mode.
The wheels move in reverse in Tracking mode, as to attempt to find the track again, after 10 seconds the design retuurns to tracking mode.
   
   
Theoretical Design
This section should summarize the theory of operation behind the design and present the design itself.  This section should include the following parts:
Top-level design: 
 Read sensor function
Switch state
If standby{
If LED not white{
set LED2
}
If LED white{
LED1 off, Set LED2 to white
}
}
If tracking{
Set LED1 on, set LED2
}
If lost{
Set LED1 off, set LED2 white
}
Robot control function()
delay


Functional description of modules: 
 Read sensor function{
Read in sensor values for P7.0-P7.7
Count how many left sensors are 1
Count how many right sensors are 1
Run sensor resolve function(Left val, right val)
}
Sensor resolve{
If state is lost, it is now tracking
If Left == right == 0
Color is white
If Left == right != 0
Color is green
If Left > Right+1 {
If Right == 0 color is red
Else color is yellow
}
If Right > Left+1{
If Left == 0 color is Blue
Else color is Cyan
}
}
Robot control function{
Switch (mode)
	Case power up,  toggle robit LEDS
	Case stand by, set wheels to coast, turn off robot LEDS
	Case tracking, check LED2 enum
	If red, Lreverse, Rforward
	If yellow, Lcoast, Rforward
	If green, Lforward, Rforward
	If cyan, Lforward, Rcoast
	If blue, Lforward, Rreverse
	For all colors, toggle forward robot LEDS
	Case Lost, reverse both wheels, turn on back robot LEDs
}
 


Lab report questions.  
Did we change our design from week 8 to week 10?
We did change our design slightly, we needed the design to be slightly dumber, so we made it so that left and right turns, to slightly turn, needed 2 more sensors on a side, rather than just 1. This made the design less sensitive to the crossing in the middle of the figure 8. 

Synthesized Design

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

Experimental Results
Certification sheet is attached to this file on canvas.  
 

Summary
7 passed
0 failed
100% passed
42.99s on track   
Lessons Learned

We learned sometimes, a complecated solution isn’t always the best
The sensors on the Roomba, can be too sensitive if you have a significant number of them
Sometimes, failure can be a better teacher than success.
This lab went very well, I waited and watched what the flaws in our lab 8 design were for the track, then upon implementing them, we had a minor typo, that when resolved, had our Roomba successfully navigate the track with an ~100% success rate.



	
