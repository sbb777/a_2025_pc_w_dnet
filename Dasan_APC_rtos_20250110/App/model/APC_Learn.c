/*
 * APC_Learn.c
 *
 *  Created on: 2023. 11. 19.
 *      Author: Yongseok
 */

#include <APC_Learn.h>

#include <APC_Flash.h>
#include <APC_Warning.h>

/*********************************************************************************/
// Board(128) + Valve(64) + Sensor(64) + Controller(64) + RemotePort(64)
#define ADDR_LEARN_START       (_ADDR_MO_DATA + 128 + 64 + 64 + 64 + 64)

#define ADDR_LearnRunningStatus             (ADDR_LEARN_START +  0)
#define ADDR_LearnDatasetPresentStatus      (ADDR_LEARN_START +  1)
#define ADDR_LearnAbortStatus               (ADDR_LEARN_START +  2)
#define ADDR_LearnOpenPressureStatus        (ADDR_LEARN_START +  3)
#define ADDR_LearnClosePressureStatus       (ADDR_LEARN_START +  4)
#define ADDR_LearnPressureRaisingStatus     (ADDR_LEARN_START +  5)
#define ADDR_LearnPressureStabilityStatus   (ADDR_LEARN_START +  6)
// reserved 1-byte
#define ADDR_LearnPressureLimit             (ADDR_LEARN_START +  8)
//#define ADDR_LearnData                      (ADDR_LEARN_START + 12)

#define LDR_SPEEDY_TOTAL_CNT        43
#define LDR_NORMAL_TOTAL_CNT        16
#define LDR_STABLE_TOTAL_CNT        35
#define LDC_RAW_TOTAL_CNT           (LDR_SPEEDY_TOTAL_CNT+LDR_NORMAL_TOTAL_CNT+LDR_STABLE_TOTAL_CNT)

/*********************************************************************************/
static bool _learn_initialized = false;

static EN_LearnRunningStatus           MO_LearnRunningStatus       ;
static EN_LearnDatasetPresentStatus    MO_LearnDatasetPresentStatus;
static EN_LearnAbortStatus             MO_LearnAbortStatus         = 0;
static EN_LearnOpenPressureStatus      MO_LearnOpenPressureStatus  = 0;
static EN_LearnClosePressureStatus     MO_LearnClosePressureStatus = 0;
static EN_LearnPressureRaisingStatus   MO_LearnPressureRaisingStatus = 0;
static EN_LearnPressureStabilityStatus MO_LearnPressureStabilityStatus = 0;
static uint32_t                        MO_LearnPressureLimit = 10000000;

static int    MO_LearnMinConductance     = 0;
static int    MO_LearnRecommendedGasFlow = 0;

static uint8_t  MO_LearnData[LEARN_INDEX_SIZE][LEARN_DATA_LENGTH+1] = { 0x00 };
static uint8_t  TEST_LearnData[LEARN_INDEX_SIZE][LEARN_DATA_LENGTH+1] = {
        "0004000097500",
        "0007000098273",
        "0011000099000",
        "0014000088708",
        "0017000082150",
        "0021000076077",
        "0024000068673",
        "0027000063596",
        "0031000058895",
        "0034000053162",
        "0037000049232",
        "0041000045593",
        "0044000041155",
        "0047000038113",
        "0051000035295",
        "0054000031860",
        "0057000029505",
        "0061000027323",
        "0064000024664",
        "0067000022841",
        "0071000021152",
        "0074000019094",
        "0077000017682",
        "0081000016375",
        "0084000014781",
        "0087000013688",
        "0091000012676",
        "0094000011443",
        "0097000010597",
        "0101000009813",
        "0104000008858",
        "0107000008203",
        "0111000007597",
        "0114000006857",
        "0117000006351",
        "0121000005881",
        "0124000005309",
        "0127000004916",
        "0131000004553",
        "0134000004110",
        "0137000004110",
        "0140000004110",
        "0150000004110",
        "0160000004010",
        "0170000003955",
        "0180000003903",
        "0190000003854",
        "0200000003808",
        "0210000003765",
        "0220000003723",
        "0230000003683",
        "0240000003645",
        "0250000003609",
        "0260000003574",
        "0270000003541",
        "0280000003509",
        "0290000003478",
        "0300000003448",
        "0320000003419",
        "0340000003364",
        "0360000003312",
        "0380000003263",
        "0400000003210",
        "0420000003210",
        "0440000003210",
        "0460000003210",
        "0480000003210",
        "0500000003210",
        "0520000003210",
        "0540000003210",
        "0560000003210",
        "0580000003210",
        "0600000003210",
        "0620000003210",
        "0640000003210",
        "0660000003210",
        "0680000003210",
        "0700000003210",
        "0720000003210",
        "0740000003210",
        "0760000003210",
        "0780000003210",
        "0800000003210",
        "0820000003210",
        "0840000003210",
        "0860000003210",
        "0880000003210",
        "0900000003210",
        "0920000003210",
        "0940000003210",
        "0960000003210",
        "0980000003210",
        "1000000003210",
        "0000039AE9681",
        "0000000025000",
        "0000000001000",
        "00000P250M01L",
        "0000001000000",
        "000000000CA5E"
};

