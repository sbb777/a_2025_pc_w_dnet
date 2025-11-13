/**************************************************************************************
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
***************************************************************************************
$Id: DNS_Public.h $:

Description:
This header includes all required header which contribute to the API of the DeviceNet
Slave Stack.

These definitions contribute to the LFW API of the DeviceNet Slave stack and are
applicable to the DPM packet interface.

**************************************************************************************/

#ifndef DNS_PUBLIC_H_
#define DNS_PUBLIC_H_

/* LFW Packet API: Compiler-specific macro definitions */
#include "Hil_Compiler.h"

/* LFW Packet API: Hilscher common packet definitions */
#include "Hil_Packet.h"

/* LFW Packet API: Hilscher common status codes */
#include "Hil_Results.h"

/* LFW Packet API: Hilscher common command codes and packets */
#include "Hil_ApplicationCmd.h"

/* LFW Packet API: Hilscher component identifiers */
#include "Hil_ComponentID.h"

/* LFW Packet API: Common definitions of CIP objects */
#include "CIP_common_definitions.h"

/* LFW Packet API: Hilscher specific CIP objects */
#include "CIP_hilscher_definitions.h"

/* LFW Packet API: Packet command definition */
#include "DNS_packet_commands.h"

/* LFW Packet API: Packet definition */
#include "DNS_packet_set_configuration.h"
#include "DNS_packet_cip_service.h"
#include "DNS_packet_create_assembly.h"
#include "DNS_packet_register_class.h"
#include "DNS_packet_reset.h"


/* LFW Packet API: Common DeviceNet Result code definition*/
#include "DeviceNet_Results.h"



/***************************************************************************************/
#endif
