#include <xc.h>
#include <string.h>
#include "config.h"
#include "init.h"
#include "modbus.h"

void pic_init(void)
{
    OSCCON  = 0x60;
    OSCTUNE = 0x00;

    // ���������� ����� ������ �� ���������� ����� � �������� �����
    ANSEL   = 0x00;
    ANSELH  = 0x00;
    SLRCON  = 0x00;

    PORTA   = 0;                            /*����� ������������� ����� C, ��� ���� � ������ �������*/
    TRISA   = 0b00000011;                   /*���� ����� C ��������� �� ����� RA0 � RA1 ����*/
    PORTB   = 0;                            /*����� ������������� ����� C, ��� ���� � ������ �������*/
    TRISB   = 0b00000000;                   /*���� ����� C ��������� �� �����*/
    PORTB   = 0;                            /*����� ������������� ����� C, ��� ���� � ������ �������*/
    PORTC   = 0;                            /*����� ������������� ����� C, ��� ���� � ������ �������*/
    TRISC   = 0b10110000;                   /*���� ����� C ��������� �� ����� � RC4 RC5 RC7 ����*/
    
    //TRISBbits.RB4 = 1;
    //TRISBbits.RB6 = 0;

    /*�������� ������ 0 ��� ����������� ���������� ���� �������*/
    T0CON = 0b11000111;

    /*�������� ������ 1 ��� ����������� ���������� ���� ������� 1ms*/
    TMR1H = 0xFC;		    // Preload TMR1 register pair for 1 milsecond overflow
    TMR1L = 0xDF;
    T1CON = 0x00;
    T1CONbits.TMR1CS = 0;	    // Configure for internal clock,asynchronous operation, internal oscillator
    T1CONbits.T1SYNC = 0;
    T1CONbits.T1RUN = 0;
    T1CONbits.TMR1ON = 1;	    // Enable Timer1
    T1CONbits.T1CKPS0 = 0;
    T1CONbits.T1CKPS1 = 0;

    PIE1bits.TMR1IE = 1;	    // Enable Timer1 interrupt
    INTCONbits.T0IE = 1;	    // Enable Timer0 interrupt
    CLRWDT();
}

void load_data_from_eeprom(void)
{
    BYTE crc_out = 0xAA;
    BYTE crc = read_eeprom_my(0);
    memset(res_table, 0, sizeof(res_table));

    CLRWDT();
    addr=read_eeprom_my(1);                             /*������������� ������ ����� ����������*/
    res_table[0].byte.HB=0x00;				/*� ����� ��������� ������ ���������� ����� ����������*/
    res_table[0].byte.LB=read_eeprom_my(1);             /*� ����� ��������� ������ ���������� ����� ����������*/
    res_table[1].byte.HB=read_eeprom_my(2);		/*�������� ������ ������� ����*/
    res_table[1].byte.LB=read_eeprom_my(3);             /*�������� ������ ������� ����*/
    res_table[2].byte.HB=read_eeprom_my(4);		/*������ ��������*/
    res_table[2].byte.LB=read_eeprom_my(5);             /*������ ����� ����� ����������*/
    res_table[3].byte.HB=read_eeprom_my(6);		/*������ ��������*/
    res_table[3].byte.LB=read_eeprom_my(7);             /*������ ����� ����� ����������*/
    res_table[4].byte.HB=read_eeprom_my(8);		/*������ ��������*/
    res_table[4].byte.LB=read_eeprom_my(9);             /*������ ����� ����� ����������*/
    res_table[5].byte.HB=read_eeprom_my(10);		/*������ ��������*/
    res_table[5].byte.LB=read_eeprom_my(11);            /*������ ����� ����� ����������*/
    
    CLRWDT();
    for (int i=0; i<6; i++)
	crc_out ^= (res_table[i].byte.HB ^res_table[i].byte.LB);
        
    if (get_mbaddr_from_pin() != 0)
	addr = get_mbaddr_from_pin();                     /*������������� ������ ����� ����������*/
        
    CLRWDT();
    if (crc_out != crc)
    {
        addr=get_mbaddr_from_pin();                     /*������������� ������ ����� ����������*/
        res_table[0].byte.HB=0x00;			/*� ����� ��������� ������ ���������� ����� ����������*/
        res_table[0].byte.LB=addr;			/*� ����� ��������� ������ ���������� ����� ����������*/
        res_table[1].byte.HB=0x25;			/*�������� ������ ������� ����*/
        res_table[1].byte.LB=0x80;			/*�������� ������ ������� ����*/
        res_table[2].byte.HB=100;			/*������ ��������*/
        res_table[2].byte.LB=100;			/*������ ����� ����� ����������*/
        res_table[3].byte.HB=100;			/*������ ��������*/
        res_table[3].byte.LB=100;			/*������ ����� ����� ����������*/
        res_table[4].byte.HB=100;			/*������ ��������*/
        res_table[4].byte.LB=100;			/*������ ����� ����� ����������*/
        res_table[5].byte.HB=100;			/*������ ��������*/
        res_table[5].byte.LB=100;			/*������ ����� ����� ����������*/
	CLRWDT();
        for (int i=0; i<6; i++)
	    crc_out ^= (res_table[i].byte.HB ^res_table[i].byte.LB);
	CLRWDT();
	res_table[0].byte.HB = crc_out;
	CLRWDT();
	for (int i=0; i<6; i++)
	{
	    CLRWDT();
	    write_eeprom_my(i*2,res_table[i].byte.HB);
	    CLRWDT();
	    write_eeprom_my(i*2+1,res_table[i].byte.LB);
	}
	res_table[0].byte.HB = 0x00;
    }
}

