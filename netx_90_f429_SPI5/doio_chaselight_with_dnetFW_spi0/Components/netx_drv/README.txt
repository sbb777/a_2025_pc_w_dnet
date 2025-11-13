/**************************************************************************************//*
$Id: README.txt 11618 2024-10-07 13:01:36Z dmilkov $: 
**************************************************************************************//**
\copyright Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
\if netx_drv_mainpage
  \mainpage
\else
  \page netx_drv netX Peripherals Driver
\endif

Introduction
------------

This is the driver library, designated for the netX microcontroller application side.
This page will provide an overview of the driver and explain how it is structured and 
how it is used. The target IDE of this introduction and the driver itself is netX Studio.

The CMSIS component is also used and explained here. In future versions both components
will be merged together. 

\tableofcontents

Features and requirements of the driver
-----------------------
The target of the driver layer is to provide a simple API for the applications to 
interact with the devices available. We have two kinds of drivers. The first one are the
ones we have developed after the CMSIS recommendations. The other ones are developed for
verification purposes and provide a hardware near abstaction layer. To support the family of 
chips a so called chip support layer was introduced, where the drivers functionality is
mapped to requirement definitons and of course the hardware registers.  

This meta layer shall met the following requirements:
- The API shall be user friendly.
- There shall be no dynamic memory allocation.
- The API shall have a consistent look and feel.
- The driver should be performant but shall also be maintainable.
- For security reasons the drivers API shall only return elements of the state
    enumeration.
- The CMSIS standard has to be respected and be a general guideline.
  - Changes on it shall be minimal.
  - A pack shall be available in the future and contain the flasher.
  - The svd and all other structures shall be supported and used by the driver.
- The driver uses the object oriented c as a guideline.
  - The first parameter shall be the associated class/handle/context object.
  - If the context object is global and constant it should not be necessary to
      be the first parameter.
  - The definition of functions as function pointers and object attributes is
      optional.
  - There should not be the need for inheritance.
- The driver shall support operating systems.
- The functions shall be reentrant.
- The driver shall support concurrent acesses.
  - A locking mechanisms shall be implemented and used therefore
- The driver shall contain debug tracing functionality
- The data and control flow shall be separated as good as possible.
  - A flusing function shall be introduced to cope with the data flow.
  - There shall be IRQ, DMAC and Polling programing models be implemented
      for each device driver.
- The driver is configured via the configuration object. It may not be 
    modified except by driver.
- The drivers context object or handle shall not be modified exept by
    driver.
- A driver is referenced to an other driver by its handle.
- Blocking calls shall implement a timeout.
- Blocking calls shall provide a sleep and rescheduling option.
- There shall be callbacks available.
- DeInit shall deactivate the peripheral module from an application point of view. 
    (e.g. disable interrupts, stop DMA, etc.) It is not required to reset 
    HW registers to reset state.

The following devices are supported with full drivers:
- \b DIO - Digital Input and Output devices
  - \b MMIO - Multiplex Matrix for Input and Output
  - \b HIF - Host InterFace
  - \b PIO - Programmed Input and Output device
  - \b GPIO - General Purpose Input and Output device
  - \b BOD - Brown Out Detection
- \b MLED - Multiple Light Emitting Diode device
- \b Timer - Timer device
  - \b GPIO \b PWM - General Purpose Input and Output device for Pulse Width Modulated signals
  - \b GPIO \b Blink - General Purpose Input and Output device for Blinking modes
  - \b Timer \b (ARM) - Timer devices similar to ARM
  - \b System \b Tick - Timer device dedicated to generate a system tick
  - \b Systime - High precision clock devices with seconds and nanoseconds registers
  - \b Systime \b Latch - Device for latching the available Systimes at the same moment
  - \b Systime \b Compare - Device generating interrupt if a Systime value is reached
- \b ADC - Analog Digital Converter peripheral
  - \b Temperature \b sensor - The temperature sensor readable by the ADC
- \b UART - Universal Asynchronous Receiver Transmitter
- \b SPI - Serial Peripheral Interface
- \b SQI - Serial Quad mode Interface
- \b I2C - Inter-Integrated Circuit
- \b DMAC - Direcet Memory Access Controller
- \b TRACE - A software module with a ring buffer for fast tracing.

The following devices are supported by legacy drivers:
- \b BiSS - Bidirectional Serial Synchron interface
- \b CAN - Controlle Area Network
- \b XPIC \b Ethernet \b MAC - Driver for the ethernet mac that runs on the xPIC
- \b XPIC \b Loader - Driver that enables the xPIC device with load, execute and debug
    functionalities.

Folder structure
----------------
- \b CMSIS - The Cortex Microcontroller Software Interface Standard 
  - \b Device - The devices supported
    - \b Hilscher - The vendor contained in this pack
      - \b netx - The chip series
        - \b Include - Include files of this chip series
          - \b regdef - The register definitions by legacy impementation
            - \b netx90_app - The final silicon of netX 90
              - regdef_netx90_arm_app.h - The c header of the netx90 register definition
              - regdef_netx90_arm_app.S - The asm header of the netx90 register definition
              - regdef_netx90_arm_app.html - The register definition as html site
              - regdef_netx90_arm_app.svd - The register definition as unedited svn
            - \b netx90_mpw_app - The engineering samples of netX 90
              - regdef_netx90_MPW_arm_app.h - The c header of the netx90 register definition
              - regdef_netx90_MPW_arm_app.S - The asm header of the netx90 register definition
              - regdef_netx90_MPW_arm_app.html - The register definition as html site
              - regdef_netx90_MPW_arm_app.svd - The register definition as unedited svn
            - netx90_app.h - The header file of the final silicon of netX90
            - netx90_mpw_app.h - The header file of the engineering samples of netX90
            - system_netx.h - The header file of the netx system wide initialization code
            - netx90_app.svd - The svd of the final silicon of netX90
            - netx90_mpw_app.svd - The svd of the engineering samples of netX90
        - \b Source - Startup and init source files provided with the library
          - \b ARM - Files for the ARM compiler
            - startup_netx90_app.s - netX90 final silicon startup code
            - startup_netx90_mpw_app.s - netX90 engineering sample startup code
          - \b GDB - Files for the gcc compiler
            - startup_netx90_app.S - netX90 final silicon startup code
            - startup_netx90_mpw_app.S - netX90 engineering sample startup code
            - gcc_netx90_app.ld - netX90 final silicon linker file
            - gcc_netx90_mpw_app.ld - netX90 engineering sample linker file
          - \b IAR - Files for the IAR compiler
            - startup_netx90_app.s - netX90 final silicon startup code
            - startup_netx90_mpw_app.s - netX90 engineering sample startup code
          - system_netx.c - Init functionalities of the netx controllers
  - \b Include - CMSIS-Core headers for the netx technologie.
    - cmsis_armcc.h
    - cmsis_armclang.h
    - cmsis_compiler.h
    - cmsis_gcc.h
    - cmsis_iccarm.h
    - cmsis_version.h
    - core_cm4.h
    - mpu_armv7.h
- \b netx_drv - The driver component itself
  - \b Include - The header files of the driver
    - \b csp - The chip support package containing the chip specifics
      - netx_drv_csp_netx90_app.h - The csp for the final silicon of netX90
      - netx_drv_csp_netx90_mpw_app.h - The csp for the engineerning sample of netX90
    - netx_drv_adc.h - Header of the ADC device
    - netx_drv_biss.h - Header of the ADC device
    - netx_drv_canctrl.h - Header of the ADC device
    - netx_drv_conf.h - Header containing the driver configuration
    - netx_drv_cortex.h - Header for wrapping the cortex core calls
    - netx_drv_def.h - Header with general definitions
    - netx_drv_dio.h - Header of the DIO devices
    - netx_drv_dmac.h - Header of the DMAC device
    - netx_drv_eth_xpic_def.h - Header of the ethernet xpic mac defines
    - netx_drv_eth_xpic_ram.h - Header of the ethernet xpic mac shared memory
    - netx_drv_eth_xpic.h - Header of the ethernet xpic mac itself
    - netx_drv_i2c.h - Header of the i2c devices
    - netx_drv_mled.h - Header of the mled devices
    - netx_drv_spi.h - Header of the spi and sqi devices
    - netx_drv_tim.h - Header of the timer devices
    - netx_drv_uart.h - Header of the uart device
    - netx_drv_xpic.h - Header of the xpic controller
    - netx_drv.h - Header file to be included first
  - \b Source - The source files of the driver
    - netx_drv_adc.c - Source file of the ADC device
    - netx_drv_biss.c - Source file of the BISS device
    - netx_drv_canctrl.c - Source file of the CAN device
    - netx_drv_cortex.c - Source file of the cortex core calls
    - netx_drv_dio.c - Source file of the DIO device
    - netx_drv_dmac.c - Source file of the DMAC device
    - netx_drv_eth_xpic.c - Source file of the ethernet xpic mac device
    - netx_drv_i2c.c - Source file of the i2c device
    - netx_drv_mled.c - Source file of the mled device
    - netx_drv_spi.c - Source file of the spi and sqi devices
    - netx_drv_tim.c - Source file of the timer devices
    - netx_drv_uart.c - Source file of the UART device
    - netx_drv_xpic.c - Source file of the xpic controller
    - netx_drv.c - Source file of the driver in general
  - netx_drv_user_conf_template.h - Header file to be copied, renamed 
      (without _template) and modified. Contains the driver configuration
  - \ref disclaimer_netx_drv "README_DISCLAIMER.txt" - The library's disclaimer
  - \ref netx_drv "README.txt" - This readme file

Structure of the driver
-----------------------
To illustrate the structure of the driver an example device driver "DEV" is discussed. The naming 
convention and other ones will be discussed in the next chapter called "Conventions".

### Operation modes
Most of our peripherals are communication devices or deliver an data stream of some kind. Those streams
are in most concepts transmitting and receiving data in single-, half- or full-duplex. To increase the
maintainability the data and the control flows are split up as good as possible. To transport the data,
the dma is used or the flush routine for transmitting the data. All other functions have the job to
configure, initiate and control the flow of data. 

### Layers
In the layer model below one is able to see its over all structure. At the top there is an application 
and in the bottom, the hardware peripherals are shown.
<table>
<tr><th colspan="7">Application
<tr><td rowspan="5">CMSIS, OS abstaction<BR>and helper functions<td rowspan="5">DRV_DEV_GetState/<BR>DRV_DEV_Init<td rowspan="2" colspan="2">DRV_DEV_Abort<td>DRV_DEV_Tx<td>DRV_DEV_Rx<td>DRV_DEV_TxRx
<tr><td colspan="3" >CallByControlFlow
<tr><td rowspan="3">DMA<td rowspan="3">IRQ<td rowspan="2">Poll<td>IRQ<td>DMA
<tr><td>ISR<td rowspan="2">DMA ISR
<tr><td colspan="2">Flush
<tr><th colspan="7">Hardware Device/NVIC/DMAC
</table>
Between the application and the hardware are on the upper layer the API calls. Some are directly interacting
with the hardware like the initialization and some are not. The get state for example derives hardware states
and software states in one call. The init function for example talks also with the NVIC peripheral and the 
dmac to initialize the drivers context by its given configuration. The abort function is split between the
dma and irq mode but for polling it is not necessary, because in polling it is not possible to abort the
polling context. To centralize the control flow and to switch between data direction and operation method 
there is a special layer. That configures the data flow parameters and then calls the associated operation
method. This is done, because in most cases, the operation mode is independent of the data flow configured.

### Configuration file
The driver is configured by a configuration file. This file is called netx_drv_user_conf.h, which is derived
from its template called netx_drv_user_conf_template.h.

The configuration file may be used to include components of the driver into the build process or not, without
managing the inclusion inside waf and the sources. This may be performed my commenting out or in the enabled
macros.

It is also used for the os specific parts of the driver, where it is possible to exchange the api locking with the
os free mutexes against os specific ones.

Because the DMA is used by some other drivers it has to be included if those are used. The same will be necessary
for the SQI-Flash driver with the SQI driver. 

### OS abstraction layer for the driver
The os abstraction layer currently exists solely by the locking mechanism of the API. If a context is already in
use, the locking mechanism shall prevent two callers from using the same context. Two callers for example might
be on the netX90 two tasks of an operating system or the normal execution context (user mode) and the interrupt
context (privileged mode).

### Errors, states and assertion

### Class diagram of example device "DEV"
In the diagram below, one is able to take a look on the general structure of drivers.
The first attribute of all function calls shall contain a pointer to the structure
containing the execution context. The only exception of this are the DIO driver, that
has a global context objet and the legacy hardware abstraction layer drivers.
\dot
digraph example {
  edge [fontname="Helvetica",fontsize="9",labelfontname="Helvetica",labelfontsize="9"];
  node [fontname="Helvetica",fontsize="9",shape=record];
  Node1 [label="{DRV_DEV_HANDLE_T\n|+ Buffer\l+ BufferSize\l+ BufferCounter\l+ tLock\land many more...\l|+ DRV_DEV_Init()\l+ DRV_DEV_DeInit()\l+ DRV_DEV_Transmit()\l+ DRV_DEV_Receive()\l+ DRV_DEV_TransmitReceive()\l+ DRV_DEV_
()\l+ DRV_DEV_Abort()\l+ DRV_DEV_GetState()\land many more...\l}",height=0.2,width=0.4,color="black", fillcolor="grey75", style="filled", fontcolor="black",tooltip="The device handle/class/context object."];
  Node2 -> Node1 [color="grey25",fontsize="9",style="solid",label=" +tConfiguration" ,arrowhead="odiamond",fontname="Helvetica"];
  Node2 [label="{DRV_DEV_CONFIGURATION_T\n|+ eDeviceID\l+ eOperationMode\l+ eFrequency\l+ eFormat\l+ eDuplex\land many more...\l|}",height=0.2,width=0.4,color="black",tooltip="The configuration structure of the device driver. "];
  Node3 -> Node2 [color="grey25",fontsize="9",style="solid",label=" +uAttribute" ,arrowhead="odiamond",fontname="Helvetica"];
  Node3 [label="{DRV_DEV_ATTRIBUTE_T\n|+ value\l+ field\l|}",height=0.2,width=0.4,color="red",tooltip="Typedefs for structured attributes. "];
  Node4 -> Node2 [color="grey25",fontsize="9",style="solid",label=" +ptSubDeviceTx\n+ptSubDeviceRx" ,arrowhead="odiamond",fontname="Helvetica"];
  Node4 [label="{DRV_SUBDEV_HANDLE_T\n|+ atList\l+ tLock\land many more...\l|+ DRV_SUBDEV_Init()\l+ DRV_SUBDEV_DeInit()\l+ DRV_SUBDEV_Start()\l+ DRV_SUBDEV_Abort()\land many more...\l}",height=0.2,width=0.4,color="red",tooltip="The handle of the sub driver. "];
  Node6 -> Node1 [color="grey25",fontsize="9",style="solid",label=" +ptDEV" ,arrowhead="odiamond",fontname="Helvetica"];
  Node6 [label="{dev_Type\n|+ dev_cr\l+ datasize\l+ reserved0\l+ polarity\l+ phase\l+ multiplyadd\l+ reserved1\l+ cfg\land many more...\l|}",height=0.2,width=0.4,color="black",tooltip="The device registers."];
}
\enddot
The handle structure associated with the device consists out of the configuration,
other attributes and a set of functions. The attributes shall not be altered, except
the configuration, which shall be provided before (re)initializing the driver. So it
is possible to change configuration parameters afterwards, but they have to be
followed by an initialization. This has a lot of side effects and should only be 
performed if really necessary. The change of operation mode by reinitilization is
particularly not recommended at the moment, because of its huge test vector.

In case that a subdevice is used, such as the dma controller, one has to provide
a context handle.

### Usage of the API
The driver has the legacy HALs that are not part of this structure, also the dio driver
is a bit special. The DIO has a global handle, that is not visible to the user. This design
choice was made, because the devices have no interleaved configuration and no higher context.
Each channel (pin) has a well defined state so that it is not necessary to provide context
sensitive informations between interactions.

The normal API however has some core functions. As already used in the class diagram above,
we define a device called DEV by functionalities as init, deinit, transmit, receive, change, 
abort and at last get state. All methods return the state of the drivers api. In c peudo
code, such a device interaction is shown. The example is some kind of spi device, where the
chip or frame select line is set before and reset after a transmission.

\code{.c}
typedef DRV_SPI_HANDLE_T;
typedef DRV_STATUS_E;
typedef DRV_SPI_FSS_E;

DRV_STATUS_E DRV_SPI_Init(DRV_SPI_HANDLE_T * const ptHandle);
DRV_STATUS_E DRV_SPI_DeInit(DRV_SPI_HANDLE_T * const ptHandle);
DRV_STATUS_E DRV_SPI_Transmit(DRV_SPI_HANDLE_T * const ptHandle, uint8_t* pcData, size_t size);
DRV_STATUS_E DRV_SPI_Receive(DRV_SPI_HANDLE_T * const ptHandle, uint8_t* pcData, size_t size);
DRV_STATUS_E DRV_SPI_ChangeFss(DRV_SPI_HANDLE_T * const ptHandle, DRV_SPI_FSS_E eFSS);
DRV_STATUS_E DRV_SPI_GetState(DRV_SPI_HANDLE_T * const ptHandle);

DRV_SPI_HANDLE_T ptHandle;
\endcode

### Buffers: data and control flows
The data and the control instructions are split up into several contexts. While the data and the
control instructions originate from the same source, they should be split up as early as possible
and executed by different (meta) contexts. Those conexts and the execution Sequence of the 

### Polling mode
The polling mode of an driver is intended to give the user a mode where the device is acessed in 
high priority by the core to exchange data. This means, that the caller context is blocked and its
primary task is to flush the buffers. In the sequence diagram below one can see how this behaviour
is achieved.
\startuml
skinparam sequence {
    ArrowColor #000050
    ActorBorderColor #000050
    LifeLineBorderColor #000050
    
    ParticipantBorderColor #000050
    ParticipantBackgroundColor Azure
    ParticipantFontColor #000050
    
    ActorFontColor #000050
    ActorBackgroundColor Azure
}
skinparam backgroundColor transparent
skinparam roundcorner 5

participant Handle
actor Caller
participant API
participant PollingMode
participant Flush
participant Callbacks
participant Device
participant Data

create Handle
Caller -> Handle: Create
Caller -> Handle: Configure
Caller -> API: Initialize
activate API #YellowGreen
group Parameter Issue
API X-> Caller: DRV_ERROR_PARAM
end
API -> Handle: Initialize
activate Handle #DarkKhaki
API -> Device: Initialize
activate Device #DarkKhaki
API --> Handle: Unlock
API -> Caller: Return
deactivate API
Caller -> Caller: Do other things
activate Caller #cccccc
deactivate Caller
Caller -> API: Tx|Rx|(TxRx)
activate API #YellowGreen
API --> Handle: Trylock
group Failed to lock
API X-> Caller: DRV_LOCKED
end
group Parameter Issue
API X-> Caller: DRV_ERROR_PARAM
end
API -> Handle: Check for Busy and Errors
API -> Device: Check for Busy and Errors
API --> Handle: Configure
API -> Device: Configure
API -> PollingMode: Call
activate PollingMode #MediumAquaMarine
PollingMode --> Handle: << Uses >>
loop Flush Buffers
PollingMode -> Flush: Call
activate Flush #DarkSeaGreen
Flush --> Handle: << Uses >>
alt Data to read
Flush <- Data: Read
Flush -> Device: Write
end
alt Data to write
Flush <- Device: Read
Flush -> Data: Write
end
Flush -> PollingMode: Return
deactivate Flush
end
PollingMode -> Callbacks: Complete
activate Callbacks #cccccc
Callbacks -> API: GetState
activate API #YellowGreen
API -> Handle: Check Buffer
API -> Device: Check Buffer and Busy Flag
API -> Callbacks: ReturnState
deactivate API
Callbacks -> Callbacks: CheckState
Callbacks -> PollingMode: Return
deactivate Callbacks
PollingMode --> Handle: Reset
PollingMode -> API: Return
deactivate PollingMode
API --> Handle: Unlock
API -> Caller: Return
deactivate API
\enduml

### Interrupt mode
This shall be an explatory text for the IRQ Mode.

\startuml
skinparam sequence {
    ArrowColor #000050
    ActorBorderColor #000050
    LifeLineBorderColor #000050
    
    ParticipantBorderColor #000050
    ParticipantBackgroundColor Azure
    ParticipantFontColor #000050
    
    ActorFontColor #000050
    ActorBackgroundColor Azure
}
skinparam backgroundColor transparent
skinparam roundcorner 5

participant Handle
actor Caller
participant API
participant IRQMode
participant ISR
participant Flush
participant Callbacks
participant Device
participant Data

create Handle
Caller -> Handle: Create
Caller -> Handle: Configure
Caller -> API: Initialize
activate API #YellowGreen
group Parameter Issue
API X-> Caller: DRV_ERROR_PARAM
end
API -> Handle: Initialize
activate Handle #DarkKhaki
API -> Device: Initialize
activate Device #DarkKhaki
API --> Handle: Unlock
API -> Caller: Return
deactivate API
Caller -> Caller: Do other things
activate Caller #cccccc
deactivate Caller
Caller -> API: Tx|Rx|(TxRx)
activate API #YellowGreen
API --> Handle: Trylock
group Failed to lock
API X-> Caller: DRV_LOCKED
end
group Parameter Issue
API X-> Caller: DRV_ERROR_PARAM
end
API -> Handle: Check for Busy and Errors
API -> Device: Check for Busy and Errors
API --> Handle: Configure
== Transaction Begin / Busy ==
API -> Device: Configure
API -> IRQMode: Call
activate IRQMode #MediumAquaMarine
IRQMode -> Device: MaskIRQ
note over Device
From now on interrupts might occurr
endnote
Device --> ISR: (Interrupt)
IRQMode -> API: Return
API --> Handle: Unlock
deactivate IRQMode
API -> Caller: Return
deactivate API
Caller -> Caller: Do other things
activate Caller #cccccc
deactivate Caller
loop Occasional check for State
Caller -> API: Check Busy State
activate API #YellowGreen
API -> Handle: Check Buffer
API -> Device: Check Buffer and Busy Flag
API -> Caller: Return Busy
deactivate API
end
Caller -> Caller: Do other things
activate Caller #cccccc
Device -> ISR: Interrupt 
activate ISR #PaleGreen
ISR --> Handle: << Uses >>
ISR -> Flush: Call
activate Flush #DarkSeaGreen
Flush --> Handle: << Uses >>
alt Data to read
Flush <- Data: Read
Flush -> Device: Write
end
alt Data to write
Flush <- Device: Read
Flush -> Data: Write
end
Flush -> ISR: Return
deactivate Flush
alt Transaction Completed
ISR -> Callbacks: Complete
activate Callbacks #cccccc
Callbacks -> API: Check for Errors 
activate API #YellowGreen
API -> Handle: Check Buffer
API -> Device: Check Buffer and Busy Flag
API -> Callbacks: Return 
deactivate API
Callbacks -> Callbacks: CheckState
Callbacks -> ISR: Return
deactivate Callbacks
end
ISR --> Handle: Reset
== Transaction End / Not Busy anymore ==
deactivate ISR
Caller -> Caller: Do other things
deactivate Caller
Caller -> API: Check Busy State
activate API #YellowGreen
API -> Handle: Check Buffer
API -> Device: Check Buffer and Busy Flag
API -> Caller: Return 
deactivate API
\enduml


### DMA mode
This shall be an explatory text for the DMA Mode.

\startuml
skinparam sequence {
    ArrowColor #000050
    ActorBorderColor #000050
    LifeLineBorderColor #000050
    
    ParticipantBorderColor #000050
    ParticipantBackgroundColor Azure
    ParticipantFontColor #000050
    
    ActorFontColor #000050
    ActorBackgroundColor Azure
}
skinparam backgroundColor transparent
skinparam roundcorner 5

participant DMAHandle
participant Handle
actor Caller
participant API
participant DMAMode
participant DMAAPI
participant DMAISR
participant Callbacks
participant DMADevice
participant Device
participant Data

create Handle
Caller -> Handle: Create
create DMAHandle
Caller -> DMAHandle: Create
Caller -> Handle: Configure
Caller -> API: Initialize
activate API #YellowGreen
group Parameter Issue
API X-> Caller: DRV_ERROR_PARAM
end
loop Foreach DMAC
API -> DMAHandle: Configure
API -> DMAAPI: Initialize
activate DMAAPI #YellowGreen
group Parameter Issue
DMAAPI X-> API: DRV_ERROR_PARAM
API X-> Caller: DRV_ERROR
end
DMAAPI -> DMAHandle: Initialize
activate DMAHandle #DarkKhaki
DMAAPI -> DMADevice: Initialize
activate DMADevice #DarkKhaki
DMAAPI --> DMAHandle: Unlock
DMAAPI -> API: Return
deactivate DMAAPI
end
group Error during DMA Init
API X-> Caller: DRV_ERROR
end
note right of API
All DMACs have to be configured. If only one has been,
an error is returned and a new init has to be performed.
endnote

API -> Handle: Initialize
activate Handle #DarkKhaki
API -> Device: Initialize
activate Device #DarkKhaki
API --> Handle: Unlock
API -> Caller: Return
deactivate API
Caller -> Caller: Do other things
activate Caller #cccccc
deactivate Caller
Caller -> API: Tx|Rx|(TxRx)
activate API #YellowGreen
API --> Handle: Trylock
group Failed to lock
API X-> Caller: DRV_LOCKED
end
group Parameter Issue
API X-> Caller: DRV_ERROR_PARAM
end
API -> Handle: Check for Busy and Errors
API -> Device: Check for Busy and Errors
API --> Handle: Configure
== Transaction Begin / Busy ==
API -> Device: Configure

loop Foreach DMAC
API -> DMAMode: Call
activate DMAMode #MediumAquaMarine
DMAMode -> DMAAPI: Start
activate DMAAPI #MediumAquaMarine
DMAAPI -> DMADevice: Configure
DMAAPI -> DMADevice: Enable
activate DMADevice #MediumAquaMarine
DMAAPI -> DMAMode: Return
deactivate DMAAPI
DMAMode -> API: Return
deactivate DMAMode
end
group Error during DMA Start
API X-> Caller: DRV_ERROR
end
note right of API
All DMACs have to be started at the same time. If only one was not,
the transaction has to be aborted and restarted again.
endnote
API --> Handle: Unlock
API -> Caller: Return
deactivate API

Caller -> Caller: Do other things
activate Caller #cccccc
deactivate Caller
loop Occasional check for State
Caller -> API: Check Busy State
activate API #YellowGreen
API -> Handle: Check Buffer
API -> Device: Check Buffer and Busy Flag
API -> Caller: Return Busy
deactivate API
end
Caller -> Caller: Do other things
activate Caller #cccccc

loop Data is exchanged in background
DMADevice <-> Device: Data IO
DMADevice <-> Data: Data IO
end

DMADevice -> DMAISR: End of IO
deactivate DMADevice #MediumAquaMarine
activate DMAISR #MediumAquaMarine
alt Transaction Completed
DMAISR -> Callbacks: Complete
activate Callbacks #cccccc
Callbacks -> API: Check for Errors 
activate API #YellowGreen
API -> Handle: Check Buffer
API -> Device: Check Buffer and Busy Flag
API -> Callbacks: Return 
deactivate API
Callbacks -> Callbacks: CheckState
Callbacks -> DMAISR: Return
deactivate Callbacks
end
DMAISR --> Handle: Reset
deactivate DMAISR 
== Transaction End / Not Busy anymore ==
deactivate DMAISR
Caller -> Caller: Do other things
deactivate Caller
Caller -> API: Check Busy State
activate API #YellowGreen
API -> Handle: Check Buffer
API -> Device: Check Buffer and Busy Flag
API -> Caller: Return 
deactivate API
\enduml

### Error Handling
The following code snipped is an example code on how to make use of the returned state. Not every DRV_STATE_E differing from DRV_OK is an error.
A good example might be the DRV_LOCKED state, which might occur because an interrupt interrupted the allocation of the locking mutex. If this
has happened one has to try again locking it. However, the API might also be locked and another task is using it. So it is feasible to log the
time or count how often this happens and decide at wich point this might be an error.

\code{.c}
int spifaulthandlingexample(int argc, char const * argv[])
{
  uint32_t ulDataTx = 0xaabbccddul;
  uint32_t ulDataRx = 0x11223344ul;
  DRV_SPI_HANDLE_T tSPI = { { 0 } };
  DRV_SPI_STATE_E eSPIState;
  tSPI.tConfiguration.eOperationMode = DRV_OPERATION_MODE_IRQ;
  tSPI.tConfiguration.eSPIDeviceID = DRV_SPI_DEVICE_ID_SPI0;
  tSPI.tConfiguration.eFrequency = DRV_SPI_FREQUENCY_1_56MHz;
  DRV_STATUS_E eReturn;
  if(DRV_OK != (eReturn = DRV_SPI_Init(&tSPI)))
  {
    //ERROR
    __BKPT(0);
    // A diagnostics function and fault handling shall be performed here
    return 0;
  }
  // Transmit and receive 4 Byte of data via the SPI in the specified IRQ operation mode
  while(DRV_OK != (eReturn = DRV_SPI_TransmitReceive(&tSPI, (uint8_t*) &ulDataTx, (uint8_t*) &ulDataRx, 4)))
  {
    switch (eReturn)
    {
    case DRV_LOCKED:
      // Either the API is locked or the trylock was interrupted.
      // A counter might be implemented to check if there is some kind of deadlock
      continue;
    case DRV_BUSY:
      // The device/driver is busy performing a task given.
      // A counter might be implemented to check if there is some kind of deadlock
      continue;
    case DRV_ERROR:
      /* A generic error has occurred. In case of DMA operation mode, this will happen if the
       * return values of both DMAC channels do not match. In this case it is possible to
       * recover by aborting pending transfers.
       */
      while(DRV_OK != (eReturn = DRV_SPI_Abort(&tSPI)))
      {
        switch (eReturn)
        {
        case DRV_LOCKED:
          // a counter might be implemented to check if there is some kind of deadlock
          continue;
        case DRV_BUSY:
          // a counter might be implemented to check if there is some kind of deadlock
          continue;
        case DRV_ERROR:
          // a counter might be implemented to check if there is some kind of deadlock
          __BKPT(0);
          // A diagnostics function and fault handling shall be performed here
          continue;
        default:
          // should not occur
          __BKPT(0);
          // A diagnostics function and fault handling shall be performed here
          continue;
        }
      }
      // a counter might be implemented to check if there is some kind of deadlock
      continue;
    case DRV_ERROR_BUFFER:
      // Some generic buffer has an issue
      __BKPT(0);
      // A diagnostics function and fault handling shall be performed here
      continue;
    case DRV_ERROR_PARAM:
      // A parameter given to the function is not valid. Most likely a null pointer
      __BKPT(0);
      // A diagnostics function and fault handling shall be performed here
      continue;
    case DRV_NIMPL:
    case DRV_NSUPP:
      // The function you are calling is not supported or not implemented
      // This depends also on the configuration settings. Not every functionality
      // with every configuration setting is possible or supported.
      __BKPT(0);
      // A diagnostics function and fault handling shall be performed here
      continue;
    case DRV_TOUT:
      // The function has reached the given timeout in polling mode.
      __BKPT(0);
      // A diagnostics function and fault handling shall be performed here
      continue;
    default:
      // should not occur
      __BKPT(0);
      // A diagnostics function and fault handling shall be performed here
      continue;
    }
  }
  // Wait until the transaction is finished
  while(DRV_OK != (eReturn = DRV_SPI_GetState(&tSPI, &eSPIState)))
  {
    switch (eReturn)
    {
    case DRV_BUSY:
      // The device/driver is busy performing a task given.
      // A counter might be implemented to check if there is some kind of deadlock
      continue;
    case DRV_ERROR_PARAM:
      // A parameter given to the function is not valid. Most likely a null pointer
      __BKPT(0);
      // A diagnostics function and fault handling shall be performed here
      continue;
    case DRV_ERROR_BUFFER:
      // Some buffer has an issue
      if((uint32_t) eSPIState & (uint32_t) DRV_SPI_STATE_RX_FIFO_OVERFLOW)
      {
        // Receive Buffer had an overflow
        // External master too fast or if one is the master, too much data was transmitted
      }
      if((uint32_t) eSPIState & (uint32_t) DRV_SPI_STATE_RX_FIFO_UNDERRUN)
      {
        // Receive Buffer had an underrun
        // Driver has read out too much data. Should not occur.
      }
      if((uint32_t) eSPIState & (uint32_t) DRV_SPI_STATE_TX_FIFO_OVERFLOW)
      {
        // Transmit buffer had an overflow
        // Driver has written too much data into the fifo. Should not occur.
      }
      if((uint32_t) eSPIState & (uint32_t) DRV_SPI_STATE_TX_FIFO_UNDERRUN)
      {
        // Transmit buffer had an underrun
        // Might happen as a slave if there is no data present while the master selects the slave,
        // or because the slave is too slow filling up the fifo.
        // In case one is the master, a underrun should not occur because the hardware would just
        // stop transmitting data.
      }
      if((uint32_t) eSPIState
        & ((uint32_t) DRV_SPI_STATE_RX_FIFO_OVERFLOW | (uint32_t) DRV_SPI_STATE_RX_FIFO_UNDERRUN | (uint32_t) DRV_SPI_STATE_TX_FIFO_OVERFLOW
          | (uint32_t) DRV_SPI_STATE_TX_FIFO_UNDERRUN))
      {
        // Will catch all states
      }
      __BKPT(0);
      // A diagnostics function and fault handling shall be performed here
      continue;
    default:
      // should not occur
      __BKPT(0);
      // A diagnostics function and fault handling shall be performed here
      continue;
    }
  }
  return eReturn;
}
\endcode

