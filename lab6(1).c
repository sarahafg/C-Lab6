#include "lcd.h"
#include <msp430.h>

// LCD memory map for numeric digits
const char digit[10][2] =
{
    {0xFC, 0x28},  /* "0" LCD segments a+b+c+d+e+f+k+q */
    {0x60, 0x20},  /* "1" */
    {0xDB, 0x00},  /* "2" */
    {0xF3, 0x00},  /* "3" */
    {0x67, 0x00},  /* "4" */
    {0xB7, 0x00},  /* "5" */
    {0xBF, 0x00},  /* "6" */
    {0xE4, 0x00},  /* "7" */
    {0xFF, 0x00},  /* "8" */
    {0xF7, 0x00}   /* "9" */
};

const char digitpos[6] = {pos6, pos5, pos4, pos3, pos2, pos1};
unsigned int count = 0;
unsigned char dis_matrix[3] = {0x01, 0x03, 0x07};
volatile unsigned int n;
unsigned int lcd_val;
unsigned int col = 0;

void displayNum(unsigned int num, unsigned int num1, unsigned int num2, unsigned int num3, unsigned int digits)
{
    short i;
    if(count == 0){
        for(i = digits; i != 0; i--){
            showDigit((num % 10), (digits - i));
            num = num / 10;
        }
    }
    if(count == 3){
        for(i = num3; i != 0; i--){
            showDigit((num % 10), (num3 - i));
            num = num / 10;
        }
    }
    if(count == 1){
        for(i = num1; i != 0; i--){
            showDigit((num % 10), (num1 - i));
            num = num / 10;
        }
        }
    if(count == 2){
        for(i = num2; i != 0; i--){
            showDigit((num % 10), (num2 - i));
            num = num / 10;
        }
    }
}

void showDigit(char c, unsigned int position)
{
    unsigned int mydigit;

    LCDMEM[digitpos[position]] = digit[c][0];
    mydigit = digit[c][1];
    LCDMEM[digitpos[position]+1] = mydigit;
}

int lcd_init()
{
    PJSEL0 = BIT4 | BIT5;                   // For LFXT

    LCDCPCTL0 = 0xFFF0;     // Init. LCD segments 4, 6-15
    LCDCPCTL1 = 0xF83F;     // Init. LCD segments 16-21, 27-31
    LCDCPCTL2 = 0x00F8;     // Init. LCD segments 35-39

    PM5CTL0 &= ~LOCKLPM5;

    CSCTL0_H = CSKEY >> 8;                  // Unlock CS registers
    CSCTL4 &= ~LFXTOFF;                     // Enable LFXT
    do
    {
      CSCTL5 &= ~LFXTOFFG;                  // Clear LFXT fault flag
      SFRIFG1 &= ~OFIFG;
    }while (SFRIFG1 & OFIFG);               // Test oscillator fault flag
    CSCTL0_H = 0;                           // Lock CS registers

    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP | LCDSON;

    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;

    LCDCCPCTL = LCDCPCLKSYNC;               // Clock synchronization enabled

    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory

    LCDCCTL0 |= LCDON;

    return 0;
}

void lcd_clear()
{
    // Initially, clear all displays.
    LCDCMEMCTL |= LCDCLRM;
}

int lcd_nums() {
    if(count == 3) {
        if((dis_matrix[0]*100) + (dis_matrix[1]*10) + (dis_matrix[2]) > 100) {
            return 0;
        }
        return (dis_matrix[0]*100) + (dis_matrix[1]*10) + dis_matrix[2];
    }
    if(count == 1) {
        return dis_matrix[0];
    }
    if (count == 2) {
        return (dis_matrix[0]*10) + (dis_matrix[1]);
    }
    if(count == 0) {
        return 0;
    }
    return 0;
}

