/*	Author: Edward Segura
 *  Partner(s) Name: 
 *	Lab Section: 21
 *	Assignment: Lab 10  Exercise 2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: https://drive.google.com/file/d/1tfgyGXtHoMnftCVeJ_7CZ4P5pFu9mnrU/view?usp=sharing
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

int main(void) {
    /* Insert DDR and PORT initializations */
    //DDRA = 0xFF; PORTA = 0x00;
    DDRB = 0x0F; PORTB = 0xF0;
    DDRC = 0xF0; PORTC = 0x0F;
    static task task1, task2;
    task *tasks[] = { &task1, &task2};
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

    TimerSet(50);
    TimerOn();

    unsigned short i;
    /* Insert your solution below */
    while (1) {
        for(i = 0; i < numTasks; i++){
            if(tasks[i]->elapsedTime == tasks[i]->period){
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += 50;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}