Conventions
-----------
We would like to give an introduction in the conventions used in this library.

### Naming
As a naming convention we will use a namespacing scheme. Every driver call or data
type will be prefixed with "DRV_" and the files with "netx_drv_". After those prefixes
the next part will be the drivers module name like "DRV_SPI", which is in this case the
SPI module.
#### Files
All files shall be prefixed by "netx_drv". Thus for example the spi driver would be named netx_drv_spi.
The suffix shall suffice the language requirements. So C source files are suffixed with ".c" and the headers
with ".h". In the case of C++ the sources are suffixed with ".cpp" and the headers with ".hpp".
Assembler files with ".S" or ".s". The CMSIS SVD file has the suffix ".svd" and documentations end with
".txt". The waf script is without any suffix and named unique by folder as wscript.
#### Devices
After the drivers prefix DRV_ the device is specified with a short naming in capital letters DRV_DEV
#### Enumerations
The enumerations will all be prefixed with DRV_ and then followed by the device for which they are defined.
Then the name of the enumeration is given in capital letters  DRV_DEV_ENUM_E 
#### Definitions
DRV_DEV_SOMETHING
#### Structures
struct DRV_DEV_STRUCT_Ttag

DRV_DEV_STRUCT_T
#### CMSIS and device header
__NVIC and so on.
#### Functions
DRV_DEV_Init
#### Variables
enum eVariableValue