/*********************************************************************************/
void checkLearnCondition(void);

void initLearn(void)
{
    checkLearnCondition();

    _learn_initialized = true;
}

bool syncLearnData(uint32_t addr)
{
    bool result = true;

    if (addr >= _ADDR_MO_DATA && addr < _ADDR_LEARN_DATA) {
        result &= writeByteToFlash(ADDR_LearnRunningStatus,        MO_LearnRunningStatus);
        result &= writeByteToFlash(ADDR_LearnDatasetPresentStatus, MO_LearnDatasetPresentStatus);
        result &= writeByteToFlash(ADDR_LearnAbortStatus,          MO_LearnAbortStatus);
        result &= writeByteToFlash(ADDR_LearnOpenPressureStatus,   MO_LearnOpenPressureStatus);
        result &= writeByteToFlash(ADDR_LearnClosePressureStatus,  MO_LearnClosePressureStatus);
        result &= writeByteToFlash(ADDR_LearnPressureRaisingStatus, MO_LearnPressureRaisingStatus);
        result &= writeByteToFlash(ADDR_LearnPressureStabilityStatus, MO_LearnPressureStabilityStatus);
        result &= writeUint32ToFlash(ADDR_LearnPressureLimit,      MO_LearnPressureLimit);
    }
    else if (addr == _ADDR_LEARN_DATA) {
        for (int i = 0; i < LEARN_INDEX_SIZE; i++)
            result &= writeFlash(_ADDR_LEARN_DATA + i*LEARN_DATA_LENGTH, MO_LearnData[i], LEARN_DATA_LENGTH);

        MO_LearnRunningStatus = LearnRunningStatus_No;
        MO_LearnDatasetPresentStatus = LearnDatasetPresentStatus_Ok;
    }

    return result;
}

