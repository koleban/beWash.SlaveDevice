//
// Заголовочный файл протокола ModBus RTU
 //


#ifndef MODBUS_H
#define	MODBUS_H

#include "type_def.h"

//Формируем буфер для УАРТа//

#define BUF_SZ 255                      //Количество байт данных фрейма Modbus//
#define MODBUS_ADDR 2                 //Сетевой адрес modbus устройства//
#define OBJ_SZ 11                       //Общее количество регистров//

WORD_VAL res_table[OBJ_SZ];             //Таблица регистров модбас, с которых происходит чтение значений пареметров//
                                        //установленных пользователем//
                                        //[0] - модбас адрес устройства, можно применить для конфигурирования по сети//
                                        //[1] - скорость передачи УАРТ, можно конфигурировать по сети//
                                        //[2] - Ширина импульса в мс Канал 1 [2 байта] [байтH - Ширина импульса] [байтL - ширина паузы]//
                                        //[3] - Ширина импульса в мс Канал 2 [2 байта] [байтH - Ширина импульса] [байтL - ширина паузы]//
                                        //[4] - Ширина импульса в мс Канал 3 [2 байта] [байтH - Ширина импульса] [байтL - ширина паузы]//
                                        //[5] - Ширина импульса в мс Канал 4 [2 байта] [байтH - Ширина импульса] [байтL - ширина паузы]//
                                        //[6] - Количество импульсов Канал 1//
                                        //[7] - Количество импульсов Канал 2//
                                        //[8] - Количество импульсов Канал 3//
                                        //[9] - Количество импульсов Канал 4//
                                        //[10]- Счетчик успешных команд//

//УАРТ структура Modbus//

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

//Объявление прототипов функции//

void MODBUS_SLAVE(UART_DATA *MODBUS);
unsigned int CRC16(unsigned char *ptrByte, int byte_cnt);
void TX_03(UART_DATA *MODBUS);
void TX_16(UART_DATA *MODBUS);
void TX_ERROR(UART_DATA *MODBUS,unsigned char error_type);
void TX_FRAME(UART_DATA *MODBUS);

#endif	// MODBUS_H //