### Documentation
There shall be documentation everywhere
#### Files
At the beginning. The doxygen grouping. Disclaimer
#### Types
Before the types definition. Each structure or enum element
#### Functions
Before the function in the header brief with member of and
in the source detailled with member of, params return and
visibility.

### Code Formatting

Memory Model
------------
The drivers are designed in the way that a caller is able to allocate the context
handle on its stack. The drivers do not allocate memory on themselfs. Also the drivers
API is protected with an os free mutex implementation for locking. This of course
might be exchanged with operating system functions. So in drivers default, it is always
possible to analyze the used stack memory with call graphs.

Compiling the driver
--------------------
The driver has a file called user_drv_conf_template.h which shall be copied/renamed to
user_drv_conf.h and modified. The driver has to be build without any extra parameters 
necessary. The CMSIS compliant device description header files and starting code has 
to be used. In the templates and Examples provided from hilscher are waf scripts.
Those scripts contain the script parts necessary for compiling the driver.

The driver component depends on the CMSIS component and both shall be used together.

In the waf scripts we recommend the following compiler options:
\code{.ld}
__STACK_SIZE = 0x00002000 
__HEAP_SIZE =  0x00002000

defines = ["__STACK_SIZE = %u" % __STACK_SIZE,
           "__HEAP_SIZE = %u" % __HEAP_SIZE,
           "__STARTUP_CLEAR_BSS",
           "__START = main",
           ]
