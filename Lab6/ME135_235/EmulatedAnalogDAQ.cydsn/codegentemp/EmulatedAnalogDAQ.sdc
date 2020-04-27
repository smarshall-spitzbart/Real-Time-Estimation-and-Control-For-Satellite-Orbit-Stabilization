# THIS FILE IS AUTOMATICALLY GENERATED
# Project: C:\Users\George Anwar\Documents\UC Berkeley\ME135 Spring 2020\PSOC Creator\ME135_235\EmulatedAnalogDAQ.cydsn\EmulatedAnalogDAQ.cyprj
# Date: Thu, 19 Mar 2020 00:32:24 GMT
#set_units -time ns
create_clock -name {ADC_DelSig_1_Ext_CP_Clk(routed)} -period 41.666666666666664 -waveform {0 20.8333333333333} [list [get_pins {ClockBlock/dclk_0}]]
create_clock -name {WaveDAC8_1_DacClk(routed)} -period 10000 -waveform {0 5000} [list [get_pins {ClockBlock/dclk_1}]]
create_clock -name {CyILO} -period 1000000 -waveform {0 500000} [list [get_pins {ClockBlock/ilo}] [get_pins {ClockBlock/clk_100k}] [get_pins {ClockBlock/clk_1k}] [get_pins {ClockBlock/clk_32k}]]
create_clock -name {CyIMO} -period 333.33333333333331 -waveform {0 166.666666666667} [list [get_pins {ClockBlock/imo}]]
create_clock -name {CyPLL_OUT} -period 41.666666666666664 -waveform {0 20.8333333333333} [list [get_pins {ClockBlock/pllout}]]
create_clock -name {CyMASTER_CLK} -period 41.666666666666664 -waveform {0 20.8333333333333} [list [get_pins {ClockBlock/clk_sync}]]
create_generated_clock -name {CyBUS_CLK} -source [get_pins {ClockBlock/clk_sync}] -edges {1 2 3} [list [get_pins {ClockBlock/clk_bus_glb}]]
create_clock -name {ADC_DelSig_1_Ext_CP_Clk} -period 41.666666666666664 -waveform {0 20.8333333333333} [list [get_pins {ClockBlock/dclk_glb_0}]]
create_generated_clock -name {ADC_DelSig_1_theACLK} -source [get_pins {ClockBlock/clk_sync}] -edges {1 39 77} [list [get_pins {ClockBlock/aclk_glb_0}]]
create_generated_clock -name {WaveDAC8_1_DacClk} -source [get_pins {ClockBlock/clk_sync}] -edges {1 241 481} [list [get_pins {ClockBlock/dclk_glb_1}]]

set_false_path -from [get_pins {__ONE__/q}]

# Component constraints for C:\Users\George Anwar\Documents\UC Berkeley\ME135 Spring 2020\PSOC Creator\ME135_235\EmulatedAnalogDAQ.cydsn\TopDesign\TopDesign.cysch
# Project: C:\Users\George Anwar\Documents\UC Berkeley\ME135 Spring 2020\PSOC Creator\ME135_235\EmulatedAnalogDAQ.cydsn\EmulatedAnalogDAQ.cyprj
# Date: Thu, 19 Mar 2020 00:32:17 GMT
