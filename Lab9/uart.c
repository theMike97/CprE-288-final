/**
*
* 	@file  uart.c
*
*
*
*
*
*   @author Andrew T. and Talon S.
*   @date
*/

#include "uart.h"
#include "lcd.h"
#include "wifi.h"
#include "button.h"

/**
 * @brief sets all necessary registers to enable the uart 1 module.
 */
//int main(void){
//    uart_init();
//    lcd_init();
//    WiFi_start("password");
//    index of char
//    int i;
    //char array of 20
//    char data[20];
//    while(1){
        //receive 20 chars before printing it
//        for(i = 0; i < 40; i++){
//             data[i] = uart_receive();
             //if enter is pressed
//             if(data[i] == '\r'){
//                 uart_sendChar('\n \r');
//             }
             //keep sending them chars
//             else{
//                 uart_sendChar(data[i]);
//             }
             //print as chars are entered
//             lcd_printf("char: %c\n index: %d", data[i], i/2+1);
             //break out if enter is pressed
//             if(data[i] == '\r'){
//                 i = 40;
//                 uart_sendChar('\0');
//                 break;
//             }
//         }
             //uart_sendStr(&data);
//             lcd_printf("%s", data);
//    }
//}

void uart_init(void){
    //enable clock to GPIO, R1 = port B
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
    //enable clock to UART1, R1 = UART1
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R1;
    //enable alternate functions on port b pins 0 and 1
    GPIO_PORTB_AFSEL_R |= (BIT0 | BIT1);
    //enable Rx and Txon port B on pins 0 and 1
    GPIO_PORTB_PCTL_R |= 0x00000011;
    //set pin 0 and 1 to digital
    GPIO_PORTB_DEN_R |= (BIT0 | BIT1);
    //set pin 0 to Rx or input
    GPIO_PORTB_DIR_R &= ~BIT0;
    //set pin 1 to Txor output
    GPIO_PORTB_DIR_R |= BIT1;
    //baudrate
    uint16_t iBRD = 8; //16000000 / (16*115200)
    uint16_t fBRD = 44; //(16000000 / (16*115200) - 8) * 64 + 0.5
    UART1_CTL_R &= ~(UART_CTL_UARTEN);
    UART1_IBRD_R = iBRD;
    UART1_FBRD_R = fBRD;

    //set frame, 8 data bits, 1 stop bit, no parity, no FIFO
    UART1_LCRH_R = UART_LCRH_WLEN_8 ;
    //use system clock as source
    UART1_CC_R = UART_CC_CS_SYSCLK;
    //re-enable enable RX, TX, and uart1
    UART1_CTL_R = (UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN);

}

/**
 * @brief Sends a single 8 bit character over the uart 1 module.
 * @param data the data to be sent out over uart 1
 */
void uart_sendChar(char data){
	while(UART1_FR_R & 0x20)
	{
	}

	UART1_DR_R = data;
}

/**
 * @brief polling receive an 8 bit character over uart 1 module.
 * @return the character received or a -1 if error occured
 */
//int uart_receive(void){
//	//uint32_t ret;
//    char data = 0;
//    int button;
//    int last;
//	//wait to receive
//	while(UART1_FR_R & UART_FR_RXFE)
//	{
//	    //while waiting, if buttons are pressed, send preloaded strings to uart/putty and do not repeat
//	    button = button_getButton();
//	    while(button > 0){
//	        if(button == 6 && last != 6){
//	            uart_sendStr("Yes");
//	            last = 6;
//	        }
//	        else if(button == 5 && last != 5){
//	            uart_sendStr("No");
//	            last = 5;
//	        }
//	        else if(button == 4 && last != 4){
//	            uart_sendStr("Blue, no green, Ahhhhh!!!");
//	            last = 4;
//	        }
//	        else{
//	            button = 0;
//	        }
//	    }
//	}
//	//mask the 4 error bits and grab only 8 data bits
//	data = (char)(UART1_DR_R & 0xFF);
//	return data;
//}

char uart_receive(void) {
    char data = 0;

    while (UART1_FR_R & UART_FR_RXFE) {
        if (button_getButton() > 0) {
            return 0;
        }
    }
     data = (char) (UART1_DR_R & 0xff); // mask off error bits
     return data;
}

/**
 * @brief sends an entire string of character over uart 1 module
 * @param data pointer to the first index of the string to be sent
 */
void uart_sendStr(const char *data){
	while(*data != '\0'){
	    uart_sendChar(*data);
	    data++;
	}
}