\endcode

Linking the program
-------------------
The example linker scripts are available in the CMSIS folder.

Changelog
--------- 

Version       | Date       | Who  | Description
--------------|------------|------|-----------------------------------
V0.1.5.3      | 2024-10-07 | DM   | enhancements and bug-fixes
V0.1.5.2      | 2024-06-21 | DM   | enhancements and bug-fixes
V0.1.5.1      | 2024-04-22 | DM   | enhancements and bug-fixes
V0.1.5.0      | 2024-02-20 | DM   | enhancements and bug-fixes
V0.1.4.0      | 2023-06-27 | EB   | enhancements and bug-fixes
V0.1.3.0      | 2022-02-09 | EB   | Versioning, usage for 'Template'
V0.1.2.0      | 2022-01-28 | JZ   | Versioning
V0.1.0.3      | 2020-11-06 | AGR  | SPI and I2C
V0.1.0.2      | 2020-09-28 | AGR  | GCC Note
V0.1.0.1      | 2020-09-28 | AGR  | Versioning
V0.1.1.0      | 2020-09-22 | AGR  | Versioning
V0.1.0.0      | 2020-09-07 | AGR  | NAE, MCP and FPU
V0.0.5.0      | 2019-12-17 | AGR  | Header changes
V0.0.4.10     | 2019-12-16 | AGR  | XPIC Hotfix
V0.0.4.9      | 2019-12-05 | AGR  | SPI and UART Hotfix
V0.0.4.8      | 2019-10-07 | AGR  | CMSIS Temperature Support
V0.0.4.7      | 2019-05-14 | AGR  | CMSIS Template ld
V0.0.4.6      | 2019-05-02 | AGR  | TIM Hotfix
V0.0.4.5      | 2019-04-25 | AGR  | Info Page Fix
V0.0.4.4      | 2019-04-17 | AGR  | Production Tested Chips
V0.0.4.3      | 2018-12-18 | AGR  | End of year
V0.0.4.2      | 2018-11-22 | AGR  | Trading Show Beta
V0.0.4.1      | 2018-11-15 | AGR  | First Hotfix Round
V0.0.4.0      | 2018-11-13 | AGR  | Final Silicon Pre-Beta
V0.0.3.0      | 2018-07-20 | AGR  | DMA Pre-Beta
V0.0.2.0      | 2018-03-01 | AGR  | CAN Pre-Beta
V0.0.1.0      | 2018-01-26 | AGR  | Pre-Beta
V0.0.0.3564   | 2018-01-16 | AGR  | Alpha
V0.0.0.3452   | 2017-11-06 | AGR  | Pre-Alpha

