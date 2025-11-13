/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
$Id:  $:

Description:
  Generic AP result code definitions
**************************************************************************************/

#ifndef GAP_RESULTS_H_
#define GAP_RESULTS_H_

/////////////////////////////////////////////////////////////////////////////////////
// Generic AP result codes
/////////////////////////////////////////////////////////////////////////////////////
//
// MessageId: ERR_GAP_INVALID_COMPONENT_ID
//
// MessageText:
//
// Invalid component identifier.
//
#define ERR_GAP_INVALID_COMPONENT_ID     ((uint32_t)0xC1090001L)

//
// MessageId: ERR_GAP_SET_REMANENT_DATA_NOT_ALLOWED
//
// MessageText:
//
// It is not allowed to set remanent data.
//
#define ERR_GAP_SET_REMANENT_DATA_NOT_ALLOWED ((uint32_t)0xC1090002L)

//
// MessageId: ERR_GAP_INVALID_WORKER
//
// MessageText:
//
// Invalid worker.
//
#define ERR_GAP_INVALID_WORKER           ((uint32_t)0xC1090003L)

//
// MessageId: ERR_GAP_LOGBOOK_CREATE_FAIL
//
// MessageText:
//
// The Logbook could not be created.
//
#define ERR_GAP_LOGBOOK_CREATE_FAIL      ((uint32_t)0xC1090004L)

//
// MessageId: ERR_GAP_POOL_CREATE_FAIL
//
// MessageText:
//
// The service pool could not be created.
//
#define ERR_GAP_POOL_CREATE_FAIL         ((uint32_t)0xC1090005L)

//
// MessageId: ERR_GAP_QUEUE_CREATE_FAIL
//
// MessageText:
//
// The GAP queue could not be created.
//
#define ERR_GAP_QUEUE_CREATE_FAIL        ((uint32_t)0xC1090006L)

//
// MessageId: ERR_GAP_MUTEX_INIT_FAIL
//
// MessageText:
//
// The initialization of a mutex failed.
//
#define ERR_GAP_MUTEX_INIT_FAIL          ((uint32_t)0xC1090007L)

//
// MessageId: ERR_GAP_TIMER_INIT_FAIL
//
// MessageText:
//
// The initialization of a PS Timer failed.
//
#define ERR_GAP_TIMER_INIT_FAIL          ((uint32_t)0xC1090008L)

//
// MessageId: ERR_GAP_COMPMGR_REGISTER_FAIL
//
// MessageText:
//
// Generic AP could not register at Component Manager.
//
#define ERR_GAP_COMPMGR_REGISTER_FAIL    ((uint32_t)0xC1090009L)

//
// MessageId: ERR_GAP_COMPMGR_DIAG_REGISTER_FAIL
//
// MessageText:
//
// Generic AP could not register the diagnostic callback at Component Manager.
//
#define ERR_GAP_COMPMGR_DIAG_REGISTER_FAIL ((uint32_t)0xC109000AL)

//
// MessageId: ERR_GAP_WATCHDOG_INIT_FAIL
//
// MessageText:
//
// The DPM watchdog initialization failed.
//
#define ERR_GAP_WATCHDOG_INIT_FAIL       ((uint32_t)0xC109000BL)

//
// MessageId: ERR_GAP_SET_CHANNEL_QUEUE_FAIL
//
// MessageText:
//
// The DPM channel queue could not be registered.
//
#define ERR_GAP_SET_CHANNEL_QUEUE_FAIL   ((uint32_t)0xC109000CL)

//
// MessageId: ERR_GAP_GET_CHANNEL_QUEUE_FAIL
//
// MessageText:
//
// The DPM channel queue could not be retrieved.
//
#define ERR_GAP_GET_CHANNEL_QUEUE_FAIL   ((uint32_t)0xC109000DL)

//
// MessageId: ERR_GAP_REMANENT_FILE_CREATE_FAIL
//
// MessageText:
//
// The remanent data file could not be created.
//
#define ERR_GAP_REMANENT_FILE_CREATE_FAIL ((uint32_t)0xC109000EL)

//
// MessageId: ERR_GAP_COMPONENT_INIT_FAIL
//
// MessageText:
//
// The component initialization failed.
//
#define ERR_GAP_COMPONENT_INIT_FAIL      ((uint32_t)0xC109000FL)

//
// MessageId: ERR_GAP_COMPONENT_START_FAIL
//
// MessageText:
//
// The component could not be started.
//
#define ERR_GAP_COMPONENT_START_FAIL     ((uint32_t)0xC1090010L)

//
// MessageId: ERR_GAP_REMANENT_DATA_NOT_ENOUGH_MEMORY
//
// MessageText:
//
// Not enough memory for remanent data.
//
#define ERR_GAP_REMANENT_DATA_NOT_ENOUGH_MEMORY ((uint32_t)0xC1090011L)

