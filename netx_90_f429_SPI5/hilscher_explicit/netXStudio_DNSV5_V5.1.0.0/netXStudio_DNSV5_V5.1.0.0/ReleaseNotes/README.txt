/**********************************************************************************//**
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
**************************************************************************************//*
$Id: README.txt 7837 2020-06-03 06:18:21Z kai $:
**************************************************************************************//**
<H2>DISCLAIMER</H2>
  Exclusion of Liability for this demo software:
  The following software is intended for and must only be used for reference and in an
  evaluation laboratory environment. It is provided without charge and is subject to
  alterations. There is no warranty for the software, to the extent permitted by
  applicable law. Except when otherwise stated in writing the copyright holders and/or
  other parties provide the software "as is" without warranty of any kind, either
  expressed or implied.
\note
  Please refer to the Agreement in \ref disclaimer "README_DISCLAIMER.txt", provided together with this file!
  By installing or otherwise using the software, you accept the terms of this Agreement.
  If you do not agree to the terms of this Agreement, then do not install or use the
  Software!
**************************************************************************************//**

\mainpage

<H2>Introduction</H2>
This example is delivered with a **limited evaluation version** firmware - LFW, which must **not be used for production**
Please refer to \ref lfw "LFW/readme.txt" for some more information, related to the LFW.
The LFW (*.nxi file) must be programmed into the flash memory of netx 90

Depending on the use case A,B or C, a matching flash structure must be prepared. I.e. a matching Hardware Configuration HWC (*.hwc file), Flash Device Label FDL (*.fdl file) and the proper Maintancen Firmware MTF must be programmed into the flash internal memory. 

Please check netX 90 documentation and the FAQ section for more information

netX 90 page:         https://kb.hilscher.com/x/dYJ3B
netX 90 FAQ section:  https://kb.hilscher.com/x/KXQWBg

HWC templates for different use cases are provided under the directory HWC.

FDL templates for different use case are provided under the directory FDL, the device
number, serial number and MAC address must be changed accordingly.

### Common functions supported by the firmware:
- netX Diagnostic and Remote Access: \n
    Interface for Hilscher diagnosis and configuration tools via Serial Line (UART) on 
    netX90 /FTDI USB bridge on NXHX90
- DPM Channel 0 - RTE Protocol Specific and FW Generic Services
- DPM Channel 1 - Network Services: \n
    Network Services are included in all Real-Time Ethernet firmware variants and provide following functionality: \n
    Socket Interface API
        allows to use socket communication via netX integrated TCP/IP stack and same MAC address \n
    Ethernet Interface API (disabled by default, can be enabled via Firmware TagListEditor)
        allows to send / receive raw ethernet frames using dedicated own MAC address \n
    Web Interface API
        allows to forward specific URL requests (HTTP GET / POST) to user application to build custom web page content by the user
- Integrated Webserver: \n
    Use case A - Basic Web Server: \n
    Basic Webserver is included in all Real-Time Ethernet firmware variant and provides following built-in functionality: \n
    "URL\netx"
        provides simple grafical interface to other functions (diag, fw update, reset) \n
    "URL\netx\diag"
        delivers plain basic information about the netX device ( current uptime, MAC address, device number, serial number etc.) \n
    "URL\netx\firmware"
        allows to upload a new firmware update file to netX flash. Firmware install can be forced via Maintenance Firmware \n
    "URL\netx\reset"
        allows to trigger a netX reset i.e. to install the new uploaded firmware \n
    "URL\"
        URL requests are forwarded to the user application via DPM Channel 1(see Web Interface API) \n
    Use case C (netX with external SPI Flash and external SDRAM) - Extended Web Server: \n
    Provides the functions same as Basic Webserver, but extended with handling of local flash file system (external SPI Flash) \n
    "URL/netx/files"
        allows to stream the customer web content according to media type (i.e. txt, js, htm, xhtml, jpeg, json etc) located in the netX Flash File System \n
    Note: actual FW / tools do not provide yet convinient possibility to upload web server content files to netX file system. This will be improved in the next version

netx90_app_iflash_dummy.nai is a dummy file to be flashed on app iflash, if "netx90_app_usecase_X_sdram.elf" is used to debug the 
application instead of "netx90_app_usecase_X_iflash.nai". Because the firmware X090H000.nxi on com side will start, only if 
it has detected a valid application code on the app iflash.

There are other hardware config files of dpm or spm available under the folder HWC, if a companion chip is 
used as a host instead of netX90 app or external sdram is used on the host interface.

The following picture shows the application software structure. The application depends on a generic part and
a protocol specific one. The protocol specific code is in component <EM>cifXApplicationDemoEIS</EM>

Process data is handled in an interrupt service routine and is independend from used protocol stack.
\image html "LFW structure generic.png"

<H2>Protocol specific part</H2>
The protocol specific part is covered by the component cifXApplicationdemoXXX, XXX is a placeholder for PNS (PROFINET slave), EIS (EthernetIP slave), ECS (EtherCAT slave), etc.

Protocol specific startup begins in function Protocol_StartConfiguration(). 
All responses and indications are handled in Protocol_PacketHandler().


<H2>Documentation</H2>
Documentation in HTML-format can be generated by using <B>doxygen</B>.
A respective <EM>Doxyfile</EM> is part of this example. You can run doxygen from command line
or from inside <EM>netX Studio CDT</EM>. The output will be created in folder <EM>Doc</EM>.

<H3>netX Studio CDT</H3>
Just click on @ symbol at menu bar and choose Doxyfile.
First doing this <EM>netX Studio CDT</EM>
will ask for installing Doxygen and Graphviz, which need to be installed.

<H3>Command line</H3>
Just execute <code>doxygen</code> from command line in the root directory of this example.
It is required to have doxygen.exe in the PATH. If the "graphviz" package is installed,
the documentation contains visual dependency diagrams.
Due to a bug in dogygen, it is necessary to specify the path to dot.exe (Graphviz)
in the "Doxyfile" -> DOT_PATH parameter

<H3>Notes</H3>
- If new source code is added to the project, it might be necessary to add new source code directories or files to the INPUT statement in the doxygen configuration file Doxyfile.
- The doxygen documentation requires doxygen version 1.8.0. or higher in order to support Markdown plain text formatting
- It is recommended to edit Doxyfile using doxywizard to make a diff of two Doxyfiles easier.


**************************************************************************************/

