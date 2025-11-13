/*
 * APC_ChipSelect.h
 *
 *  Created on: 2023. 11. 27.
 *      Author: Yongseok
 */

#ifndef _APC_CS_H_
#define _APC_CS_H_

#include <APC_Define.h>

#define LED_CS_         (_DEF_CH1 + 1)
#define SENSEMODE       (_DEF_CH2 + 1)
#define SPI_CS_IF_      (_DEF_CH3 + 1)
#define SPI_CS_DP_      (_DEF_CH4 + 1)
#define SPI_CS_MD_      (_DEF_CH5 + 1)
#define SPI_CS_E2_      (_DEF_CH6 + 1)
#define SPI_CS_GL_      (_DEF_CH7 + 1)
#define SPI_CS_PS_      (_DEF_CH8 + 1)

#define CS_IF1_         (_DEF_CH1 + 17)
#define CS_IF2_         (_DEF_CH2 + 17)
#define CS_IF3_         (_DEF_CH3 + 17)
#define CS_PS_          (_DEF_CH4 + 17)

/* Interface WRITE */
#define IFC_WRITE_U3    (_DEF_CH1 + 1)
#define IFC_WRITE_U7    (_DEF_CH2 + 1)
#define IFC_WRITE_U8    (_DEF_CH3 + 1)
#define IFC_RELAY_CLOSE (_DEF_CH6 + 1)
#define IFC_RELAY_OPEN  (_DEF_CH7 + 1)
#define IFC_SPI_CS_E2   (_DEF_CH8 + 1)
/* Interface READ */
#define IFC_CLOSE_INPUT (_DEF_CH6 + 1)
#define IFC_OPEN_INPUT  (_DEF_CH7 + 1)
#define IFC_PW_MD_ON    (_DEF_CH8 + 1)


void initChipSelect(void);

void setChipSelect(uint8_t order, bool on_off);

#endif /* _APC_CS_H_ */
