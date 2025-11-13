/**************************************************************************************
 * STM32 DeviceNet Explicit Message Debugging Code
 *
 * Purpose: Add this code to main.c to debug Explicit message communication issues
 *
 * Usage:
 * 1. Add PrintDeviceNetStatus() to main.c after stack initialization
 * 2. Compile and run
 * 3. Check UART output for status information
 **************************************************************************************/

#include "App_DemoApplication.h"
#include "Hil_DualPortMemory.h"

/**************************************************************************************
 * Print DeviceNet Stack Status
 *
 * Call this function after AppDNS_ConfigureStack() to verify stack initialization
 **************************************************************************************/
void PrintDeviceNetStatus(APP_DATA_T* ptAppData)
{
    CHANNEL_INFORMATION tInfo;
    int32_t lRet;

    // Get channel information
    lRet = xChannelInfo(ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->hChannel,
                        sizeof(CHANNEL_INFORMATION),
                        &tInfo);

    if (lRet != CIFX_NO_ERROR) {
        printf("[ERROR] Failed to get channel info: 0x%08X\n", (unsigned int)lRet);
        return;
    }

    // Print status banner
    printf("\n");
    printf("=============================================\n");
    printf("  DeviceNet Stack Status\n");
    printf("=============================================\n");

    // Channel READY status
    printf("Channel READY:     %s\n",
        (tInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY) ? "[OK] YES" : "[FAIL] NO");

    // BUS ON status
    printf("BUS ON:            %s\n",
        (tInfo.ulDeviceCOSFlags & HIL_COMM_COS_BUS_ON) ? "[OK] YES" : "[FAIL] NO");

    // Communication state
    printf("Comm State:        %s\n",
        (tInfo.ulDeviceCOSFlags & HIL_COMM_COS_RUN) ? "[OK] RUN" : "[WARN] STOP");

    // Node configuration
    printf("\n--- Configuration ---\n");
    printf("Node ID:           %d\n", g_tAppDnsData.bNodeIdValue);
    printf("Baud Rate:         ");
    switch (g_tAppDnsData.bBaudRateValue) {
        case 0: printf("125 kB/s\n"); break;
        case 1: printf("250 kB/s\n"); break;
        case 2: printf("500 kB/s\n"); break;
        default: printf("Unknown (%d)\n", g_tAppDnsData.bBaudRateValue); break;
    }

    // Device information
    printf("\n--- Device Info ---\n");
    printf("Device Number:     %u\n", (unsigned int)tInfo.ulDeviceNumber);
    printf("Serial Number:     %u\n", (unsigned int)tInfo.ulSerialNumber);
    printf("Firmware:          %u.%u.%u build %u\n",
        (unsigned int)tInfo.usFWMajor,
        (unsigned int)tInfo.usFWMinor,
        (unsigned int)tInfo.usFWRevision,
        (unsigned int)tInfo.usFWBuild);

    // Error information
    printf("\n--- Error Status ---\n");
    if (tInfo.ulDeviceCOSFlags & HIL_COMM_COS_CONFIG_LOCKED) {
        printf("Config Locked:     [WARN] YES (cannot reconfigure)\n");
    } else {
        printf("Config Locked:     [OK] NO\n");
    }

    printf("=============================================\n\n");

    // Explicit message readiness check
    if ((tInfo.ulDeviceCOSFlags & HIL_COMM_COS_READY) &&
        (tInfo.ulDeviceCOSFlags & HIL_COMM_COS_BUS_ON)) {
        printf("✅ Stack is READY for Explicit Messaging\n\n");
    } else {
        printf("❌ Stack is NOT READY for Explicit Messaging\n");
        printf("   Please check Channel READY and BUS ON status\n\n");
    }
}

/**************************************************************************************
 * Example: How to integrate into main.c
 **************************************************************************************/
#if 0
int main(void)
{
    // ... (existing initialization code) ...

    /* Initialize DeviceNet Stack */
    AppDNS_ConfigureStack(&g_tAppData, 0);

    /* Wait for Channel READY */
    App_AllChannels_GetChannelInfo_WaitReady(&g_tAppData);

    /* Print stack status for debugging */
    PrintDeviceNetStatus(&g_tAppData);  // <-- ADD THIS LINE

    /* Main loop */
    while (g_tAppData.fRunning)
    {
        App_IODataHandler(&g_tAppData);
        App_AllChannels_PacketHandler(&g_tAppData);
        OS_Sleep(1);
    }

    return 0;
}
#endif

