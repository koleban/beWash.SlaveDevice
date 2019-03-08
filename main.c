#include <string.h>
#include "config.h"
#include "init.h"
#include "modbus.h"

__EEPROM_DATA (0x00, MODBUS_ADDR,0x25,0x80,100,100,100,100);
__EEPROM_DATA (100,100,100,100,0x00, 0x00,0x00, 0x00);

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
	INTCONbits.T0IF = 0;                              /*Сбрасываем флаг прерывания от таймера 0*/
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
            RCSTAbits.CREN=0;                           // Отключаем приемник для сброса ошибки
            RCSTAbits.CREN=1;                           // Включаем приемник для продолжения работу УАРТ
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
    // Иницилизируем контроллер
    pic_init();
    
    // Загружаем начальные данные из EEPROM
    load_data_from_eeprom();

    int acnt = 0;
    while (acnt++ < addr)
    {
	PORTBbits.RB5 = 1;
	__delay_ms(200);
	PORTBbits.RB5 = 0;
	__delay_ms(100);
    }
    
    // Инициализируем USART
    uart_init(res_table[1].Val);
    // Инициализируем протокол MODBUS
    memset(MODBUS.buffer, 0, sizeof(MODBUS.buffer));
    MODBUS.delay=3;                                     //Устанавливаем задержку, для определения конца фрейма млдбас
    	    
    // Прерывания !!! Go go go ...
    INTCONbits.GIE  = 1; // ENABLE interrupts

    while(1)
    {
	CLRWDT();
        if (MODBUS.rxgap)
	{
            MODBUS_SLAVE(&MODBUS);
            MODBUS.rxgap=0;
        }
	
	RL1 = 0;
	int timePulse = 0;
	while ((IN1 == 0) && (timePulse < (PULSE_MAX_WIDTH+200)))
	{
	    timePulse++;
	    __delay_ms(1);
	    // Фильтр помех IN1 - RL1 настоеный на импульс не менее PULSE_MIN_WIDTH
	    // >>>>>>
	    // Пропускаем импульс, только если он длиннее чем PULSE_MIN_WIDTH
	    if ((addr == 99) && (timePulse >= PULSE_MIN_WIDTH))
		RL1 = 1;
	    // <<<<<<
	}
	if ((timePulse >= PULSE_MIN_WIDTH) && (timePulse <= PULSE_MAX_WIDTH))
	{
	    res_table[11].Val++;
	    // Фильтр помех IN1 - RL1 настоеный на импульс не менее PULSE_MIN_WIDTH
	    // >>>>>>
	    // Добиваем ширину имульса до того, что получили
	    if (addr == 99)
    		__delay_ms(PULSE_MIN_WIDTH);
	    // <<<<<<
	}
	RL1 = 0;
	if (timePulse > PULSE_MAX_WIDTH)
	{
	    timePulse = 0;
	    for (int tmp = 0; tmp < 3; tmp++)
	    {
		RL4 = 1;
		RL3 = 1;
		__delay_ms(200);
		RL4 = 0;
		RL3 = 0;
		__delay_ms(100);
	    }
	    RL4 = 0;
	    RL3 = 0;
	}
	    
	timePulse = 0;
	while ((IN2 == 0) && (timePulse < (PULSE_MAX_WIDTH+200)))
	{
	    timePulse++;
	    __delay_ms(1);
	}
	if ((timePulse >= PULSE_MIN_WIDTH) && (timePulse <= PULSE_MAX_WIDTH))
	{
	   res_table[12].Val++; 
	}
	if (timePulse > PULSE_MAX_WIDTH)
	{
	    for (int tmp = 0; tmp < 5; tmp++)
	    {
		RL4 = 1;
		RL3 = 1;
		__delay_ms(200);
		RL4 = 0;
		RL3 = 0;
		__delay_ms(100);
	    }
	    RL4 = 0;
	    RL3 = 0;
	}
	    
	if (MODBUS.write)
	{
	    INTCONbits.GIE=0;                                             /* время передачи отключим прерывания*/
	    for (int rlIndex = 0; rlIndex < 4; rlIndex++)
	    {
		// IMPULSE RL1 - RL4
		int cnt = res_table[rlIndex+6].Val;
		res_table[rlIndex+6].Val = 0;
		for (int i=0; i< cnt; i++)
		{
		    if (rlIndex == 0) RL1 = 1;
		    if (rlIndex == 1) RL2 = 1;
		    if (rlIndex == 2) RL3 = 1;
		    if (rlIndex == 3) RL4 = 1;
		    for (int m=0; m < res_table[rlIndex+2].byte.HB; m++)
		    {
			CLRWDT();
			__delay_ms(1);
		    }
		    if (rlIndex == 0) RL1 = 0;
		    if (rlIndex == 1) RL2 = 0;
		    if (rlIndex == 2) RL3 = 0;
		    if (rlIndex == 3) RL4 = 0;
		    for (int m=0; m < res_table[rlIndex+2].byte.LB; m++)
		    {
			CLRWDT();
			__delay_ms(1);
		    }
		}
	    }
	    INTCONbits.GIE=1;
        }
    }
}
