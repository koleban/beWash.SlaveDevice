#include <xc.h>
#include "config.h"
#include "init.h"
#include "modbus.h"

/*
 * Функция обработки модбас фрейма
 */

void MODBUS_SLAVE(UART_DATA *MODBUS){
    unsigned int tmp;

    /*если обращаются к нам или широковещательныйм адресом, и в буфере больше 5 байт*/

    if (((MODBUS->buffer[0] == addr) | (MODBUS->buffer[0] == 255)) & (MODBUS->rxcnt > 5))
    {
	tmp = CRC16(MODBUS->buffer, MODBUS->rxcnt - 2); /*проверяем контрольную сумму*/
	/*если контрольная сумма совпала, обрабатываем сообщение*/
	
	if ((MODBUS->buffer[MODBUS->rxcnt - 2] == (tmp & 0x00FF)) & (MODBUS->buffer[MODBUS->rxcnt - 1] == (tmp >> 8)))
	{
	    switch (MODBUS->buffer[1])
	    {
	    case 3: /*Обработка команды чтения*/
		TX_03(MODBUS);
		break;
	    case 16: /*Обработка команды запись*/
		TX_16(MODBUS);
		break;
	    default: /*Если пришла команда не поддерживаемая, отвечаем ошибкой*/
		TX_ERROR(MODBUS, 0x01);
		break;
	    }
	    /*добавляем контрольную сумму*/

	    tmp = CRC16(MODBUS->buffer, MODBUS->txlen - 2);
	    MODBUS->buffer[MODBUS->txlen - 2] = tmp;
	    MODBUS->buffer[MODBUS->txlen - 1] = tmp >> 8;
	    MODBUS->txcnt = 0;

	}
	/*отправляем ответ*/

	TX_FRAME(MODBUS);
	MODBUS->rxgap = 0;
	MODBUS->rxcnt = 0;
	MODBUS->rxtimer = 0x00;
    }
    else
    {
	MODBUS->rxgap = 0;
	MODBUS->rxcnt = 0;
	MODBUS->rxtimer = 0x00;
    }
}
/*
* Функция чтении мультирегистров (03)
*/

void TX_03(UART_DATA *MODBUS)
{
unsigned int regAddr,regCount;
unsigned int m=0,n=0;

   regAddr=((MODBUS->buffer[2]<<8)+MODBUS->buffer[3]);  // Адрес первого регистра
   regCount=((MODBUS->buffer[4]<<8)+MODBUS->buffer[5]); // Количество регистров для чтения

   n=3;
/*если не в наше адресное пространство, отвечаем ошибкой*/

  if((regCount > 0) & (((regAddr + (regCount - 1)) < OBJ_SZ) & (regAddr + regCount <= OBJ_SZ)))
   {

    for(m=0;m<regCount;m++)
    {
     MODBUS->buffer[n]=res_table[regAddr + m].byte.HB;
     MODBUS->buffer[n+1]=res_table[regAddr + m].byte.LB;
     n+=2;
    }

     MODBUS->buffer[2]=m*2;                                         /*byte count*/
     MODBUS->txlen=m*2+5;                                           /*responce length*/

   }
  else
  {
   /*область памяти не поддерживается*/

   TX_ERROR(MODBUS,0x02);
  }
}
/*
 * Функция записи мультирегистров (16)
 */
void TX_16(UART_DATA *MODBUS)
{
    unsigned int regAddr;
    unsigned char regCount,j;

   regAddr=((MODBUS->buffer[2]<<8)+MODBUS->buffer[3]);                          /*adress*/
   regCount=((MODBUS->buffer[4]<<8)+MODBUS->buffer[5]); // Количество регистров для записи

   /*если в нашем адресном пространстве*/
    if((regAddr+regCount) <= OBJ_SZ)
    {
       j=7;
       for (unsigned int i=regAddr; i < (regAddr + regCount);i++)
       {
	   // Если это регистры накопленых импульсов то минусуем
	   // в остальные пишем так как есть
	    if ((i == REG_IN1_COUNTER) || (i == REG_IN2_COUNTER))
	    {
		if (res_table[i].Val >= (MODBUS->buffer[j]<<8)+MODBUS->buffer[j+1])
		    res_table[i].Val = res_table[i].Val - (MODBUS->buffer[j]<<8)+MODBUS->buffer[j+1];
		else
		    res_table[i].Val = 0;
	    }
	    else
	    {
		res_table[i].Val=(MODBUS->buffer[j]<<8)+MODBUS->buffer[j+1];
	    }
            j+=2;
        }
       MODBUS->txlen=8;
       MODBUS->write=1;
       res_table[10].Val++;
    }
   else
   {
    /*область памяти не поддерживается*/
    TX_ERROR(MODBUS,0x02) ;
   }
}
/*
 * Функция отправки сообщения мастеру
 */
void TX_FRAME(UART_DATA *MODBUS)
{
    TX_TRANS = 0;
    RCSTAbits.CREN=0;
    TXSTAbits.TXEN=1;
    INTCONbits.GIE=0;                                             /* время передачи отключим прерывания*/
    TX_ENABLE = 1;
    while(MODBUS->txcnt<MODBUS->txlen)
    {
       TXREG=MODBUS->buffer[MODBUS->txcnt++];
       while(!PIR1bits.TXIF) NOP();
    }
    TX_ENABLE = 0;
    INTCONbits.GIE=1;
    MODBUS->txlen=0;
    RCSTAbits.CREN=1;
}
/*
 * Фукция формирования ошибки
 */

void TX_ERROR(UART_DATA *MODBUS,unsigned char error_type)
{
 /*Отправляем ошибку*/

    MODBUS->buffer[1]|=0b10000000;
    MODBUS->buffer[2]=error_type;                                       /*код ошибки*/
    MODBUS->txlen=5;                                                    /*длина сообщения*/
}

/*
 * Функция расчета контрольной суммы
 */
unsigned int CRC16(unsigned char *ptrByte, int byte_cnt)
{
	unsigned int w=0;
	char shift_cnt;

	if(ptrByte)
	{
   		w=0xffffU;
   		for(; byte_cnt>0; byte_cnt--)
   		{
    		w=(unsigned int)((w/256U)*256U+((w%256U)^(*ptrByte++)));
    		for(shift_cnt=0; shift_cnt<8; shift_cnt++)
    		{
	     		if((w&0x1)==1)
	     			w=(unsigned int)((w>>1)^0xa001U);
	     		else
	     			w>>=1;
	    	}
   		}
	}
	return w;
}