int startLearn(void)
{
//    enLearnStatusCode runLEARN(
//        bool     _isLearnSimulation,    // APC Simulation mode
//        double   _learnLimitPV,         // LEARN limit pressure
//       uint32_t _speedyRangeInterval,  // Valve close interval(ms) in LEARN SPEEDY range
//        uint32_t _normalRangeInterval,  // Valve close interval(ms) in LEARN NORMAL range
//        uint32_t _stableRangeInterval)  // Valve close interval(ms) in LEARN STABLE range
//    {
    // TODO learn

    uint8_t  tst_LearnData[103][LDL_ROW_LENGTH+1] = { 0x00 };
    int zNewPV = 0;

//        prtLog(LL_DEBUG, __FUNCTION__, "\n\n <---------------- (Start - LEARN...) ---------------->");

        // Set LEARN 구간별 interval time
//        learnIntervalTime[LDR_ID_SPEEDY] = _speedyRangeInterval;
//        learnIntervalTime[LDR_ID_NORMAL] = _normalRangeInterval;
//        learnIntervalTime[LDR_ID_STABLE] = _stableRangeInterval;

        // Set LEARN's pressure limit & status
//        setLearnLimit(_learnLimitPV);
//        setLearnStatus(LEARN_IN_PROGRESS, LEARN_ERROR_NONE);

        // LERAN을 진행. Open(100%) ---> Close(0%) 까지 진행
        static int lsSpeedDetailCnt = 1;    // SPEEDY 단계일 때 0.3% 씩 Close 하기 위해 필요한 counter
        uint16_t zIndex = 0;
        uint16_t zCloseRate = 0;
//        float    zOpenedPercent = LDP_MAX_PERCENT;  // Open 상태의 값

        setValveOpen();
//        setPV(getPV());     // 현재 상태에서 PV 값 획득
        uint32_t zOpenedPercent = 100000;   // 100.0%

        // 각 구간에 따른 다른 LEARN 정책을 수행
        // LEARN data는 일단 TmpData에 저장하고, Simul mode가 아니거나 LEARN procedure가 정상적으로 종료되면 정식 data로 복사하여 처리
/*        for(zIndex=(LDC_RAW_TOTAL_CNT-1); zIndex >= 0; zIndex--) {
            // Stable 구간
            if(zIndex >= (LDC_RQW_TOTAL_CNT - LDR_STABLE_TOTAL_CNT)) {
//                zCloseRate = 2000;
                zNewPV = getCurrentSensorVoltage();
                zOpenedPercent = zOpenedPercent - 2000;
                setSetpointValvePosition(zOpenedPercent);
            }
            // Normal 구간
            else if(zIndex >= (LDC_RQW_TOTAL_CNT - LDR_STABLE_TOTAL_CNT - LDR_NORMAL_TOTAL_CNT)) {
                zNewPV = getCurrentSensorVoltage();
                zOpenedPercent = zOpenedPercent - 1000;
                setSetpointValvePosition(zOpenedPercent);
            }
            // Speedy 구간 : 0.3%씩 Valve Close 하면서 처리하는 구간이다. 경계 부분에서는 0.4씩 Valve close (10.7, 10.4, 10.1, 9.7, 9.4, 9.1 ,,,)
            else {
                // 보통은 0.3을 close 하는데 0.4를 Close 하는 경우 검사
                if( (lsSpeedDetailCnt % (LDR_SPEEDY_DETAIL_CNT+1)) == 0) {
                    zCloseRate = learnValveCloseRate[LDR_ID_SPEEDY] + 0.10;     // 0.4% valve close
                    lsSpeedDetailCnt = 1;
                }
                else {
                    zCloseRate = learnValveCloseRate[LDR_ID_SPEEDY];            // 0.3% valve close
                }

                if(zIndex == 1) {   // 마지막 Valve 개도율을 0%로 만들기 위해
                  zCloseRate = learnValveCloseRate[LDR_ID_SPEEDY] + 0.10;
                }
                // 현재 Valve의 개도율 계산
                zNewPV = doLearnStep(LDR_ID_SPEEDY, starrLearnTmpData, zIndex, zCloseRate, zOpenedPercent, learnIntervalTime[LDR_ID_SPEEDY]);
                zOpenedPercent = zOpenedPercent - zCloseRate;

                // 0.4씩 Valve close를 check하기 위한 counter
                ++lsSpeedDetailCnt;

                // Check LEARN 종료 : index 0 -> terminated index
                if(zIndex == 0)
                    break;
            }  */
        for(zIndex=100; zIndex >= 0; zIndex--) {
            zNewPV = getCurrentSensorVoltage();
            zOpenedPercent = zOpenedPercent - 1000;
            setSetpointValvePosition(zOpenedPercent);
            sscanf(tst_LearnData[zIndex], "%03d%05d%08d", zIndex, (zOpenedPercent+1000)/100, getCurrentSensorVoltage());       // Learn Data 입력
            APC_Delay(10);
            while(!(getMotorStatus()&0x05)) APC_Delay(5);                           // Valve 멈춤 대기
            APC_Delay(100);     // Possible to change time interval
        }

        setSetpointValvePosition(100000);


            // LEARN 종료 판단
//            if(zNewPV >= getLearnLimit()) {
//                if(zIndex > 0) {    // 폐도율 0 까지 도달하기전에 Learn limit이 된 경우
//                    setLearnStatus(LEARN_FAILED, LEARN_ABORTED_BY_NOT_REACHED_0_CLOSED);
//                    prtLog(LL_DEBUG, __FUNCTION__, "\n\n <---------------- (End - LEARN (fail)... Reason=[%2d] ) ---------------->", LEARN_ABORTED_BY_NOT_REACHED_0_CLOSED);
//                    resetLEARN();
//                    return LEARN_FAILED;
//                }
//            }
//        }

        // LEARN 이 정상적으로 종료 됨 : TmpData를 정식 RawData로 복사
        // APC simulation mode가 아닐 때만 정식 LEARN data로 변환 (for _PC_TESTING)
        //if(!_isLearnSimulation) {
//        if(true) {          // 지금은 시험용으로 모두 Conversion 및 Printout 한다
//            copyLearnDataTmp2Raw();
//            prtLearnRawData(starrLearnRawData);

            // Convert Raw Learn data to Formal Learn data
//            convRawToFormalLearnData();
//            prtLearnFormalData();
//        }

    // --> Just for testing
    //convFormalToRawLearnData();
    //prtLearnRawData(starrLearnRawData);

        // 특히 simulation mode를 모두 false로 reset
//        resetLEARN();
//       setLearnStatus(LEARN_COMPLETED_SUCCESSFULLY, LEARN_ERROR_NONE);

        // LEARN이 종료되면 반드시 Valve를 Open 상태로 둔다
//        openValve();

//        prtLog(LL_DEBUG, __FUNCTION__, "\n\n <---------------- (End - LEARN (success)... ) ---------------->\n\n");
        return 0;
}

