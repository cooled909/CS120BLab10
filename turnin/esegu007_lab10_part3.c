/*	Author: Edward Segura
 *  Partner(s) Name: 
 *	Lab Section: 21
 *	Assignment: Lab 10  Exercise 3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: https://drive.google.com/file/d/1gIpkg291wQ6Kgg5wCRUTHnF2FsWc__CM/view?usp=sharing
 */
#include <avr/io.h>
#include "../header/timer.h"
#include "../header/keypad.h"
#include "../header/scheduler.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char input = 0x00;

enum keypadSM {SM_Start, SM_Input, SM_Press, SM_Stop};
char combo[6] = {'#','1', '2', '3', '4', '5'};
unsigned char limit = 0x06;
unsigned char x;
unsigned char currChar = 0x00;
int KeypadTickFct(int state){
    x = GetKeypadKey();
    switch(state){
        case SM_Start:
            state = SM_Input;
        break;
        case SM_Input:
            if(x != '\0'){
                state = SM_Press;
            }
            if(input == 0x01){
                state = SM_Stop;
            }
        break;
        case SM_Press:
            if(x == combo[currChar-1]) { 
                state = SM_Press;
            }
            else{
                state = SM_Input;
            }
            if(input == 0x01){
                state = SM_Stop;
            }
        break;
        case SM_Stop:
        if(input == 0x00){
            state = SM_Input;
        }
        break;
        default:
            state = SM_Start;
        break;
    }
    switch(state){
        case SM_Start:
        break;
        case SM_Input:
            if(x == combo[currChar]){
                currChar++;
            }
            else if(x != combo[currChar] && x != '\0'){
                currChar = 0x00;
            }
            if(currChar == limit){
                input = 0x01;
                currChar = 0x00;
            }
            //PORTA = currChar; testing
        break;
        default:
        break;
    }
    return state;
}

enum LockSM {SM_Start1, SM_Lock, SM_Unlock};
unsigned char button;
int LockTick(int state){
    button = ~PINB & 0x80;
    switch(state){
        case SM_Start1:
            state = SM_Lock;
        break;
        case SM_Lock:
            if(input == 0x01){
                state = SM_Unlock;
            }
        break;
        case SM_Unlock:
            if(button){
                state = SM_Lock;
            }
        break;
        default:
        state = SM_Start1;
        break;
    }
    switch(state){
        case SM_Start1:
        break;
        case SM_Lock:
            input = 0x00;
            PORTB = input;
        break;
        case SM_Unlock:
            PORTB = 0x01;
        break;
    }
    return state;
}

void set_PWM(double frequency) {
    static double current_frequency;

    if (frequency != current_frequency) {
        if(!frequency)
            TCCR3B &= 0x08;
        else
            TCCR3B |= 0x03;

        if(frequency < 0.954)
            OCR3A = 0xFFFF;
        else if (frequency > 31250)
            OCR3A = 0x0000;
        else
            OCR3A = (short) (8000000 / (128 * frequency)) - 1;
        
        TCNT3 = 0;
        current_frequency = frequency;
    }
}

void PWM_on() {
    TCCR3A = (1 << COM3A0);
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
    set_PWM(0);
}

void PWM_off() {
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}

enum BellSM {SM_Start2, SM_Wait, SM_Play, SM_Press1};
unsigned char button0;
unsigned char currnote = 0x01;
unsigned char i = 0x00;
double notes[8] = {523.25, 659.25, 587.33, 392.00, 523.25, 587.33, 659.25, 523.25};
int length[8] = {1,1,1,3,1,1,1,3};
    int BellTickFct(int state){
        button0 = ~PINA & 0x80;
        switch(state){
            case SM_Start2:
            state = SM_Wait;
            break;
            case SM_Wait:
            if(button0){
                state = SM_Play;
            }
            break;
            case SM_Play:
            if(i > 0x07){
                i = 0x00;
                if(button0){
                    state = SM_Press1;
                }
                else{
                    state = SM_Wait;
                }           
            }
            break;
            case SM_Press1:
            if(!button0){
                state = SM_Wait;
            }
            default:
            state = SM_Start2;
            break;
        }
        switch(state){
        case SM_Start:
        break;
        case SM_Press1:
        set_PWM(0);
        break;
        case SM_Wait:
        set_PWM(0);
        break;
        case SM_Play:
        if(currnote  == 1){ 
            set_PWM(notes[i]);
        }
        if(currnote > length[i]){
            i++;
            currnote = 0x01;
        }
        else{
            currnote++;
        }
        break;
    }
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0x7F; PORTB = 0x80;
    DDRC = 0xF0; PORTC = 0x0F;
    static task task1, task2, task3;
    task *tasks[] = { &task1, &task2, &task3};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    task1.state = start;
    task1.period = 50;
    task1.elapsedTime = task1.period; 
    task1.TickFct = &KeypadTickFct;

    task2.state = start;
    task2.period = 50;
    task2.elapsedTime = task2.period;
    task2.TickFct = &LockTick;

    task3.state = start;
    task3.period = 200;
    task3.elapsedTime = task3.period;
    task3.TickFct = &BellTickFct;



    TimerSet(50);
    TimerOn();
    PWM_on();
    unsigned short j;
    /* Insert your solution below */
    while (1) {
        for(j = 0; j < numTasks; j++){
            if(tasks[j]->elapsedTime == tasks[j]->period){
                tasks[j]->state = tasks[j]->TickFct(tasks[j]->state);
                tasks[j]->elapsedTime = 0;
            }
            tasks[j]->elapsedTime += 50;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}