//
// MessageId: ERR_GAP_FRAGMENT_FORWARD_INIT_FAIL
//
// MessageText:
//
// The fragment forwarding initialization failed.
//
#define ERR_GAP_FRAGMENT_FORWARD_INIT_FAIL ((uint32_t)0xC1090012L)

//
// MessageId: ERR_GAP_DPM_CHANNEL_START_FAIL
//
// MessageText:
//
// The DPM channel could not be started.
//
#define ERR_GAP_DPM_CHANNEL_START_FAIL   ((uint32_t)0xC1090013L)

//
// MessageId: ERR_GAP_RESOURCE_ALLOCATION_FAIL
//
// MessageText:
//
// Failed to allocate Generic AP resources.
//
#define ERR_GAP_RESOURCE_ALLOCATION_FAIL ((uint32_t)0xC1090014L)

//
// MessageId: ERR_GAP_CONFIG_VERSION_INVALID
//
// MessageText:
//
// Invalid Generic AP configuration version.
//
#define ERR_GAP_CONFIG_VERSION_INVALID   ((uint32_t)0xC1090015L)

//
// MessageId: ERR_GAP_CONFIG_DPM_COMM_CHANNEL_INVALID
//
// MessageText:
//
// Invalid DPM communication channel.
//
#define ERR_GAP_CONFIG_DPM_COMM_CHANNEL_INVALID ((uint32_t)0xC1090016L)

//
// MessageId: ERR_GAP_CONFIG_DPM_CHANNEL_FW_INFO_INVALID
//
// MessageText:
//
// Invalid firmware information for the related DPM communication channel.
//
#define ERR_GAP_CONFIG_DPM_CHANNEL_FW_INFO_INVALID ((uint32_t)0xC1090017L)

//
// MessageId: ERR_GAP_REMANENT_FILE_READ_FAIL
//
// MessageText:
//
// The remanent data file could not be read.
//
#define ERR_GAP_REMANENT_FILE_READ_FAIL  ((uint32_t)0xC1090018L)

//
// MessageId: ERR_GAP_REMANENT_FILE_WRITE_FAIL
//
// MessageText:
//
// The remanent data file could not be written.
//
#define ERR_GAP_REMANENT_FILE_WRITE_FAIL ((uint32_t)0xC1090019L)

//
// MessageId: ERR_GAP_REMANENT_DATA_DELETE_FAIL
//
// MessageText:
//
// The remanent data could not be deleted.
//
#define ERR_GAP_REMANENT_DATA_DELETE_FAIL ((uint32_t)0xC109001AL)

//
// MessageId: WARN_GAP_SET_REMANENT_DATA_DENIED
//
// MessageText:
//
// The component did not accept the remanent data.
//
#define WARN_GAP_SET_REMANENT_DATA_DENIED ((uint32_t)0x8109001BL)

//
// MessageId: WARN_GAP_SERVICE_DENIED
//
// MessageText:
//
// The component did not accept the service.
//
#define WARN_GAP_SERVICE_DENIED          ((uint32_t)0x8109001CL)

//
// MessageId: ERR_GAP_NO_RESPONSE_HANDLER
//
// MessageText:
//
// No response service handler could be found.
//
#define ERR_GAP_NO_RESPONSE_HANDLER      ((uint32_t)0xC109001DL)

//
// MessageId: WARN_GAP_SERVICE_RETRY
//
// MessageText:
//
// The forwarding of a service to the component or application has failed and will be repeated.
//
#define WARN_GAP_SERVICE_RETRY           ((uint32_t)0x8109001EL)

//
// MessageId: ERR_GAP_QUEUE_INVALID_SERVICE_LEN
//
// MessageText:
//
// Queue service length is invalid.
//
#define ERR_GAP_QUEUE_INVALID_SERVICE_LEN ((uint32_t)0xC109001FL)

//
// MessageId: ERR_GAP_DPM_INVALID_SERVICE_LEN
//
// MessageText:
//
// DPM service length is invalid.
//
#define ERR_GAP_DPM_INVALID_SERVICE_LEN  ((uint32_t)0xC1090020L)

//
// MessageId: WARN_GAP_NO_REMANENT_DATA
//
// MessageText:
//
// Remanent data does not exist.
//
#define WARN_GAP_NO_REMANENT_DATA        ((uint32_t)0x81090021L)

//
// MessageId: ERR_GAP_QUEUE_JOB
//
// MessageText:
//
// Service job queued failed.
//
#define ERR_GAP_QUEUE_JOB                ((uint32_t)0xC1090022L)

#endif  /* GAP_RESULTS_H_ */
