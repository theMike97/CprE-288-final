/*
 * main.c
 */
#include "lcd.h"
#include "String.h"
#include "Timer.h"
#include "stdlib.h"
#include <stdbool.h>
#include "driverlib/interrupt.h"
#include <inc/tm4c123gh6pm.h>



#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

// vars could change at any time - "prevent" optimization
volatile unsigned sonar_last_time = 0;
volatile unsigned sonar_curr_time = 0;
volatile int update_flag = 0;

void TIMER3B_Handler(void){
    switch(update_flag) {
    case 0:
        sonar_last_time = TIMER3_TBR_R;
        break;
    case 1:
        sonar_curr_time = TIMER3_TBR_R;
        break;
    default:
        break;
    }
    update_flag = ++update_flag % 3;
    TIMER3_ICR_R |= 0x400; // clear interrupt
}

void sonar_init(void){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1; // send clock to GPIO
    GPIO_PORTB_AFSEL_R |= BIT3; // select alternate bit3 func for ping sensor
    GPIO_PORTB_PCTL_R |= 0x7000;
    GPIO_PORTB_DEN_R |= BIT3; // digital enable bit3
    GPIO_PORTB_DIR_R &= ~BIT3; // make bit3 input

    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R3; // send clock to TIMER3
    TIMER3_CTL_R &= ~0x100; // disable timer3b
    TIMER3_CFG_R |= 0x00000004; // set 16 bit timer config
    TIMER3_TBMR_R |= 0x17; // input capture mode, edge time count up (0b0111)
    TIMER3_CTL_R |= 0x0c00; // both edges trigger interrupt
    TIMER3_TBPR_R = 0xff; // max out 8 bits (24 bit timer now)
    TIMER3_TBILR_R = 0xffff; // max out load
    TIMER3_ICR_R |= 0x400; // clear timerb interrupt capture
    TIMER3_IMR_R |= 0x400; // enable timer3 interrupt
    TIMER3_CTL_R |= 0x100; // re-enable timer3b

    NVIC_PRI9_R |= 0x20; // set interrupt priority
    NVIC_EN1_R |= 0x10; // enable interrupt for timer3b

    IntRegister(INT_TIMER3B, TIMER3B_Handler);
}


void sonar_send_pulse(void){
//    TIMER3_TBR_R &= 0;

    TIMER3_CTL_R &= ~0x100;
//    GPIO_PORTB_DEN_R &= ~BIT3;
    GPIO_PORTB_AFSEL_R &= ~BIT3; // turn alternate function off
    GPIO_PORTB_DIR_R |= BIT3; // set PB3 as output
    GPIO_PORTB_DATA_R |= BIT3; // set PB3 to high
    timer_waitMicros(5);
    // wait at least 5 microseconds based on data sheet (10 to be safe)
    GPIO_PORTB_DATA_R &= ~BIT3; // set PB3 to low
    GPIO_PORTB_DIR_R &= ~BIT3; // set PB3 as input
    GPIO_PORTB_AFSEL_R |= BIT3; // turn alternate function back on
//    GPIO_PORTB_DEN_R |= BIT3;
    TIMER3_CTL_R |= 0x100;
}

double sonar_get_distance(void) {
    double diff = 0;
    int overflow = 0;

    sonar_send_pulse();
    if (update_flag == 2) {
        diff = (double) (sonar_curr_time - sonar_last_time);
        if (sonar_curr_time < sonar_last_time) {
            overflow += (sonar_curr_time < sonar_last_time);
            diff = (double)(overflow << 24) + diff;
        }
        update_flag = 0;
    }
    diff = 0.0011 * diff - 0.6429;
    return diff;
}

//int main(void){
//
//    lcd_init();
//    pulse_init();
//
//    unsigned long diff = 0;
//    int overflow = 0;
//
//    while(1) {
//        timer_waitMillis(100);
//        send_pulse();
//        if (update_flag == 2) {
//            diff = sonar_curr_time - sonar_last_time; // get basic time differential
//            if (sonar_curr_time < sonar_last_time) { // adjust for overflow
//                overflow += (sonar_curr_time < sonar_last_time);
//                diff = ((unsigned long) overflow<<24) + diff;
//            }
//            lcd_printf("Cycles: %d\nDistance: %d", diff, get_distance(diff));
//            timer_waitMillis(500);
//            update_flag = 0;
//        }
//    }
//}
