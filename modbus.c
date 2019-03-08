#include <xc.h>
#include "config.h"
#include "init.h"
#include "modbus.h"

/*
 * ������� ��������� ������ ������
 */

void MODBUS_SLAVE(UART_DATA *MODBUS){
    unsigned int tmp;

    /*���� ���������� � ��� ��� ������������������ �������, � � ������ ������ 5 ����*/

    if (((MODBUS->buffer[0] == addr) | (MODBUS->buffer[0] == 255)) & (MODBUS->rxcnt > 5))
    {
	tmp = CRC16(MODBUS->buffer, MODBUS->rxcnt - 2); /*��������� ����������� �����*/
	/*���� ����������� ����� �������, ������������ ���������*/
	
	if ((MODBUS->buffer[MODBUS->rxcnt - 2] == (tmp & 0x00FF)) & (MODBUS->buffer[MODBUS->rxcnt - 1] == (tmp >> 8)))
	{
	    switch (MODBUS->buffer[1])
	    {
	    case 3: /*��������� ������� ������*/
		TX_03(MODBUS);
		break;
	    case 16: /*��������� ������� ������*/
		TX_16(MODBUS);
		break;
	    default: /*���� ������ ������� �� ��������������, �������� �������*/
		TX_ERROR(MODBUS, 0x01);
		break;
	    }
	    /*��������� ����������� �����*/

	    tmp = CRC16(MODBUS->buffer, MODBUS->txlen - 2);
	    MODBUS->buffer[MODBUS->txlen - 2] = tmp;
	    MODBUS->buffer[MODBUS->txlen - 1] = tmp >> 8;
	    MODBUS->txcnt = 0;

	}
	/*���������� �����*/

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
* ������� ������ ��������������� (03)
*/

void TX_03(UART_DATA *MODBUS)
{
unsigned int regAddr,regCount;
unsigned int m=0,n=0;

   regAddr=((MODBUS->buffer[2]<<8)+MODBUS->buffer[3]);  // ����� ������� ��������
   regCount=((MODBUS->buffer[4]<<8)+MODBUS->buffer[5]); // ���������� ��������� ��� ������

   n=3;
/*���� �� � ���� �������� ������������, �������� �������*/

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
   /*������� ������ �� ��������������*/

   TX_ERROR(MODBUS,0x02);
  }
}
/*
 * ������� ������ ��������������� (16)
 */
void TX_16(UART_DATA *MODBUS)
{
    unsigned int regAddr;
    unsigned char regCount,j;

   regAddr=((MODBUS->buffer[2]<<8)+MODBUS->buffer[3]);                          /*adress*/
   regCount=((MODBUS->buffer[4]<<8)+MODBUS->buffer[5]); // ���������� ��������� ��� ������

   /*���� � ����� �������� ������������*/
    if((regAddr+regCount) <= OBJ_SZ)
    {
       j=7;
       for (unsigned int i=regAddr; i < (regAddr + regCount);i++)
       {
	   // ���� ��� �������� ���������� ��������� �� ��������
	   // � ��������� ����� ��� ��� ����
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
    /*������� ������ �� ��������������*/
    TX_ERROR(MODBUS,0x02) ;
   }
}
/*
 * ������� �������� ��������� �������
 */
void TX_FRAME(UART_DATA *MODBUS)
{
    TX_TRANS = 0;
    RCSTAbits.CREN=0;
    TXSTAbits.TXEN=1;
    INTCONbits.GIE=0;                                             /* ����� �������� �������� ����������*/
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
 * ������ ������������ ������
 */

void TX_ERROR(UART_DATA *MODBUS,unsigned char error_type)
{
 /*���������� ������*/

    MODBUS->buffer[1]|=0b10000000;
    MODBUS->buffer[2]=error_type;                                       /*��� ������*/
    MODBUS->txlen=5;                                                    /*����� ���������*/
}

/*
 * ������� ������� ����������� �����
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