BYTE get_mbaddr_from_pin(void)
{
    BYTE sw_value = 0;
    sw_value += (SW4 << 3);
    sw_value += (SW3 << 2);
    sw_value += (SW2 << 1);
    sw_value += (SW1);
    if (sw_value == 0)
	sw_value = 100;
    return sw_value;
}

void uart_init(WORD serialSpeed)
{

    serialSpeed = 9600;
    
    TX_TRIS = 0;                          /*��� ������ ����� �������� �� �����*/
    RX_TRIS = 1;                          /*��� ����� ����� �������� �� ����*/
    TX_TRANS = 0;                         /*��� ���������� ������������ �������� RS-485 �� �����*/
    TX_ENABLE = 0;                        /*����������� �� �����*/

    SPBRGH = 0;                           /*������������� ��������*/
    SPBRG = BAUD_CALC(serialSpeed);

    INTCONbits.PEIE = 1; // ENable peripheral interrupts.
    PIE1bits.RCIE   = 1; // ENABLE USART receive interrupt
    PIE1bits.TXIE   = 0; // disable USART TX interrupt

    TX9 = 0;
    TX9D = 0;
    TXEN = 1;
    SYNC = 0;
    BRGH = 0;
    BRG16 = 1;
    RCSTAbits.SPEN = 1; //Serial Port enabled
    RCSTAbits.RX9 = 0; //8bit reception
    RCSTAbits.CREN = 1;
    CLRWDT();
}

unsigned int BAUD_CALC(WORD x)
{
    unsigned long xlat = (unsigned long)_XTAL_FREQ;
    unsigned long var = (unsigned long)x;
    var *= 16;
    return ((unsigned int)(xlat / var) - 1); /*���������� �������� �� ������� ����������*/
}

CHAR read_eeprom_my (CHAR addr)
{
    EEADR = addr;
    EECON1bits.EEPGD = 0;
    EECON1bits.RD = 1;
    while (EECON1bits.RD) NOP();
    return EEDATA;
}

void write_eeprom_my (CHAR addr, CHAR eeval)
{
    EEADR = addr;   
    EEDATA = eeval;
    EECON1bits.EEPGD = 0;   
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1;
    BYTE bk = INTCONbits.GIE;
    INTCONbits.GIE = 0;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    // Wait for write to complete
    while (EECON1bits.WR);
    EECON1bits.WREN = 0;
    INTCONbits.GIE = bk;
}
