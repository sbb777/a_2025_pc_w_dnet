/*!\page hwc_files Hardware Configurations
NETX90 Hardware Configurations
==============================

The hardware configurations in this folder are preconfigured examples of common
use cases. The purpose is to use them out of the box. They may have to be adapted
to the application cases.

Compatibility version is:
 - netXStudio V1.1010.1.5895 (tool_version="4.0.3")

The sub folders contain preconfigured hardware configurations for 
the mass production chips(netx90) and the netRAPID90.

(Configurations for netx90_rev0 are not available, because netx90_rev0 are 
 no longer supported by netXStudio V1.09 and V1.10)

-----------------------------------------------------------------------------------
Configurations for netx90:

# hardware_config_16bitdpm.xml (Usecase A)
  - 16bit dpm interface
  - internal DPM (idpm) deactive
  - Application CPU deactive

# hardware_config_idpm_can.xml (Usecase A)
  - internal DPM (idpm) active
  - Application CPU active
  - CAN interface

# hardware_config_idpm_cclink.xml (Usecase A)
  - internal DPM (idpm) active
  - Application CPU active
  - CC-Link interface
  
# hardware_config_idpm_flash_sdram.xml (Usecase C)
  - internal DPM (idpm) active
  - Application CPU active
  - external flash active (NXHX 90-JTAG V5:MX25L3233F)
  - external SDRAM interface active
  
# hardware_config_idpm_profibus.xml (Usecase A)
  - internal DPM (idpm) active
  - Application CPU active
  - Profibus interface
  
# hardware_config_idpm.xml (Usecase A)
  - internal DPM (idpm) active
  - Application CPU active
  
# hardware_config_spm_flash_sdram.xml (Usecase C)
  - Serial DPM in mode3 active
  - internal DPM (idpm) deactive
  - Application CPU deactive
  - external flash active (NXHX 90-JTAG V5:MX25L3233F)
  - external SDRAM interface active
  
# hardware_config_spm.xml (Usecase A)
  - Serial DPM in mode3 active
  - internal DPM (idpm) deactive
  - Application CPU deactive
  
-----------------------------------------------------------------------------------
Configurations for netRAPID90:

# hardware_config_idpm_app_flash_sdram.xml (Usecase C)
  - Serial DPM deactive
  - internal DPM (idpm) active
  - Application CPU active
  - external flash active (NRPEB H90-RE V2:W25Q64CVAG(customer))
  - external SDRAM interface active
  
# hardware_config_idpm_app.xml (Usecase A)
  - Serial DPM deactive
  - internal DPM (idpm) active
  - Application CPU active
  
# hardware_config_spm_noapp_flash_sdram.xml (Usecase C)
  - Serial DPM in mode3 active
  - internal DPM (idpm) deactive
  - Application CPU deactive
  - external flash active (NRPEB H90-RE V2:W25Q64CVAG(customer))
  - external SDRAM interface active
  
# hardware_config_spm_noapp.xml (Usecase A)
  - Serial DPM in mode3 active
  - internal DPM (idpm) deactive
  - Application CPU deactive
  
*/