void msp_init()
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    P2DIR   |= BIT6 | BIT7;      // P2.6 and P2.7 set as output
    P2SEL1  &= ~(BIT6 | BIT7);   // connect P2.6 and P2.7
    P2SEL0  |= BIT6 | BIT7;      // to TB0.5 and TB0.6
    P3DIR   |= BIT6 | BIT7;      // P3.6 and P3.7 set as output
    P3SEL1  |= BIT6 | BIT7;      // connect P3.6 and P3.7
    P3SEL0  &= ~(BIT6 | BIT7);   // to TB0.2 and TB0.3

    P8DIR |= (BIT4 | BIT5 | BIT6 | BIT7);
    P8OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);
    P9DIR |= BIT0;
    P9OUT |= BIT0;

    TB0CCR0 = 100;              // (32768 Hz / 4) / 100 ticks = 81.92 Hz
    TB0CCR2 = 20;               // 20 / 100 = 20% duty cycle
    TB0CCR3 = 40;               // 40 / 100 = 40% duty cycle
    TB0CCR5 = 60;               // 60 / 100 = 60% duty cycle
    TB0CCR6 = 80;               // 80 / 100 = 80% duty cycle

    // set output mode to reset/set (see page 459 in user's guide - slau367f)
    TB0CCTL2 = TB0CCTL3 = TB0CCTL5 = TB0CCTL6 = OUTMOD_7;
    // clock source = ACLK divided by 4, put timer in UP mode, clear timer register
    TB0CTL = TBSSEL__ACLK | ID__4 | MC__UP | TBCLR;

    PM5CTL0 &= ~LOCKLPM5;       // Unlock ports from power manager

    P2DIR &= ~(BIT1);
    P2REN |= (BIT1);
    P2OUT |= (BIT1);
    P2IES |= (BIT1);
    P2IFG &= ~(BIT1);
    P2IE  |= (BIT1);

    P2DIR &= ~(BIT2);
    P2REN |= (BIT2);
    P2OUT |= (BIT2);
    P2IES |= (BIT2);
    P2IFG &= ~(BIT2);
    P2IE  |= (BIT2);

    P2DIR &= ~(BIT3);
    P2REN |= (BIT3);
    P2OUT |= (BIT3);
    P2IES |= (BIT3);
    P2IFG &= ~(BIT3);
    P2IE  |= (BIT3);

    P2DIR &= ~(BIT4);
    P2REN |= (BIT4);
    P2OUT |= (BIT4);
    P2IES |= (BIT4);
    P2IFG &= ~(BIT4);
    P2IE  |= (BIT4);
}

void main(void)
{
    msp_init();
    lcd_init();
    TB0CCR5 = 0;
    TB0CCR3 = 0;
    TB0CCR6 = 0;
    TB0CCR2 = 0;
    lcd_val = 0;
    __enable_interrupt();
    while(1){
        lcd_val = lcd_nums();
        lcd_clear();
        if(count != 0) {
            displayNum(lcd_val,count,count,count,count);
        }
        else {
            displayNum(0,0,0,0,1);
        }
        if(col == 3) {
            P8OUT |= (BIT4 | BIT5 | BIT6);
            P9OUT &= ~(BIT0);
            for(n = 800; n > 0; n--);
            col = 0;
        }
        if(col == 1) {
            P9OUT |= BIT0;
            P8OUT |= (BIT4 | BIT6);
            P8OUT &= ~(BIT5);
            for(n = 800; n > 0; n--);
            col++;
        }
        if(col == 0) {
            P9OUT |= BIT0;
            P8OUT |= (BIT5 | BIT6);
            P8OUT &= ~(BIT4);
            for(n = 800; n > 0; n--);
            col++;
        }
        if(col == 2) {
            P9OUT |= BIT0;
            P8OUT |= (BIT4 | BIT5);
            P8OUT &= ~(BIT6);
            for(n = 800; n > 0 ;n--);
            col++;
        }
    }
}

