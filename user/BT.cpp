#include "main.h"
#include <stdlib.h>
#include <stdio.h> /* for printf */
#include <string.h> /* for printf */
#include "logUART.h"

extern classLog log;

extern volatile int USART2_DMA_TX_Complete;

void BT_Send_String(char *str);

//char dma_str[1024];

/*
  Name  : CRC-8
  Poly  : 0x31    x^8 + x^5 + x^4 + 1
  Init  : 0xFF
  Revert: false
  XorOut: 0x00
  Check : 0xF7 ("123456789")
  MaxLen: 15 байт(127 бит) - обнаружение
    одинарных, двойных, тройных и всех нечетных ошибок
*/
uint8_t CRC8(char *pcBlock, unsigned int len)
{
    unsigned char _crc = 0xFF;
    unsigned int i;

    while (len--)
    {
        _crc ^= *pcBlock++;

        for (i = 0; i < 8; i++)
            _crc = _crc & 0x80 ? (_crc << 1) ^ 0x31 : _crc << 1;
    }
    return _crc;
}

void BT_Send_RAW(char *str) {

	int max = strlen(str);

	if (max < 128) {
		log.w((char *)"TX",'D', str);
	} else {

//		SEGGER_RTT_printf(0, "%s%sTX>%s", RTT_CTRL_TEXT_BLACK,
//		RTT_CTRL_BG_BRIGHT_YELLOW, RTT_CTRL_RESET);
//		SEGGER_RTT_printf(0, "%s%s>len:%d%s\n", RTT_CTRL_TEXT_BRIGHT_WHITE,
//		RTT_CTRL_BG_GREEN, strlen(str), RTT_CTRL_RESET);

	}

	int index = 0;

	//Ждем завершение прошлой пересылки
	while (USART2_DMA_TX_Complete)
	{
	   __NOP();
	}

	DMA1_Channel7->CCR &= ~DMA_CCR_EN;   //Выключаем DMA
	DMA1_Channel7->CCR &= ~DMA_CCR_HTIE; //Выкл перрывание по половине
	DMA1_Channel7->CCR &= ~DMA_CCR_TEIE; //Выкл прерывание по ошибке
	DMA1_Channel7->CCR |= DMA_CCR_TCIE;  //Вкл  перрывание завершению
	DMA1_Channel7->CPAR = 0x40004428;    //Адресс USART2 TDR
    DMA1_Channel7->CMAR = (uint32_t) str;
    DMA1_Channel7->CNDTR = max;
    USART2->CR3 |= USART_CR3_DMAT;       //Включение DMA передатчик
    USART2_DMA_TX_Complete = 1;
    DMA1_Channel7->CCR |= DMA_CCR_EN;   //Включаем DMA

//	while (1) {

//		while (1) {
//			dma_str[index % 1024] = str[index];
//			index++;
//
//			if (index >= max)
//				break;
//
//			if (index % 1024 == 0)
//				break;
//
//		}

//		U3_DMA_TX_Complete = 0;
//		//HAL_UART_Transmit_DMA(&huart3, (uint8_t*)str, strlen(str));
//		DMA1_Channel7->CR &= ~DMA_SxCR_EN; //Выключаем DMA
//
//		DMA1_Channel7->CR &= ~DMA_SxCR_HTIE;
//		DMA1_Channel7->CR |= DMA_SxCR_TCIE;
//
//		if (index % 1024 == 0)
//			DMA1_Channel7->NDTR = 1024;
//        else
//        	DMA1_Channel7->NDTR = index % 1024;
//
//		DMA1_Channel7->M0AR = (uint32_t) &dma_str[0];
//		DMA1_Channel7->CR |= DMA_SxCR_EN; //Включаем DMA
//
//		while (U3_DMA_TX_Complete == 0)
//			__NOP();
//
//		if (index >= max)
//			break;
//	}

}


char str16_1[1024];

void BT_Send_String(char *str) {
	//char str16[64];

	uint8_t crc;

	//sprintf(str16, "98#%s", str);

	crc = CRC8(&str[0], strlen(str));

	sprintf(str16_1, "!%s;%d$\n", str, crc);

	//SEGGER_RTT_printf(0, "%s%sTX>%s", RTT_CTRL_TEXT_BLACK,
	//RTT_CTRL_BG_BRIGHT_YELLOW, RTT_CTRL_RESET);
	//SEGGER_RTT_printf(0, "%s%s%s%s\n", RTT_CTRL_TEXT_BRIGHT_WHITE,
	//RTT_CTRL_BG_GREEN, str16_1, RTT_CTRL_RESET);

	BT_Send_RAW(&str16_1[0]);

	//HAL_UART_Transmit(&huart3, (uint8_t*) &str16_1[0], strlen(str16_1), 1000);

}