### V0.1.5.3

** Bug
- [NX90APPDRV-183] - meaningless test of eDioMskOutputReference in DRV_TIM_ChannelGetCapture()
- [NX90APPDRV-264] - I2C - Master Transmit results in mFIFO Underrun when addressing is 10-bit
- [NX90APPDRV-265] - I2C - Master Receive results in mFIFO Underrun when addressing is 10-bit
- [NX90APPDRV-266] - I2C - Master Transmit does not work with 10-bit slave addressing
- [NX90APPDRV-267] - I2C - Master Receive does not work correctly with 10-bit slave addressing
- [NX90APPDRV-268] - I2C - Master Transmit repeats sending 10-bit address' second byte in continuous mode
- [NX90APPDRV-269] - Hardfault occurs when debugging with SDRAM as target 
- [NX90APPDRV-270] - I2C - Master Receive repeats sending second 10-bit address' byte in continuous mode
- [NX90APPDRV-271] - I2C - sAckPollMaximum configuration value not validated. Value range should be added in description.
- [NX90APPDRV-273] - I2C - eSpeedMode is not validated
- [NX90APPDRV-274] - I2C - Polling mode MasterTransmit() and MasterReceive() error in transfer in consecutive Continuous and Continuous End operations
- [NX90APPDRV-275] - I2C - Slave Transmit function generates cyclic IRQs while waiting for Master's request
- [NX90APPDRV-276] - I2C - Abort function fix
- [NX90APPDRV-277] - I2C - Device address verification for Master and Slave mode
- [NX90APPDRV-279] - I2C - Slave does not detect transfer direction change or premature stop from Master
- [NX90APPDRV-281] - I2C - Master does not send Stop in IRQ mode, when Slave is not responding
- [NX90APPDRV-282] - I2C - Master Receive with 10-bit addressing does not correspond to the Philips standard

