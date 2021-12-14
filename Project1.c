/* File: Project1StateMachine
 * Author: Lilah Kelly & Samuel Degemu
 * Date: 1/22/21
 * Purpose: Program Temperature Alarm
 *
 */

/*********** COMPILER DIRECTIVES *********/
#include "pic24_all.h"  //Textbook library header file
#include <stdint.h>
#define B (_RB2) //read pin 6
#define Z  (_RB4) //read pin 11

/*********** GLOBAL VARIABLE AND FUNCTION DEFINITIONS *******/

char arr[4]; //array of length four for xx.x ASCII form
char newarr[4]; //array of length four modified from first array to include temp values  
uint16_t analog; //variable hold analog value to be manually converted to digital
float millivolts; //variable to hold voltage value converted from digital
float celsius_value; //variable to hold celsuis value converted from voltage 
float fahrenheit_value; //variable to hold fahrenheit value converted from celsuis
enum Alarm_States{Alarm_SMStart, Alarm_DISARMED, Alarm_Wait1, Alarm_ARMED, Alarm_Wait2, Alarm_ALARM} Alarm_State; //global state variable

/********** MAIN PROGRAM ********************************/
float mV_to_C(uint16_t mV){     // convert millivolts to Celsius 
    float Cel = (mV - 1047.375)/(-6.125);
    return Cel;
}

float C_to_F(float Cel){        // convert Celsius to Fahrenheit
    float Fer = Cel * 1.8 +32;
    return Fer;
}

char conv_to_Ascii(float Temp){ // convert the temperature to ASCII array
    arr[0]=0x30; //0x30 is value to add decimals 0-9 to get ascii value
    arr[1]=0x30;
    arr[2]=0x2E; //ascii value of decimal . 
    arr[3]=0x30;
    uint16_t Nint = Temp * 10; //multiply parameter Temp by 10 to get rid of decimal
    uint8_t TensPlace = 0; //value of Nint in tens place
    uint8_t OnesPlace = 0; //value of Nint in ones place
    uint8_t TenthPlace = 0; //value of Nint in tenth place
    int i;
    for (i = 0; i < 9; i++){ 
        if (Nint >= 100){
            TensPlace = TensPlace + 1;
            Nint = Nint - 100; //iterates through each 100 in Nint for Temp's tens place
        }
        else if (Nint >= 10){
            OnesPlace = OnesPlace + 1;
            Nint = Nint - 10; //iterates through each 10 in Nint (excluding 100s) for Temp's ones place
        }
        else if (Nint >= 1){
            TenthPlace = TenthPlace + 1;
            Nint = Nint - 1; //iterates through each 1 in Nint (excluding 100s and 10s) for Temp's tenth place
        }
        
    }  
    newarr[0] = arr[0] + TensPlace; 
    newarr[1] = arr[1] + OnesPlace;
    newarr[2] = arr[2];
    newarr[3] = arr[3] + TenthPlace;
    return newarr; //array containing tenth place, ones place, decimal, and tenth place of Temp
}

void TickFct(){ //define state machine
    switch(Alarm_State){ //Switch transitions
        case Alarm_SMStart: //Initial Transition
            Alarm_State = Alarm_DISARMED;
            break;
        
        case Alarm_DISARMED: //pressing button transfers to Wait1 state; otherwise stays on disarmed state
            if (!B){
                Alarm_State = Alarm_Wait1;
            }
            else if (B){
                Alarm_State = Alarm_DISARMED;
            }
            break;
        
        case Alarm_Wait1: //purpose is to wait until button is pressed AND released to progress to armed state
            if (B){
                Alarm_State = Alarm_ARMED;
            }
            else if (!B){
                Alarm_State = Alarm_Wait1;
            }
            break;
            
        case Alarm_ARMED: //if temp reads at 80 degrees F or higher, transfers to alarm state; otherwise stays in armed state
            if (fahrenheit_value >= 80){
                Alarm_State = Alarm_ALARM;
            }
            else{
                Alarm_State = Alarm_ARMED;
            }
            break;
            
        case Alarm_ALARM: //pressing button transfers to Wait2 state; otherwise stays on alarm state
            if (!B){
                Alarm_State = Alarm_Wait2;
            }
            else{
                Alarm_State = Alarm_ALARM;
            }
            break;
        
        case Alarm_Wait2: //purpose is to wait until button is pressed AND released to progress to disarmed state
            if (B){
                Alarm_State = Alarm_DISARMED;
            }
            else if (!B){
                Alarm_State = Alarm_Wait2;
            }
            break;
    }
    switch(Alarm_State){ //State actions
        case Alarm_SMStart:
            break;
        
        case Alarm_DISARMED:
            Z = 0;
            conv_to_Ascii(fahrenheit_value); //converts fahrenheit_value to ascii for monitor to display 
            DELAY_MS(100);   //delay of 100 ms between conversion and print                         
            outString("DISARMED "); 
            DELAY_MS(100);  //delay of 100ms between print statements
            outString(newarr);                               
            outString(" degrees F\n\r");
            break;
            
        case Alarm_Wait1:
            Z = 0;
            break;
        
        case Alarm_ARMED:
            Z = 0;
            conv_to_Ascii(fahrenheit_value);
            DELAY_MS(100);                               
            outString("ARMED ");
            DELAY_MS(100);  
            outString(newarr);
            outString(" degrees F\n\r");
            break;
        
        case Alarm_ALARM:
            Z = 1;
            conv_to_Ascii(fahrenheit_value);
            DELAY_MS(100);                              
            outString("ALARM! Press Button to Stop! ");
            DELAY_MS(100);  
            outString(newarr);
            break;
            
        case Alarm_Wait2:
            Z = 1;
            break;
    }
}
int main(){
    configClock(); ////Sets the clock to 40MHz using FRC and PLL
    configUART1(230400); //parameter is the baudrate for serial communication
    CONFIG_RA1_AS_ANALOG(); //makes pin 3 analog
    configADC1_ManualCH0(RA1_AN, 31, 0); //configures 10 bit ADC for manual conversion using RA1 (pin 3) 
    Alarm_State = Alarm_SMStart; //initializes state machine
    CONFIG_RB2_AS_DIG_INPUT(); //configures pin 6 as digital input
    CONFIG_RB4_AS_DIG_OUTPUT(); //configures pin 11 as digital output
    while(1){
        analog = convertADC1(); //manually converts to digital
        millivolts = analog * 3.1672; //converts analog to millivolts
        celsius_value = mV_to_C(millivolts);
        fahrenheit_value = C_to_F(celsius_value);
        DELAY_MS(300);
        TickFct();
    }
    return 0;
}