EN_LearnRunningStatus getLearnRunningStatus(void)
{
    return MO_LearnRunningStatus;
}
void setLearnRunningStatus(EN_LearnRunningStatus status)
{
    MO_LearnRunningStatus = status;
}

EN_LearnDatasetPresentStatus getLearnDatasetPresentStatus(void)
{
    return MO_LearnDatasetPresentStatus;
}
void setLearnDatasetPresentStatus(EN_LearnDatasetPresentStatus status)
{
    MO_LearnDatasetPresentStatus = status;
}

EN_LearnAbortStatus getLearnAbortStatus(void)
{
    return MO_LearnAbortStatus;
}
void setLearnAbortStatus(EN_LearnAbortStatus status)
{
    MO_LearnAbortStatus = status;
}

EN_LearnOpenPressureStatus getLearnOpenPressureStatus(void)
{
    return MO_LearnOpenPressureStatus;
}
void setLearnOpenPressureStatus(EN_LearnOpenPressureStatus status)
{
    MO_LearnOpenPressureStatus = status;
}

EN_LearnClosePressureStatus getLearnClosePressureStatus(void)
{
    return MO_LearnClosePressureStatus;
}
void setLearnClosePressureStatus(EN_LearnClosePressureStatus status)
{
    MO_LearnClosePressureStatus = status;
}

EN_LearnPressureRaisingStatus getLearnPressureRaisingStatus(void)
{
    return MO_LearnPressureRaisingStatus;
}
void setLearnPressureRaisingStatus(EN_LearnPressureRaisingStatus status)
{
    MO_LearnPressureRaisingStatus = status;
}

EN_LearnPressureStabilityStatus getLearnPressureStabilityStatus(void)
{
    return MO_LearnPressureStabilityStatus;
}
void setLearnPressureStabilityStatus(EN_LearnPressureStabilityStatus status)
{
    MO_LearnPressureStabilityStatus = status;
}