/**************************************************************************************
 * Enhanced Packet Handler with Debug Output
 *
 * Replace AppDNS_PacketHandler_callback() with this version to see all received packets
 **************************************************************************************/
#if 0
bool AppDNS_PacketHandler_callback_DEBUG(CIFX_PACKET* ptPacket, void* pvUserData)
{
    bool fPacketHandled = true;
    APP_DATA_T* ptAppData = (APP_DATA_T*)pvUserData;

    /* Debug: Print received packet command */
    printf("[DEBUG] Packet Received: Cmd=0x%08X, Len=%u, State=0x%08X\n",
        (unsigned int)ptPacket->tHeader.ulCmd,
        (unsigned int)ptPacket->tHeader.ulLen,
        (unsigned int)ptPacket->tHeader.ulState);

    /* Check packet buffer validity */
    if (ptPacket != &ptAppData->aptChannels[DNS_DEMO_CHANNEL_INDEX]->tPacket)
    {
        printf("[ERROR] Unexpected packet resource received!\n");
        return false;
    }

    switch (ptPacket->tHeader.ulCmd)
    {
        case DNS_CMD_CIP_SERVICE_IND:
            printf("[INFO] ✅ CIP Service Indication Received!\n");
            AppDNS_HandleCipServiceIndication(ptAppData);
            fPacketHandled = true;
            break;

        default:
        {
            if ((ptPacket->tHeader.ulCmd & 0x1) == 0)
            {
                printf("[WARN] Disregarded indication packet: 0x%08X\n",
                    (unsigned int)ptPacket->tHeader.ulCmd);

                ptPacket->tHeader.ulState = ERR_HIL_NO_APPLICATION_REGISTERED;
                ptPacket->tHeader.ulCmd |= 0x01;
                AppDNS_GlobalPacket_Send(ptAppData);
            }
            else
            {
                printf("[WARN] Disregarded confirmation packet: 0x%08X\n",
                    (unsigned int)ptPacket->tHeader.ulCmd);
            }
        }
        break;
    }

    return fPacketHandled;
}
#endif

/**************************************************************************************
 * Test Procedure
 *
 * 1. Add PrintDeviceNetStatus() to main.c after stack initialization
 * 2. Compile and flash to STM32
 * 3. Connect UART terminal (115200 baud)
 * 4. Power on device and observe output
 *
 * Expected Output (Normal):
 * =============================================
 *   DeviceNet Stack Status
 * =============================================
 * Channel READY:     [OK] YES
 * BUS ON:            [OK] YES
 * Comm State:        [OK] RUN
 * Configured:        [OK] YES
 *
 * --- Configuration ---
 * Node ID:           1
 * Baud Rate:         125 kB/s
 *
 * --- Device Info ---
 * Device Number:     0
 * Serial Number:     1234567
 * Firmware:          5.3.0 build 1
 *
 * --- Error Status ---
 * Application Error: [OK] None
 * Config Locked:     [OK] NO
 * =============================================
 *
 * ✅ Stack is READY for Explicit Messaging
 *
 *
 * 5. From netHOST, send Identity Object Get Attribute request:
 *    0E 03 20 01 24 01 30 01
 *
 * Expected Output (if successful):
 * [DEBUG] Packet Received: Cmd=0x0000B104, Len=32, State=0x00000000
 * [INFO] ✅ CIP Service Indication Received!
 *
 * === CIP Service Indication ===
 * Service:   0x0E (Get Attribute Single)
 * Class:     0x01
 * Instance:  0x01
 * Attribute: 0x01
 * Data Len:  0
 *   -> Get: Success, Data=94 01
 * Response:  Cmd=0x0000B105, Len=32, Error=0x00
 * ==============================
 *
 *
 * If you see NO output after sending request:
 * - netX Stack rejected the request
 * - Check netHOST packet structure
 * - Try Identity Object request (Class 0x01)
 *
 **************************************************************************************/
