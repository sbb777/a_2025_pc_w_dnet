#ifndef APP_VAT_IOHANDLER_H_
#define APP_VAT_IOHANDLER_H_

#include "App_VAT_Assemblies.h"
#include <stdint.h>

/******************************************************************************
 * I/O ASSEMBLY UPDATE FUNCTIONS
 ******************************************************************************/

/* Input Assembly Update Functions (Slave → Master) */
void VAT_UpdateInputAssembly(ASSEMBLY_MANAGER_T* ptManager, uint8_t instance);

/* Output Assembly Process Functions (Master → Slave) */
void VAT_ProcessOutputAssembly(ASSEMBLY_MANAGER_T* ptManager, uint8_t instance);

/* Specific Assembly Handlers */
void VAT_UpdateInputAssembly100(INPUT_ASSEMBLY_100_T* ptInput);
void VAT_UpdateInputAssembly101(INPUT_ASSEMBLY_101_T* ptInput);
void VAT_UpdateInputAssembly105(INPUT_ASSEMBLY_105_T* ptInput);

void VAT_ProcessOutputAssembly8(OUTPUT_ASSEMBLY_8_T* ptOutput);
void VAT_ProcessOutputAssembly24(OUTPUT_ASSEMBLY_24_T* ptOutput);
void VAT_ProcessOutputAssembly102(OUTPUT_ASSEMBLY_102_T* ptOutput);

/* I/O Connection Type Handlers */
void VAT_HandleCyclicIo(ASSEMBLY_MANAGER_T* ptManager);
void VAT_HandlePollIo(ASSEMBLY_MANAGER_T* ptManager);
void VAT_HandleCosIo(ASSEMBLY_MANAGER_T* ptManager);

#endif /* APP_VAT_IOHANDLER_H_ */