### V0.1.5.2

** Bug
- Bit Field usage must result always in word access, no byte access
- DRV_TIM_LatchSytimes - Corrected wrong spelling
- linker-script 'netx90_app_intram.ld' needs ALIGN(8)
- un-lucky defines like 'hash' in 'netx90_app.h' - implemented a mechanism to resolve definition conflicts with language reserved words
- DRV functions return DRV_LOCKED in single thread usage - little modification in Semaphore of locking mechanism
- Wrong file name in header comment block of netx_drv_user_conf_template.h
- DRV_UART_Receive always return BUSY if a RX timeout occurs in polling mode
- DMAC - Transfer of more than 4095 byte is not working

** New Feature
- Add version parameter to bld.stlib commmand in the component wscript



### V0.1.5.1

** Bug
- I2C module is used only as a master device, but get param error if don't config tSlaveAddress parameter
- I2C copy and paste errors
- I2C - Watermark issue when receiving data as a slave
- DRV_I2C_MasterTransmit() function seems to set incorrect I2C command in IRQ mode
- I2C - Compile error due to bool usage
- I2C - Device does not transmit data correctly in slave mode
- I2C - On High Speed in Slave Mode, Device does not transmit correctly first bytes of data
- I2C - Polling mode Master Receive functionality ends a transaction to be continued with STOP on the line
- I2C - MasterTransmit() continues mode correction
- I2C - Copying all received data out of FIFO not possible in Slave mode after STOP
- I2C - Watermark issue when receiving data as a master
- I2C - In Master Receive IRQ mode transaction is not finished if ISR is interrupted by higher priority IRQ

