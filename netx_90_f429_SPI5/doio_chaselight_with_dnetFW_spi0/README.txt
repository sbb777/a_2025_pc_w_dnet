/**********************************************************************************//**
Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
**************************************************************************************//*
$Id: README.txt 11652 2024-10-18 08:44:33Z Jknorr $:
**************************************************************************************//**
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
  Please refer to the Agreement in \ref disclaimer "README_DISCLAIMER.txt", provided 
  together with this file! By installing or otherwise using the software, you accept 
  the terms of this Agreement. If you do not agree to the terms of this Agreement, 
  then do not install or use the software!
**************************************************************************************//**

\mainpage

Introduction
------------

This is a template containing the vendor specific CMSIS implementation and a driver package.
It supports netX 90 chips. The corresponding hardware_config_xxx.hwc file for your device should be
flashed accordingly. If try to debug the example in sdram (netx90_app_sdram), the extension board
with SDRAM and the corresponding hardware_config_idpm_flash_sdram.hwc must be flashed accordingly.

Both netX90 Rev.1 and Rev.2 are supported, for more information about chip revisions and evaluation 
board revisions, please refer to the following FAQ page.
https://hilscher.atlassian.net/l/cp/U7voWryU

Changelog
--------- 

Version     | Date       | Who  | Description
------------|------------|------|------------------------------------------------------------
V0.0.1.0    | 2018-01-26 | AGR  | Pre Release
V0.0.2.0    | 2018-08-01 | JZ   | updated components and hardware_config files
V0.0.3.0    | 2018-11-26 | JZ   | updated components and hardware_config files (netX90 final)
V0.0.4.0    | 2019-01-21 | JZ   | updated components and hardware_config files
V0.0.5.0    | 2019-05-23 | JZ   | updated components, hardware_config files and linker files
V0.0.6.0    | 2019-10-08 | JZ   | updated components, folder structure changed
V0.0.7.0    | 2020-01-24 | AGR  | updated components, folder structure changed
V0.1.0.0    | 2020-11-11 | JZ   | updated components, hardware_config files and linker files
V0.1.1.0    | 2022-01-27 | JZ   | updated hardware_config files and waf extension
V0.1.2.0    | 2022-01-28 | JZ   | updated components, hardware_config files and wscript
V0.1.3.0    | 2022-02-11 | JZ   | updated components and waf extension
V0.1.3.1    | 2022-09-19 | JZ   | updated hardware_config files and FDL files
V0.1.4.0    | 2023-10-26 | JK   | updated components, HWC and FDL files, and Hilscher waf extension
V0.1.4.1    | 2023-12-01 | JZ   | updated HWC and FDL files, added WFP in the template
V0.1.4.2    | 2024-01-15 | JZ   | updated HWC files
V0.1.4.3    | 2024-02-20 | JK   | updated components
V0.1.4.4    | 2024-10-18 | JK   | updated components, HWC files, WFP and linker scripts

### V0.1.4.4 Pre Release
- updated component CMSIS V0.1.5.3
- updated component netx_drv V0.1.5.3
- updated HWC files (hwconfig_tool V4.1.8)
- updated WFP files
- updated INTRAM and SDRAM linker scripts with fixes for alignment issues that occured in certain situations

### V0.1.4.3 Pre Release
- updated component CMSIS V0.1.5.0
- updated component netx_drv V0.1.5.0

### V0.1.4.2 Pre Release
- updated HWC files

### V0.1.4.1 Pre Release
- updated HWC files
- updated FDL files
- added WFP in the template

### V0.1.4.0 Pre Release
- updated component CMSIS V0.1.4.0
- updated component netx_drv V0.1.4.0
- updated HWC files
- updated FDL files
- updated Hilscher waf extension V1.13.0.1

### V0.1.3.1 Pre Release
- updated hwc files (hwconfig_tool V4.0.1)
- updated FDL templates, which support NXE/NAE(use case C) and NAE(use case B)

### V0.1.3.0 Pre Release
- updated Hilscher waf extension
- updated component CMSIS V0.1.3.0
- updated component netx_drv V0.1.3.0

### V0.1.2.0 Pre Release
- updated component CMSIS V0.1.2.0
- updated component netx_drv V0.1.2.0
- updated hwc files (hwconfig_tool V4.0.0)
- updated wscript

### V0.1.1.0 Pre Release
- updated hwc files (hwconfig_tool V3.0.23)
- updated Hilscher waf extension

### V0.1.0.0 Pre Release
- updated component CMSIS V0.1.0.2
- updated component netx_drv V0.1.0.3
- moved netx_drv_user_conf.h from Components/netx_drv to Targets/NXHX90-JTAG/Include
- updated wscript and linker files accordingly
- updated hwc files (hwconfig_tool V3.0.17)
- added Hilscher waf extension in the project
- added Doc folder

### V0.0.7.0 New Structure
- updated component netx_drv V0.0.5.0
- updated component CMSIS V0.0.5.0
- FDL, HWC and LFW folder structure
- boot headers and docs

### V0.0.6.0 Pre Release
- updated component netx_drv V0.0.4.8
- updated component CMSIS V0.0.4.8
- FDL, HWC and LFW folder structure

### V0.0.5.0 Pre Release
- separate hardware_config files for netX90 and netX90 rev.0 (hwconfig tool V3.0.8)
- updated component netx_drv V0.0.4.7
- updated component CMSIS V0.0.4.7
- updated linker files
- added fdl template file for use case A

### V0.0.4.0 Pre Release
- updated hardware_config files (hwconfig tool V3.0.4)
- updated component netx_drv V0.0.4.3
- updated component CMSIS V0.0.4.3

### V0.0.3.0 Pre Release
- updated wscript and linker files for netX90 final
- updated hardware_config files(hwconfig tool V3.0.2)
- updated component netx_drv V0.0.4.2
- updated component CMSIS V0.0.4.2

### V0.0.2.0 Pre Release
- updated component netx_drv V0.0.3.0
- updated component CMSIS V0.0.3.0 (CMSIS 5.3.0)
- updated hardware_config.xml (hwconfig tool V2.0.0)

### V0.0.1.0 Pre Release
- Created the template from scratch


Documentation
-------------
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

\note The doxygen documentation requires doxygen version 1.8.0. or higher in order to 
support Markdown plain text formatting
\note The doxygen tag INCLUDE_PATH has to be set to include the correct user_drv_conf.h to 
generate the documentation properly.


**************************************************************************************/
**************************************************************************************/