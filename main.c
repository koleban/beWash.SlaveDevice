#include <string.h>
#include "config.h"
#include "init.h"
#include "modbus.h"

__EEPROM_DATA (0x00, MODBUS_ADDR,0x25,0x80,250,250,100,100);
__EEPROM_DATA (250,250,250,250,0x00, 0x00,0x00, 0x00);

volatile unsigned long ticks = 0;
volatile BYTE rl4state = 0;
UART_DATA MODBUS;

void __interrupt(high_priority) isr(void)
{
    //////////////////////////////////////////////
    // Timer0 Interrupt handler
    //////////////////////////////////////////////
    if ((INTCONbits.T0IF == 1) && (INTCONbits.T0IE == 1))
    {
        if((MODBUS.rxtimer++>MODBUS.delay)&(MODBUS.rxcnt>1))
           MODBUS.rxgap=1;
        else
           MODBUS.rxgap=0;
        TMR0H = 0;
	TMR0L = 0;
	INTCONbits.T0IF = 0;                              /*���������� ���� ���������� �� ������� 0*/
    }

    //////////////////////////////////////////////
    // Timer1 Interrupt handler
    //////////////////////////////////////////////
    if ((PIR1bits.TMR1IF == 1) && (PIE1bits.TMR1IE == 1))
    {
	ticks++;
	TMR1H = 0xFF;		    // Preload TMR1 register pair for 1 milsecond overflow
        TMR1L = 0x32;
	PIR1bits.TMR1IF = 0;	    // Clear interrupt flag
   }

    //////////////////////////////////////////////
    // USART Interrupt handler
    //////////////////////////////////////////////
    if (PIR1bits.RCIF)
    {
        if ((RCSTAbits.FERR == 1) || (RCSTAbits.OERR == 1))
	{
            RCSTAbits.CREN=0;                           // ��������� �������� ��� ������ ������
            RCSTAbits.CREN=1;                           // �������� �������� ��� ����������� ������ ����
        }
        while (PIR1bits.RCIF)
	{
            MODBUS.rxtimer=0;
            if(MODBUS.rxcnt>(BUF_SZ-2)) MODBUS.rxcnt=0;
	    unsigned char bfVal = RCREG;
            MODBUS.buffer[MODBUS.rxcnt++]=bfVal;
        }
	PIR1bits.RCIF=0;
    }
}

void main(void)
{
    // ������������� ����������
    pic_init();
    
    // ��������� ��������� ������ �� EEPROM
    load_data_from_eeprom();

    int acnt = 0;
    while (acnt++ < addr)
    {
	PORTBbits.RB5 = 1;
	__delay_ms(200);
	PORTBbits.RB5 = 0;
	__delay_ms(100);
    }
    
    // �������������� USART
    uart_init(res_table[1].Val);
    // �������������� �������� MODBUS
    memset(MODBUS.buffer, 0, sizeof(MODBUS.buffer));
    MODBUS.delay=3;                                     //������������� ��������, ��� ����������� ����� ������ ������
    	    
    // ���������� !!! Go go go ...
    INTCONbits.GIE  = 1; // ENABLE interrupts

    while(1)
    {
	CLRWDT();
        if (MODBUS.rxgap)
	{
            MODBUS_SLAVE(&MODBUS);
            MODBUS.rxgap=0;
        }
	
	if (MODBUS.write)
	{
	    INTCONbits.GIE=0;                                             /* ����� �������� �������� ����������*/
	    // IMPULSE RL1
	    int cnt = res_table[6].Val;
	    res_table[6].Val = 0;
	    for (int i=0; i< cnt; i++)
	    {
		RL1 = 1;
		for (int m=0; m < res_table[2].byte.HB; m++)
		{
		    CLRWDT();
		    __delay_ms(1);
		}
		RL1 = 0;
		for (int m=0; m < res_table[2].byte.LB; m++)
		{
		    CLRWDT();
		    __delay_ms(1);
		}
	    }
	    // IMPULSE RL2
	    cnt = res_table[7].Val;
	    res_table[7].Val = 0;
	    for (int i=0; i< cnt; i++)
	    {
		RL2 = 1;
		for (int m=0; m < res_table[3].byte.HB; m++)
		{
		    CLRWDT();
		    __delay_ms(1);
		}
		RL2 = 0;
		for (int m=0; m < res_table[3].byte.LB; m++)
		{
		    CLRWDT();
		    __delay_ms(1);
		}
	    }
	    // IMPULSE RL3
	    cnt = res_table[8].Val;
	    res_table[8].Val = 0;
	    for (int i=0; i< cnt; i++)
	    {
		RL3 = 1;
		for (int m=0; m < res_table[4].byte.HB; m++)
		{
		    CLRWDT();
		    __delay_ms(1);
		}
		RL3 = 0;
		for (int m=0; m < res_table[4].byte.LB; m++)
		{
		    CLRWDT();
		    __delay_ms(1);
		}
	    }
	    // IMPULSE RL4
	    cnt = res_table[9].Val;
	    res_table[9].Val = 0;
	    for (int i=0; i< cnt; i++)
	    {
		RL4 = 1;
		for (int m=0; m < res_table[5].byte.HB; m++)
		{
		    CLRWDT();
		    __delay_ms(1);
		}
		RL4 = 0;
		for (int m=0; m < res_table[5].byte.LB; m++)
		{
		    CLRWDT();
		    __delay_ms(1);
		}
	    }
	    INTCONbits.GIE=1;
        }
    }
}
