#include <xc.h>
#include <string.h>
#include "config.h"
#include "init.h"
#include "modbus.h"

void pic_init(void)
{
    OSCCON  = 0x60;
    OSCTUNE = 0x00;

    // Переключим режим вывода на аналоговых ногах в цифровой режим
    ANSEL   = 0x00;
    ANSELH  = 0x00;
    SLRCON  = 0x00;

    //*********************
    //***  PORTA
    PORTA   = 0;                            /*После инициализации порта C, все пины в низкий уровень*/
    //          76543210
    TRISA   = 0b00000011;                   /*Пины порта C настроены на выход RA0 и RA1 вход*/
    //*********************
    //***  PORTB
    PORTB   = 0;                            /*После инициализации порта C, все пины в низкий уровень*/
    //          76543210
    TRISB   = 0b01010000;                   /*Пины порта C настроены на выход*/
    //*********************
    //***  PORTC
    PORTC   = 0;                            /*После инициализации порта C, все пины в низкий уровень*/
    //          76543210
    TRISC   = 0b10110000;                   /*Пины порта C настроены на выход и RC4 RC5 RC7 вход*/

    //TRISBbits.RB4 = 1;
    //TRISBbits.RB6 = 0;

    /*Настроим таймер 0 для обеспечения системного тика времени*/
    T0CON = 0b11000111;

    /*Настроим таймер 1 для обеспечения системного тика времени 1ms*/
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
    memset(res_table, 0, sizeof(res_table));

    CLRWDT();
    addr=read_eeprom_my(1);                             /*Устанавливаем модбас адрес устройства*/
    res_table[0].byte.HB=0x00;				/*В карту регистров модбас записываем адрес устройства*/
    res_table[0].byte.LB=read_eeprom_my(1);             /*В карту регистров модбас записываем адрес устройства*/
    res_table[1].byte.HB=read_eeprom_my(2);		/*Скорость обмена старший байт*/
    res_table[1].byte.LB=read_eeprom_my(3);             /*Скорость обмена младший байт*/
    res_table[2].byte.HB=read_eeprom_my(4);		/*Ширина импульса*/
    res_table[2].byte.LB=read_eeprom_my(5);             /*Ширина паузы между импульсами*/
    res_table[3].byte.HB=read_eeprom_my(6);		/*Ширина импульса*/
    res_table[3].byte.LB=read_eeprom_my(7);             /*Ширина паузы между импульсами*/
    res_table[4].byte.HB=read_eeprom_my(8);		/*Ширина импульса*/
    res_table[4].byte.LB=read_eeprom_my(9);             /*Ширина паузы между импульсами*/
    res_table[5].byte.HB=read_eeprom_my(10);		/*Ширина импульса*/
    res_table[5].byte.LB=read_eeprom_my(11);            /*Ширина паузы между импульсами*/

    CLRWDT();

    BYTE hw_addr = get_mbaddr_from_pin();
    if (hw_addr != 0)
	addr = hw_addr;                     /*Устанавливаем модбас адрес устройства*/
}

BYTE get_mbaddr_from_pin(void)
{
    BYTE sw_value = 0;
    sw_value += (BYTE)(SW4 << 3);
    sw_value += (BYTE)(SW3 << 2);
    sw_value += (BYTE)(SW2 << 1);
    sw_value += (BYTE)(SW1);
    if (sw_value == 0)
	sw_value = 100;
    return sw_value;
}

void uart_init(WORD serialSpeed)
{

    serialSpeed = 9600;

    TX_TRIS = 0;                          /*пин выхода УАРТа настроен на выход*/
    RX_TRIS = 1;                          /*пин входа УАРТа настроен на вход*/
    TX_TRANS = 0;                         /*Пин управления направлением передачи RS-485 на выход*/
    TX_ENABLE = 0;                        /*Настраиваем на прием*/

    SPBRGH = 0;                           /*Устанавливаем скорость*/
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
    return ((unsigned int)(xlat / var) - 1); /*Вычисление скорости по входным переменным*/
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