# Invoke as
#
# quartus_sh -t de1-soc-project.tcl

# $moduleName.sv should include a module named $moduleName

set moduleName "lab1"
set systemVerilogSource "${moduleName}.sv"

project_new $moduleName -overwrite

foreach {name value} {
    FAMILY "Cyclone V"
    DEVICE 5CSEMA5F31C6
    PROJECT_OUTPUT_DIRECTORY output_files
    MIN_CORE_JUNCTION_TEMP 0
    MAX_CORE_JUNCTION_TEMP 85
    CYCLONEII_RESERVE_NCEO_AFTER_CONFIGURATION "USE AS REGULAR IO"
    NUM_PARALLEL_PROCESSORS 4
} { set_global_assignment -name $name $value }

set_global_assignment -name TOP_LEVEL_ENTITY $moduleName

foreach filename $systemVerilogSource {
    set_global_assignment -name SYSTEMVERILOG_FILE $filename
}

foreach {port pin} {    
    CLOCK_50 PIN_AF14
    
    KEY[0] PIN_AA14
    KEY[1] PIN_AA15
    KEY[2] PIN_W15
    KEY[3] PIN_Y16

    HEX0[0] PIN_AE26 
    HEX0[1] PIN_AE27 
    HEX0[2] PIN_AE28 
    HEX0[3] PIN_AG27 
    HEX0[4] PIN_AF28 
    HEX0[5] PIN_AG28 
    HEX0[6] PIN_AH28
    
    HEX1[0] PIN_AJ29 
    HEX1[1] PIN_AH29 
    HEX1[2] PIN_AH30 
    HEX1[3] PIN_AG30 
    HEX1[4] PIN_AF29 
    HEX1[5] PIN_AF30 
    HEX1[6] PIN_AD27
    
    HEX2[0] PIN_AB23 
    HEX2[1] PIN_AE29 
    HEX2[2] PIN_AD29 
    HEX2[3] PIN_AC28 
    HEX2[4] PIN_AD30 
    HEX2[5] PIN_AC29 
    HEX2[6] PIN_AC30
    
    HEX3[0] PIN_AD26 
    HEX3[1] PIN_AC27 
    HEX3[2] PIN_AD25 
    HEX3[3] PIN_AC25 
    HEX3[4] PIN_AB28 
    HEX3[5] PIN_AB25 
    HEX3[6] PIN_AB22
    
    HEX4[0] PIN_AA24 
    HEX4[1] PIN_Y23  
    HEX4[2] PIN_Y24  
    HEX4[3] PIN_W22  
    HEX4[4] PIN_W24  
    HEX4[5] PIN_V23  
    HEX4[6] PIN_W25
    
    HEX5[0] PIN_V25  
    HEX5[1] PIN_AA28 
    HEX5[2] PIN_Y27  
    HEX5[3] PIN_AB27 
    HEX5[4] PIN_AB26 
    HEX5[5] PIN_AA26 
    HEX5[6] PIN_AA25     
} {
    set_location_assignment $pin -to $port
    set_instance_assignment -name IO_STANDARD "3.3-V LVTTL" -to $port
}

set sdcFilename "${moduleName}.sdc"

set_global_assignment -name SDC_FILE $sdcFilename

set sdcf [open $sdcFilename "w"]
puts $sdcf {
    create_clock -period 20 [get_ports CLOCK_50]

    derive_pll_clocks -create_base_clocks
    derive_clock_uncertainty
}
close $sdcf

project_close
