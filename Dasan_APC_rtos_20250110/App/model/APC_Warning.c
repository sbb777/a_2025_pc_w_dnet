/*
 * APC_Warning.c
 *
 *  Created on: 2023. 11. 13.
 *      Author: Yongseok
 */

#include "APC_Warning.h"

static bool _warning_initialized = false;

EN_WarningStatus                MO_WarningStatus;
EN_WarningServiceRequest        MO_WarningServiceRequest;
EN_WarningLearnDatasetPresent   MO_WarningLearnDatasetPresent;
EN_WarningPFOBattery            MO_WarningPFOBattery;
EN_WarningCompressedAir         MO_WarningCompressedAir;

static int    warning_code;


void initWarning(void)
{
    warning_code = -1;

    MO_WarningStatus         = WarningStatus_NoWarnings;
    MO_WarningServiceRequest = WarningService_NoRequest;
    MO_WarningLearnDatasetPresent = WarningLearnDataset_Present;
    MO_WarningCompressedAir  = WarningCompressedAir_Ok;
    MO_WarningPFOBattery     = WarningPFOBattery_Ready;

    _warning_initialized = true;
}

//uint8_t getWarningCode()
//{
//    if (MO_WarningStatus == WarningStatus_Warnings) {
//        return warning_code;
//    } else {
//        return 0xFF;
//    }
//}

bool setWarningCode(uint8_t warn_no)
{
    if (warn_no >= 0) {
        warning_code = warn_no;
        MO_WarningStatus = WarningStatus_Warnings;

        if (warn_no == Warning_ServiceRequest) {
            MO_WarningServiceRequest = WarningService_Request;
        } else if (warn_no == Warning_LearnParameter) {
            MO_WarningLearnDatasetPresent = WarningLearnDataset_NotPresent;
        } else if (warn_no == Warning_CompressedAir) {
            MO_WarningCompressedAir = WarningCompressedAir_Nok;
        } else if (warn_no == Warning_PFOnotReady) {
            MO_WarningPFOBattery = WarningPFOBattery_NotReady;
        } else {
            ;
        }
        return true;
    }
    return false;
}

uint8_t getWarningStatus(void)
{
    return MO_WarningStatus;
}

uint8_t getWarningServiceRequest(void)
{
    /*
It is indicated when the control unit detects that motor steps are apparently not effective.
This may happen when the valve is heavily contaminated or the gate seal is heavily sticking.
These 'lost' steps are recognized and will be repeated to attempt target position in the short
term. But in the medium term the valve requires cleaning or inspection.
     */
    return MO_WarningServiceRequest;
}

uint8_t getWarningLearnDatasetPresent(void)
{
    return MO_WarningLearnDatasetPresent;
}

uint8_t getWarningCompressedAir(void)
{
    return MO_WarningCompressedAir;
}

uint8_t getWarningPFOBattery(void)
{
    return MO_WarningPFOBattery;
}
