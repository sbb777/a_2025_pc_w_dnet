/*
 * APC_Learn.h
 *
 *  Created on: 2023. 11. 19.
 *      Author: Yongseok
 */

#ifndef _APC_LEARN_H_
#define _APC_LEARN_H_

#include <APC_Define.h>
#include <APC_Model.h>

/*********************************************************************************/
#define   LDR_SPEEDY_TOTAL_CNT   43      // LEARN Data Range SPEEDY의 총 count 수
#define   LDR_NORMAL_TOTAL_CNT   16
#define   LDR_STABLE_TOTAL_CNT   35

#define   LDC_RAW_TOTAL_CNT       (LDR_SPEEDY_TOTAL_CNT+LDR_NORMAL_TOTAL_CNT+LDR_STABLE_TOTAL_CNT)
#define   LDC_ETC_CNT              6         // LEARN formal data의 Pressure 정보 외 추가되는 기타 정보 개수

#define LEARN_DATA_TOTAL_CNT    (LDC_RAW_TOTAL_CNT+LDC_ETC_CNT)      // LEARN Formal data total counter
#define LDL_ROW_LENGTH         16         // 각 Row 의 길이 : 3 + 5 + 8 = (16 digit)

#define LEARN_INDEX_SIZE        LEARN_DATA_TOTAL_CNT
#define LEARN_DATA_LENGTH       (13)    // ( 8)



void initLearn(void);

bool syncLearnData(uint32_t addr);

int startLearn(void);

EN_LearnRunningStatus getLearnRunningStatus(void);
void setLearnRunningStatus(EN_LearnRunningStatus status);

EN_LearnDatasetPresentStatus getLearnDatasetPresentStatus(void);
void setLearnDatasetPresentStatus(EN_LearnDatasetPresentStatus status);

EN_LearnAbortStatus getLearnAbortStatus(void);
void setLearnAbortStatus(EN_LearnAbortStatus status);

EN_LearnOpenPressureStatus getLearnOpenPressureStatus(void);
void setLearnOpenPressureStatus(EN_LearnOpenPressureStatus status);

EN_LearnClosePressureStatus getLearnClosePressureStatus(void);
void setLearnClosePressureStatus(EN_LearnClosePressureStatus status);

EN_LearnPressureRaisingStatus getLearnPressureRaisingStatus(void);
void setLearnPressureRaisingStatus(EN_LearnPressureRaisingStatus status);

EN_LearnPressureStabilityStatus getLearnPressureStabilityStatus(void);
void setLearnPressureStabilityStatus(EN_LearnPressureStabilityStatus status);

uint32_t getLearnPressureLimit(void);
void setLearnPressureLimit(uint32_t value);

void getLearnData(uint8_t index, uint8_t *pData);
void setLearnData(uint8_t index, uint8_t *pData, uint8_t length);

int  getLearnMinConductance(void);
void setLearnMinConductance(int conductance);

int  getLearnRecommendedGasFlow(void);
void setLearnRecommendedGasFlow(int gasFlow);

float findNearestValvePositionFromLearnData(double _newSV);

#endif /* _APC_LEARN_H_ */
