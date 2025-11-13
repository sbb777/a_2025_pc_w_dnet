/**************************************************************************************
VAT Diagnostic Object Implementation
Provides real-time diagnostic data and statistics
**************************************************************************************/

#include "App_VAT_Diagnostic.h"
#include "App_VAT_CIP_Objects.h"
#include "App_DemoApplication.h"
#include "main.h"  /* For HAL_GetTick() */
#include <string.h>
#include <stdio.h>
#include <limits.h>

/* External global variables */
extern APP_DATA_T g_tAppData;

/* Global diagnostic data */
VAT_DIAGNOSTIC_DATA_T g_tVatDiagnostics = {0};

/* Private variables */
static uint32_t s_ulStartTick = 0;

/******************************************************************************
 * INITIALIZATION
 ******************************************************************************/
void VAT_Diagnostic_Init(void)
{
    memset(&g_tVatDiagnostics, 0, sizeof(VAT_DIAGNOSTIC_DATA_T));

    /* Initialize min/max values */
    g_tVatDiagnostics.sPressureMin = INT16_MAX;
    g_tVatDiagnostics.sPressureMax = INT16_MIN;
    g_tVatDiagnostics.sPositionMin = INT16_MAX;
    g_tVatDiagnostics.sPositionMax = INT16_MIN;

    /* Firmware version: 1.0.0.0 */
    g_tVatDiagnostics.ulFirmwareVersion = 0x01000000;

    /* Record start time */
    s_ulStartTick = HAL_GetTick();

    printf("[VAT Diag] Initialized (FW v1.0.0.0)\r\n");
}

/******************************************************************************
 * UPDATE (Called periodically - every 100ms)
 ******************************************************************************/
void VAT_Diagnostic_Update(void)
{
    /* Update uptime */
    g_tVatDiagnostics.ulUptimeSeconds = (HAL_GetTick() - s_ulStartTick) / 1000;

    /* Increment total cycles */
    g_tVatDiagnostics.ulTotalCycles++;

    /* Update pressure statistics */
    /* Assuming INPUT_ASSEMBLY_100_T structure: output[1:2] = pressure */
    int16_t sPressure = 0;
    memcpy(&sPressure, &g_tAppData.tOutputData.output[1], 2);

    if(sPressure < g_tVatDiagnostics.sPressureMin) {
        g_tVatDiagnostics.sPressureMin = sPressure;
    }
    if(sPressure > g_tVatDiagnostics.sPressureMax) {
        g_tVatDiagnostics.sPressureMax = sPressure;
    }

    /* Calculate average (over 100 samples) */
    g_tVatDiagnostics.lPressureSum += sPressure;
    g_tVatDiagnostics.usPressureSampleCount++;

    if(g_tVatDiagnostics.usPressureSampleCount >= 100) {
        g_tVatDiagnostics.sPressureAvg =
            (int16_t)(g_tVatDiagnostics.lPressureSum / 100);
        g_tVatDiagnostics.lPressureSum = 0;
        g_tVatDiagnostics.usPressureSampleCount = 0;
    }

    /* Update position statistics */
    /* output[3:4] = position */
    int16_t sPosition = 0;
    memcpy(&sPosition, &g_tAppData.tOutputData.output[3], 2);

    if(sPosition < g_tVatDiagnostics.sPositionMin) {
        g_tVatDiagnostics.sPositionMin = sPosition;
    }
    if(sPosition > g_tVatDiagnostics.sPositionMax) {
        g_tVatDiagnostics.sPositionMax = sPosition;
    }

    /* Update temperature (simulated - 25°C) */
    g_tVatDiagnostics.sTemperature = 25;
}

/******************************************************************************
 * ERROR RECORDING
 ******************************************************************************/
void VAT_Diagnostic_RecordError(uint16_t usErrorCode)
{
    g_tVatDiagnostics.usErrorCount++;
    g_tVatDiagnostics.usLastErrorCode = usErrorCode;
    g_tVatDiagnostics.ulLastErrorTimestamp = HAL_GetTick() / 1000;

    printf("[VAT Diag] ERROR: Code 0x%04X recorded (Total: %u)\r\n",
           usErrorCode, g_tVatDiagnostics.usErrorCount);
}

/******************************************************************************
 * RESET
 ******************************************************************************/
void VAT_Diagnostic_Reset(void)
{
    /* Save firmware version */
    uint32_t ulFwVer = g_tVatDiagnostics.ulFirmwareVersion;

    /* Reinitialize */
    VAT_Diagnostic_Init();

    /* Restore firmware version */
    g_tVatDiagnostics.ulFirmwareVersion = ulFwVer;

    printf("[VAT Diag] Statistics reset\r\n");
}

/******************************************************************************
 * CIP SERVICE HANDLER
 ******************************************************************************/
