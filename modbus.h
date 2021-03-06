//
// ������������ ���� ��������� ModBus RTU
 //


#ifndef MODBUS_H
#define	MODBUS_H

#include "type_def.h"

//��������� ����� ��� �����//

#define BUF_SZ 55                       //���������� ���� ������ ������ Modbus//
#define MODBUS_ADDR 99                  //������� ����� modbus ����������//
#define OBJ_SZ 15                       //����� ���������� ���������//
#define REG_IN1_COUNTER 11              // ������� �������� ��������� ���� 1
#define REG_IN2_COUNTER 12              // ������� �������� ��������� ���� 2

WORD_VAL res_table[OBJ_SZ];             //������� ��������� ������, � ������� ���������� ������ �������� ����������//
                                        //������������� �������������//
                                        //[0] - ������ ����� ����������, ����� ��������� ��� ���������������� �� ����//
                                        //[1] - �������� �������� ����, ����� ��������������� �� ����//
                                        //[2] - ������ �������� � �� ����� 1 [2 �����] [����H - ������ ��������] [����L - ������ �����]//
                                        //[3] - ������ �������� � �� ����� 2 [2 �����] [����H - ������ ��������] [����L - ������ �����]//
                                        //[4] - ������ �������� � �� ����� 3 [2 �����] [����H - ������ ��������] [����L - ������ �����]//
                                        //[5] - ������ �������� � �� ����� 4 [2 �����] [����H - ������ ��������] [����L - ������ �����]//
                                        //[6] - ���������� ��������� ����� 1//
                                        //[7] - ���������� ��������� ����� 2//
                                        //[8] - ���������� ��������� ����� 3//
                                        //[9] - ���������� ��������� ����� 4//
                                        //[10]- ������� �������� ������//
                                        //[11]- ������� ��������� 1  REG_IN1_COUNTER//
                                        //[12]- ������� ��������� 2  REG_IN2_COUNTER//
                                        //[13]- ������//
                                        //[14]- ������//

//���� ��������� Modbus//

typedef struct {
unsigned char buffer[BUF_SZ];
unsigned char rxtimer;
unsigned char rxcnt;
unsigned char txcnt;
unsigned char txlen;
unsigned char rxgap;
unsigned char delay;
unsigned char write;
} UART_DATA;

unsigned char addr;

//���������� ���������� �������//

void MODBUS_SLAVE(UART_DATA *MODBUS);
unsigned int CRC16(unsigned char *ptrByte, int byte_cnt);
void TX_03(UART_DATA *MODBUS);
void TX_16(UART_DATA *MODBUS);
void TX_ERROR(UART_DATA *MODBUS,unsigned char error_type);
void TX_FRAME(UART_DATA *MODBUS);

#endif	// MODBUS_H //