#pragma vector = PORT2_VECTOR      // associate funct. w/ interrupt vector
__interrupt void Port_2(void)      // name of ISR
{
    switch((P2IV))
    {
    case P2IV_NONE:   break;   // Vector  0: no interrupt
    case P2IV_P2IFG0: break;   // Vector  2: P2.0
    case P2IV_P2IFG1:
        if(col == 3) {
            dis_matrix[0] = 0;
            dis_matrix[1] = 0;
            dis_matrix[2] = 0;
            count = 0;
            TB0CCR3 = lcd_val;
        }
        if(col == 1) {
            if(count >= 3){
                break;
            }
            else {
                if(count == 2) {
                    if((dis_matrix[0]*100) + (dis_matrix[1]*10) + 2 > 100) {
                        break;
                    }
                }
                dis_matrix[count] = 2;
                count++;
            }
        }
        if(col == 2) {
            if(count >= 3){
                break;
            }
            else {
                if(count == 2) {
                    if((dis_matrix[0]*100) + (dis_matrix[1]*10) + 3 > 100) {
                        break;
                    }
                }
                dis_matrix[count] = 3;
                count++;
            }
        }
        if(col == 0) {
            if(count >= 3){
                break;
            }
            else {
                if(count == 2) {
                    if((dis_matrix[0]*100) + (dis_matrix[1]*10) + 1 > 100) {
                        break;
                    }
                }
                dis_matrix[count] = 1;
                count++;
            }
        }
        break;         // Vector  4: P2.1
    case P2IV_P2IFG2:
        if(col == 3) {
            dis_matrix[0] = 0;
            dis_matrix[1] = 0;
            dis_matrix[2] = 0;
            count = 0;
            TB0CCR2 = lcd_val;
        }
        if(col == 1) {
            if(count >= 3){
                break;
            }
            else {
                if(count == 2) {
                    if((dis_matrix[0]*100) + (dis_matrix[1]*10) + 5 > 100) {
                        break;
                    }
                }
                dis_matrix[count] = 5;
                count++;
            }
        }
        if(col == 2) {
            if(count >= 3){
                break;
            }
            else {
                if(count == 2) {
                    if((dis_matrix[0]*100) + (dis_matrix[1]*10) + 6 > 100) {
                        break;
                    }
                }
                dis_matrix[count] = 6;
                count++;
            }
        }
        if(col == 0) {
            if(count >= 3){
                break;
            }
            else {
                if(count == 2) {
                    if((dis_matrix[0]*100) + (dis_matrix[1]*10) + 4 > 100) {
                        break;
                    }
                }
                dis_matrix[count] = 4;
                count++;
            }
        }
        break;         // Vector  6: P2.2
    case P2IV_P2IFG3:
        if(col == 3) {
            dis_matrix[0] = 0;
            dis_matrix[1] = 0;
            dis_matrix[2] = 0;
            count = 0;
            TB0CCR5 = lcd_val;
        }
        if(col == 1) {
            if(count >= 3){
                break;
            }
            else {
                if(count == 2) {
                    if((dis_matrix[0]*100) + (dis_matrix[1]*10) + 8 > 100) {
                        break;
                    }
                }
                dis_matrix[count] = 8;
                count++;
            }
        }
        if(col == 2) {
            if(count >= 3){
                break;
            }
            else {
                if(count == 2) {
                    if((dis_matrix[0]*100) + (dis_matrix[1]*10) + 9 > 100) {
                        break;
                    }
                }
                dis_matrix[count] = 9;
                count++;
            }
        }
        if(col == 0) {
            if(count >= 3){
                break;
            }
            else {
                if(count == 2) {
                    if((dis_matrix[0]*100) + (dis_matrix[1]*10) + 7 > 100) {
                        break;
                    }
                }
                dis_matrix[count] = 7;
                count++;
            }
        }
        break;        // Vector  8: P2.3
    case P2IV_P2IFG4:
        if(col == 3) {
            dis_matrix[0] = 0;
            dis_matrix[1] = 0;
            dis_matrix[2] = 0;
            count = 0;
            TB0CCR6 = lcd_val;
        }
        if(col == 1) {
            if(count >= 3){
                break;
            }
            else {
                dis_matrix[count] = 0;
                count++;
            }
        }
        if(col == 2) {
            dis_matrix[0] = 0;
            dis_matrix[1] = 0;
            dis_matrix[2] = 0;
            count = 0;
        }
        if(col == 0) {
            if(count <= 0) {
                break;
            }
            dis_matrix[count] = 0;
            count--;
        }
        break;        // Vector 10: P2.4
    case P2IV_P2IFG5: break;   // Vector 12: P2.5
    case P2IV_P2IFG6: break;   // Vector 14: P2.6
    case P2IV_P2IFG7: break;    // Vector 16: P2.7
    default: break;
    }
    volatile unsigned int i;
    for(i=10000;i>0;i--);
}
