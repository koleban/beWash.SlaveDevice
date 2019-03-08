#ifndef INIT_H
#define	INIT_H

#include "type_def.h"

#define     TX_TRIS     TRISBbits.TRISB7
#define     RX_TRIS     TRISBbits.TRISB5
#define     TX_TRANS    TRISCbits.RC6
#define     TX_ENABLE   PORTCbits.RC6

#define     RL1         PORTCbits.RC0
#define     RL2         PORTCbits.RC1
#define     RL3         PORTCbits.RC2
#define     RL4         PORTCbits.RC3

#define     RLED1       PORTCbits.RC0
#define     RLED2       PORTCbits.RC1
#define     RLED3       PORTCbits.RC2
#define     RLED4       PORTCbits.RC3

#define     SW1         PORTBbits.RB4
#define     SW2         PORTBbits.RB6
#define     SW3         PORTCbits.RC7
#define     SW4         PORTCbits.RC5

#define     IN1         PORTAbits.RA0
#define     IN2         PORTAbits.RA1

#define     PULSE_MIN_WIDTH     30  // msec
#define     PULSE_MAX_WIDTH     800  // msec

void pic_init(void);
void uart_init(WORD serialSpeed);
void load_data_from_eeprom(void);
BYTE get_mbaddr_from_pin(void);
CHAR read_eeprom_my (CHAR addr);
void write_eeprom_my (CHAR addr, CHAR eeval);
unsigned int BAUD_CALC(WORD x);

#endif

