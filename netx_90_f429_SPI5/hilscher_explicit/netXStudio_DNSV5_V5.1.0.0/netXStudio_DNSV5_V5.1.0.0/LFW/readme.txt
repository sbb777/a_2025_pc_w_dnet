/*********************************************************************************************//**
\page lfw NETX90 COM Loadable Firmware - LFW - LIMITED EVALUATION VERSION
===============================================================

The *.nxi files contain the netX90 limited evaluation loadable firmware,
implementing the RTE (Real-Time-Ethernet) or Fieldbus-Protocol.
This firmware is executed on the netX 90 COM (communication) side.
\note 
It must NOT BE USED for production. 

A full LFW version is a commercial product and must be licenced by Hilscher.
Please contact sales@hilscher.com for further information.

The limited evaluation LFW supports all features of the respective full LFW version,
but might stop communciation and require a reset after >30min of operation.

Please refer to the *_usedlibs.txt files for version information.
===================================

This distributes the following firmwares:

X0907000.nxi netX 90 DeviceNet Slave (Hardware use case A)
X0907001.nxi netX 90 DeviceNet Slave (Hardware use case C)

Please assure that the matching Hardware Config (*.hwc) and Flash Device Label (*.fdl) files
are loaded together with the LFW into the flash memory of netX 90.
The use cases require different FDLs.

Please check netX 90 documentation and the FAQ section for more information

netX 90 page:         https://hilscher.atlassian.net/wiki/x/BA_YC
netX 90 FAQ section:  https://hilscher.atlassian.net/wiki/x/DECYC

*************************************************************************************************/