** New Feature
- I2C Timeout Config Entry

** Change
- I2C - Implement timeout functionality in Slave Polling mode


### V0.1.5.0

** Bug
- ptTim->ulSubID
- SPI RefillLevel
- SPI device in IRQ mode stuck with DRV_BUSY when sending small amount of data with low SPI frequencies 
- DIO - Reconfiguration of the GPIO capture mode with DRV_DIO_ChannelSetMode does not work correctly
- I2C - Watermark issue when receiving data as a slave
- UART - DRV_UART_ABORT not working correctly in DMA mode
- Implement writing of ADC Sequencer command register once per job with all parameters
- UART - Transmission does not work properly when watermark level is 1 in DMA mode
- UART - ullFrameStartTick for timeout is not reset in DRV_UART_GetChar and DRV_UART_PutChar
- Problem when using a SQI interface from app side in standard SPI full-duplex mode
- SQI Chip Select hardware mode issue
- Function DRV_I2C_Abort() returns only negative result
- DRV_I2C_MasterReceive() function is no more useable if returns with timeout in polling mode

** New Feature
- Test SPI slave functionality in SQI capable SPI driver
- Uart driver: add support to 6.25M baudrate
- Trimmed ADC_VREF for low-level driver of temperature sensor
- In function DRV_ADC_Start() wrong check for selected sequencer

** Change
- UART Baud Rate Mode Calculation
- netX 90 regdef update due to ADC register description
- Change ADC software default value for tt_add to 10



### V0.1.4.0 Versioning, some enhancements due requests
- DRV_LOCK/'DRV_UNLOCK' introduced different solution for these macros 'as new default'
        in 'netx_drv_conf.h', 'netx_drv.h', not using LDREX/STREX instructions
        Note: these macros are used/included by 'all drivers'
- ADC   changed config data type of DMA Base Address 
- ADC   fixed wrong check for selected sequencer in DRV_ADC_Start 
- TIMER fixed DRV_TIM_Init not returning DRV_NSUPP when DMA mode is selected
- DIO   fixed bug in DRV_DIO_ChannelSetMode where reconfiguring of GPIO capture mode did not work
- CMSIS added feature 'using Trimmed ADC_VREF for low-level driver of temperature sensor'

### V0.1.3.0 Versioning, note on some driver-code
- ADC added more description-text in .c and .h file
- SPI introduced a fix for getting SQI-Mode 'using DMA' working

### V0.1.2.0 Versioning
- version with updated wscript

### V0.1.0.3 SPI and I2C
- SPI fixed size correction
- I2C missing clr added
- DMAC added the falsely removed align. Note does still not disapear.

### V0.1.0.2 GCC Note
- DMAC Another approach to the note regarding packed bit-fields in gcc 4.4 increased the amount of notes, so it was reverted again

### V0.1.1.0 Versioning
- Tagged the wrong version

### V0.1.0.0 NAE, MCP and FPU
- CMSIS FPU support added
- CMSIS NAE in linker scripts is correctly placed now
- CMSIS License of linker scripts has changed
- WAF modified configuration struct
- DMAC abort now disables the device before waiting for the active flag to go down
- I2C Refill Level was added
- I2C Clear is now functional
- I2C get state now copes with missing state parameter
- SPI Abort now also for polling mode
- SPI Refill Level was added
- SPI added assert
- SPI fixed size correction
- UART Abort fixed

### V0.0.5.0 Header changes
Changed the wscript and tagged basically the V0.0.4.10 again due to the
changes in driver and CMSIS regarding the headers linker files
and everything that was part of V0.0.4.9 and V0.0.4.10


### V0.0.4.10 Header Changes
- Added error message for MPW header include
- Removed MPW chip support package header file
- Wrong include of obsolete MPW header fixed

### V0.0.4.9 SPI and UART Hotfix
- SPI init assert at wrong position
- UART missing DRV_OK returns in certain cases
- UART optimization of the Tx context reset constraints
- CMSIS updated file header informations in the linker script