uint32_t VAT_Diagnostic_HandleService(
    APP_DNS_CHANNEL_HANDLER_RSC_T* ptDnsRsc,
    DNS_PACKET_CIP_SERVICE_IND_T*  ptInd,
    DNS_PACKET_CIP_SERVICE_RES_T*  ptRes,
    uint32_t*                      pulResDataSize)
{
    uint32_t ulGRC = CIP_GRC_SUCCESS;
    uint8_t bService = (uint8_t)ptInd->tData.ulService;
    uint8_t bInstance = (uint8_t)ptInd->tData.ulInstance;
    uint8_t bAttribute = (uint8_t)ptInd->tData.ulAttribute;

    /* Only instance 1 is supported */
    if(bInstance != 1) {
        printf("[VAT Diag] Invalid instance: %u\r\n", bInstance);
        return CIP_GRC_OBJECT_DOES_NOT_EXIST;
    }

    switch(bService) {
        case CIP_SERVICE_GET_ATTRIBUTE_SINGLE:
        {
            /* Read single attribute */
            uint8_t* pbSrc = NULL;
            uint8_t bSize = 0;

            switch(bAttribute) {
                case VAT_DIAG_ATTR_UPTIME:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.ulUptimeSeconds;
                    bSize = 4;
                    break;

                case VAT_DIAG_ATTR_TOTAL_CYCLES:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.ulTotalCycles;
                    bSize = 4;
                    break;

                case VAT_DIAG_ATTR_ERROR_COUNT:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.usErrorCount;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_LAST_ERROR_CODE:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.usLastErrorCode;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_LAST_ERROR_TIMESTAMP:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.ulLastErrorTimestamp;
                    bSize = 4;
                    break;

                case VAT_DIAG_ATTR_PRESSURE_MIN:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.sPressureMin;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_PRESSURE_MAX:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.sPressureMax;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_PRESSURE_AVG:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.sPressureAvg;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_POSITION_MIN:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.sPositionMin;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_POSITION_MAX:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.sPositionMax;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_NETWORK_RX_COUNT:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.ulNetworkRxCount;
                    bSize = 4;
                    break;

                case VAT_DIAG_ATTR_NETWORK_TX_COUNT:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.ulNetworkTxCount;
                    bSize = 4;
                    break;

                case VAT_DIAG_ATTR_NETWORK_ERROR_COUNT:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.usNetworkErrorCount;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_TEMPERATURE:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.sTemperature;
                    bSize = 2;
                    break;

                case VAT_DIAG_ATTR_FIRMWARE_VERSION:
                    pbSrc = (uint8_t*)&g_tVatDiagnostics.ulFirmwareVersion;
                    bSize = 4;
                    break;

                default:
                    ulGRC = CIP_GRC_ATTRIBUTE_NOT_SUPPORTED;
                    printf("[VAT Diag] Unsupported attribute: %u\r\n", bAttribute);
                    break;
            }

            if(pbSrc != NULL) {
                memcpy(ptRes->tData.abData, pbSrc, bSize);
                *pulResDataSize = bSize;
                printf("[VAT Diag] GET Attr%u: %u bytes\r\n", bAttribute, bSize);
            }
            break;
        }

        case CIP_SERVICE_GET_ATTRIBUTES_ALL:
        {
            /* Get all diagnostic data */
            uint8_t* pbDst = ptRes->tData.abData;

            /* Uptime */
            memcpy(pbDst, &g_tVatDiagnostics.ulUptimeSeconds, 4);
            pbDst += 4;

            /* Total cycles */
            memcpy(pbDst, &g_tVatDiagnostics.ulTotalCycles, 4);
            pbDst += 4;

            /* Error count */
            memcpy(pbDst, &g_tVatDiagnostics.usErrorCount, 2);
            pbDst += 2;

            /* Last error code */
            memcpy(pbDst, &g_tVatDiagnostics.usLastErrorCode, 2);
            pbDst += 2;

            /* Pressure min */
            memcpy(pbDst, &g_tVatDiagnostics.sPressureMin, 2);
            pbDst += 2;

            /* Pressure max */
            memcpy(pbDst, &g_tVatDiagnostics.sPressureMax, 2);
            pbDst += 2;

            /* Pressure avg */
            memcpy(pbDst, &g_tVatDiagnostics.sPressureAvg, 2);
            pbDst += 2;

            /* Position min */
            memcpy(pbDst, &g_tVatDiagnostics.sPositionMin, 2);
            pbDst += 2;

            /* Position max */
            memcpy(pbDst, &g_tVatDiagnostics.sPositionMax, 2);
            pbDst += 2;

            /* Temperature */
            memcpy(pbDst, &g_tVatDiagnostics.sTemperature, 2);
            pbDst += 2;

            /* Firmware version */
            memcpy(pbDst, &g_tVatDiagnostics.ulFirmwareVersion, 4);
            pbDst += 4;

            *pulResDataSize = (uint32_t)(pbDst - ptRes->tData.abData);

            printf("[VAT Diag] GET_ALL: %lu bytes\r\n", *pulResDataSize);
            break;
        }

        case CIP_SERVICE_RESET:
        {
            /* Reset statistics */
            VAT_Diagnostic_Reset();
            *pulResDataSize = 0;
            break;
        }

        default:
            ulGRC = CIP_GRC_SERVICE_NOT_SUPPORTED;
            printf("[VAT Diag] Unsupported service: 0x%02X\r\n", bService);
            break;
    }

    return ulGRC;
}