uint32_t getLearnPressureLimit(void)
{
    return MO_LearnPressureLimit;
}
void setLearnPressureLimit(uint32_t value)
{
    MO_LearnPressureLimit = value;

    // TODO start LEARN
}

void getLearnData(uint8_t index, uint8_t *pData)
{
    if (strcmp((char *) MO_LearnData[index], "") == 0)
        readFlash((_ADDR_LEARN_DATA + index*LEARN_DATA_LENGTH), MO_LearnData[index], LEARN_DATA_LENGTH);

    memcpy((char *)pData, (char *)MO_LearnData[index], LEARN_DATA_LENGTH);
}

void setLearnData(uint8_t index, uint8_t *pData, uint8_t length)
{
    memcpy((char *)MO_LearnData[index], (char *)pData, length);
    MO_LearnRunningStatus = LearnRunningStatus_Yes;
    //writeFlash((_ADDR_LEARN_DATA + index*LEARN_DATA_LENGTH), pData, LEARN_DATA_LENGTH);
}

int getLearnMinConductance()
{
    return MO_LearnMinConductance;
}

void setLearnMinConductance(int conductance)
{
    MO_LearnMinConductance = conductance;
}

int getLearnRecommendedGasFlow()
{
    return MO_LearnRecommendedGasFlow;
}
void setLearnRecommendedGasFlow(int gasFlow)
{
    MO_LearnRecommendedGasFlow = gasFlow;
}


uint8_t _ERASED[LEARN_DATA_LENGTH] = { 0x00 };

void checkLearnCondition(void)
{
    memset((char *) _ERASED, 0xff, LEARN_DATA_LENGTH);

    int i = 0;
    for ( ; i < LEARN_INDEX_SIZE; i++) {
        uint8_t data[LEARN_DATA_LENGTH] = { 0x00 };
        getLearnData(i, data);

        if (strncmp((char *) data, (char *) _ERASED, LEARN_DATA_LENGTH) == 0 ||
            strcmp( (char *) data, "") == 0) {
            setWarningCode(Warning_LearnParameter);
            break;
        }
    }

    MO_LearnDatasetPresentStatus = i == (LEARN_DATA_LENGTH - 1) ? LearnDatasetPresentStatus_Ok : LearnDatasetPresentStatus_No;
    MO_LearnRunningStatus        = LearnRunningStatus_No;
}

float findNearestValvePositionFromLearnData(double _newSV) {
    // LEARN raw data를 scan하여 newSV와의 오차가 가장 작은 Valve의 위치(개도율)를 찾아 return
    uint16_t zDataIndex = 0, zFindIndex = 0;
    double  zDeltaPressure  = 0.0;
    double  zMinGapPressure = getSensorFullScale();     // gap의 초기값은 최대값으로 설정

    // SV와 오차가 가장 작은(+-상관없이) Valve의 위치값을 찾는다
    for(zDataIndex=0; zDataIndex < LDC_RAW_TOTAL_CNT; zDataIndex++) {
//        zDeltaPressure = fabs(_newSV - MO_LearnData[zDataIndex]);     // .pressure);
        if(zDeltaPressure < zMinGapPressure) {  // Gap이 더 적은 값이다. assign valve's 개도율
            zFindIndex = zDataIndex;
            zMinGapPressure = zDeltaPressure;
            //prtLog(LL_DEBUG, __FUNCTION__, "\n  >> Find !! : index=[%3d], deltaPress=[%7.1f], minGap=[%7.1f], vPct=[%6.2f]",
            //                                  zFindIndex, zDeltaPressure, zMinGapPressure, starrLearnRawData[zDataIndex].openPercent);
        }
    }
    // 찾아낸 LEARN index를 설정
    uint16_t learnFindIndex = zDataIndex;
//    prtLog(LL_DEBUG, __FUNCTION__, "\n\n  > Find best new valve position pct. : openPct=[%6.2f], find PV=[%7.1f]\n",
//            starrLearnRawData[zFindIndex].openPercent, starrLearnRawData[zFindIndex].pressure);
    return 0.0;
//    return MO_LearnData[zFindIndex].openPercent;
}