### V0.0.4.8 CMSIS Temperature Support
- CMSIS new regdef base for netX90, not the netX MPW, so the MPW support is broken
- CMSIS extended for calibration/reference value support and convenience functions 
- CMSIS C++ constructor execution routine integrated
- CORTEX and CMSIS added software reset vector
- CSP changes for new regdef/svd and linter
- ADC naming fix of DRV_ADC_DEVICE_ID_E to DRV_ADC_SEQ_DEVICE_ID_E including the IRQ handler
- ADC initializer and documentation updated
- ADC linter related changes like break added and several type casts
- MLED changes related to the new regdef/svd file and linter
- ETH MAC XPIC driver updated and now supported by netX 90
- BiSS changes regarding the ETH MAC XPIC support
- SPI naming change of DRV_SPI_FSS_STATIC_E to more meaningfull names
- SPI eDataSize now set as supposed
- SPI DMAC abort now working as needed
- SPI DMA callback does not free context anymore
- SPI fss static driver in dma mode removed from configuration space 
- Mutex and lock definitions expanded by an type and value define
- TRACE_PRINTER define added to header
- Non functional linter issuses fixed in TRACE, DIO, DMAC, I2C, SPI, UART
- Readme and documentation updates

### V0.0.4.7 CMSIS Template ld
- ld file referred to PageExtraction an should have been PageReader
- SPI added more robustness to init and fifo clr
- DIO fixed bod unmask

### V0.0.4.6 TIM Hotfix
- TIM correction of assert

### V0.0.4.5 Info Page Fix
- CMSIS info page reader cache reset and better guarding
- ADC includes added

### V0.0.4.4 Production Tested Chips
- SPI Device ID documentation corrected
- SPI reset/default value and init of data size select fixed
- SPI alignment calculation fixed
- SPI and UART FiFo servicing order changed
- SQI fss set fixed
- UART variable set order changed
- UART added missing else
- UART str/ldr exclusive protection for watermark set
- UART added separate tx and rx get state functions
- DIO IRQ Mask fixed
- CSP DIO HIF ID fixed
- TRACE print function added
- Changed DRV_LOCK behaviour during initialization
- Moved RTOS_USED stubs into user_conf

### V0.0.4.3 End of year
- More documentation.
- IRQ disable fixed.
- MLED return values corrected.
- RMW of irq clear fixed in I2C, DIO, TIM
- DMA added missing IRQ clear pending
- I2C HS Master Code transmission got HS mode parameter check
- TIM boundary check fixed in pause and getValue and set preload
- I2C and UART Poll mode got tick timeout similar to spi
- SPI tick timeout behaviour homogenised with other modules
- SPI dma slave mode unlocked
- DeInit was implemented for ADC, I2C, DMA, SPI, UART

### V0.0.4.2 Trading Show Beta
- Some more documentation.
- UART get and put char is now accepting unsigned char
- CMSIS introduced Data memory barriers for the info page reader.

### V0.0.4.1 First Hotfix Round
- Some more documentation.
- Regdef include of legacy HALs for mpw fixed
- CMSIS linker script had wrong names for copy section
- DMAC irq chip support is now modular and dev 3 was added to list

### V0.0.4.0 Final Silicon Pre-Beta
This release shall have the support for the final silicon of the netX90 chip. Also the MLED module,
the SQI devices, the ADC device for the final and the ethernet_mac_xpic device drivers are included. 
- General documentation update
- CMSIS
  - Linker scripts
  - Final chip support including header and startup files
  - Flash info page reader for temperature calibration data
- DMAC asic_ctrl pointer changed and abort fixed
- SQI integration into the SPI
- ADC device driver added
- XPIC driver added
- ETH_XPIC added
- CSP for final silicon
- MLED driver added
- PIO driver overhaul for final silicon
- Hotfix merge of V0.0.3.3
- TIM deinit fixed
- Removed a lot of traces
- Abort for i2c

### V0.0.3.0 DMA Pre-Beta
This release introduces the chip support layer and it contains changes to some drivers and also new ones.
It also renames all driver files regarding the namespace convention.
- CMSIS was updated to 5.3.0 and the usage of the regdef was switched to the 
  svd generated headers in non legacy drivers (BiSS, CAN)
- SPI DMA and 16b support changed the master slave config option and the mode option
- I2C driver added
- DMA driver added
- BiSS legacy driver added
- TIM finished
- UART DMA support
- I2C driver added
- BOD support added to dio
- ITM support added to cortex

### V0.0.2.0 CAN Pre-Beta
The canctrl driver was added to support can functionality.
NOTE: The driver is just a temporary implementation and might be replaced in the future.

### V0.0.1.0 Pre-Beta
First release contains everything until today. (V0.0.0.3452, V0.0.0.3452)
Since last version
- DRV TIM get counter value call has changed
- added the user_drv_conf_template.h
- The callbacks now contain the driver context as first parameter
- DRV UART interrupt mode
- DRV UART getchar and putchar methods
- Some minor bugfixes in tim and spi
- DRV DIO line api changes regarding the final chip 

### V0.0.0.3564 Alpha
- New regdef integrated
- CMSIS updated with new devices
- DRV DIO finished
- DRV SPI poll and irq modes implemented
- DRV TIM poll and irq modes implemented for GPIO counter, ARM Timer and ARM SysTick
    with PWM, Capture and Compare functionality on GPIO
- DRV UART in poll mode only
- DRV Cortex with CMSIS function wrapping.
- Customization via user_drv_conf.h possible
- Doxygen documentation design

### V0.0.0.3452 Pre-Alpha
- CMSIS
- DRV GPIO
- Examples for GPIO
- DRV SPI POLL 8b

Documentation
-------------
Documentation in HTML-format can be generated by using <B>doxygen</B>.
A respective <EM>Doxyfile</EM> is part of this example. You can run doxygen from command line
or from inside <EM>netX Studio CDT</EM>. The output will be created in folder <EM>Doc</EM>.

### netX Studio CDT
Just click on @ symbol at menu bar and choose Doxyfile.
At the first time using this, <EM>netX Studio CDT</EM>
will ask for installing Doxygen and Graphviz, which have to be installed.

### Command line
Just execute <code>doxygen</code> from command line in the root directory of this example.
It is required to have doxygen.exe in the PATH. If the "graphviz" package is installed,
the documentation contains visual dependency diagrams.
Due to a bug in dogygen, it is necessary to specify the path to dot.exe (Graphviz)
in the "Doxyfile" -> DOT_PATH parameter

\note The doxygen documentation requires doxygen version 1.8.0. or higher in order to 
support Markdown plain text formatting
\note The doxygen tag INCLUDE_PATH has to be set to include the correct user_drv_conf.h to 
generate the documentation properly.

DISCLAIMER
----------
  Exclusion of Liability for this demo software:
  The following software is intended for and must only be used for reference and in an
  evaluation laboratory environment. It is provided without charge and is subject to
  alterations. There is no warranty for the software, to the extent permitted by
  applicable law. Except when otherwise stated in writing the copyright holders and/or
  other parties provide the software "as is" without warranty of any kind, either
  expressed or implied.
\note
  Please refer to the Agreement in the disclaimer "README_DISCLAIMER.txt", provided  together with this file!
  By installing or otherwise using the software, you accept the terms of this Agreement.
  If you do not agree to the terms of this Agreement, then do not install or use the
  Software!
  
**************************************************************************************/