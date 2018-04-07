#include "lcd.h"
#include "math.h"
#include "String.h"

void IR_init(){
    SYSCTL_RCGCGPIO_R |= 2;
    SYSCTL_RCGCADC_R |= 1;
    GPIO_PORTB_AFSEL_R |= 16;
    GPIO_PORTB_DEN_R &= ~16;
    GPIO_PORTB_AMSEL_R |= 16;
    GPIO_PORTB_ADCCTL_R = 0;

    //disable SS0 sample sequencer to configure it
    ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN0;
    //initialize the ADC trigger source as processor (default)
    ADC0_EMUX_R = ADC_EMUX_EM0_PROCESSOR;
    //set 1st sample to use the AIN10 ADC pin
    ADC0_SSMUX0_R |= 0x000A;
    //enable raw interrupt status
    ADC0_SSCTL0_R |= (ADC_SSCTL0_IE0 | ADC_SSCTL0_END0);
    //enable oversampling to average
    ADC0_SAC_R |= ADC_SAC_AVG_64X;
    //re-enable ADC0
    ADC0_ACTSS_R |= ADC_ACTSS_ASEN0;
    //ADC0_PSSI_R = ADC_PSSI_SS1;
}


unsigned IR_read(){
    //initiate SS1 conversion
    ADC0_PSSI_R=ADC_PSSI_SS0;
    //wait for ADC conversion to be complete
    while((ADC0_RIS_R & ADC_RIS_INR0) == 0){
        //wait
        }
    //grab result
    ADC0_ISC_R=ADC_ISC_IN0;
    int value = ADC0_SSFIFO0_R;
    return value;
}

double IR_get_distance() {
    int i;
    double average = 0; // reset average

    for(i = 0; i < 50; i++){ // repeat 50 times to get average over 50ms (average counts as 1 measurement)
        average += (double) IR_read();
        timer_waitMillis(1);
    }

    average = average / 50.0;
    return (180401.0*pow(average, -1.243));
}

//int main(){
//
//    lcd_init();
//    ADC_init();
//
//    int i = 0;
//    int average = 0;
//    int dist;
//
//    while(1) {
//        // dont need software sampling
//        average = 0; // reset average
//
//        for(i = 0; i < 50; i++){ // repeat 50 times to get average over 50ms (average counts as 1 measurement)
//            average += ADC_read();
//            timer_waitMillis(1);
//        }
//        average = average / 50;
//
//        double temp = 180401*pow((double) average, -1.243); // use power function to get distance
//        dist = (int) temp;      //cast temp to int
//        lcd_printf("Raw: %d\ncm: %d", average, dist); // print raw ADC value and distance to lcd
//        timer_waitMillis(450);
//    }
//
//}
