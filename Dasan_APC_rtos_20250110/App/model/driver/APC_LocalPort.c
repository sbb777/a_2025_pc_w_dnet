/*
 * APC_LocalPort.c
 *
 *  Created on: 2023. 8. 16.
 *      Author: Yongseok
 */

#include <APC_LocalPort.h>
#include <APC_Utils.h>

#define UART_Q_BUF_MAX    256

extern UART_HandleTypeDef huart1;


static uint16_t q_in = 0;
static uint16_t q_out = 0;
static uint8_t  q_buf[UART_Q_BUF_MAX];
//static uint8_t  q_data;


void initLocalPort(void)
{
    // XXX receive data using DMA
	HAL_UART_Receive_DMA(&huart1, &q_buf[0], UART_Q_BUF_MAX);
	//HAL_UART_Receive_IT(&huart1, &q_data, 1);
}

uint32_t availableLocalRS232(uint8_t ch)
{
	uint32_t ret = 0;

	switch(ch)
	{
		case APC_PORT_LOCAL:
			q_in = (UART_Q_BUF_MAX - huart1.hdmarx->Instance->NDTR) % UART_Q_BUF_MAX;
			ret = (UART_Q_BUF_MAX + q_in - q_out) % UART_Q_BUF_MAX;
			break;
	}

	return ret;
}

uint8_t readLocalRS232Data(uint8_t ch)
{
	uint8_t ret = 0;

	switch(ch)
	{
		case APC_PORT_LOCAL:
			if (q_out != q_in)
			{
				ret = q_buf[q_out];
				q_out = (q_out + 1) % UART_Q_BUF_MAX;
			}
			break;
	}

	return ret;
}


uint32_t writeLocalRS232Data(uint8_t ch, uint8_t *p_data, uint32_t length)
{
    uint32_t ret = 0;
    HAL_StatusTypeDef hal_ret;

    switch(ch)
    {
        case APC_PORT_LOCAL:
            hal_ret = HAL_UART_Transmit(&huart1, p_data, length, 50);
            //hal_ret = HAL_UART_Transmit(&huart1, p_data, length, 10);
            if (hal_ret == HAL_OK)
            {
                ret = length;
            }
            break;
    }

    return ret;
}

uint32_t printLocalRS232(uint8_t ch, const char *fmt, ...)
{
	uint32_t ret = 0;
	va_list arg;
	char print_buf[256];

	va_start(arg, fmt);
	int len = vsnprintf(print_buf, 256, fmt, arg);
	va_end(arg);

	if (len > 0)
	{
		ret = writeLocalRS232Data(ch, (uint8_t *)print_buf, len);
	}

	return ret;
}

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//  if (&huart1 == huart)
//  {
//    // Buffer Write
//    //
//    uint16_t q_in_next;
//
//    q_in_next = (q_in + 1) % UART_Q_BUF_MAX;
//    if (q_in_next != q_out)
//    {
//      q_buf[q_in] = q_data;
//      q_in = q_in_next;
//    }
//
//    HAL_UART_Receive_IT(&huart1, &q_data, 1);
//  }
//}
