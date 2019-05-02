.PHONY: program clean

# Program the FPGA through the USB Blaster connection
# It is the second thing on the JTAG chain (the HPS block is first), hence @2

program : output_files/lab1.sof
	quartus_pgm -c 1 -m jtag -o "p;output_files/lab1.sof@2"

# Compile the project

output_files/lab1.sof : lab1.qpf lab1.sv
	quartus_sh --flow compile lab1

# Create the project files from a Tcl script

lab1.qpf lab1.qsf lab1.sdc : de1-soc-project.tcl
	quartus_sh -t de1-soc-project.tcl

# Create a .tar.gz file of the contents

TARFILES = Makefile lab1.sv de1-soc-project.tcl

lab1.tar.gz : $(TARFILES)
	mkdir lab1
	ln $(TARFILES) lab1
	tar zcf lab1.tar.gz lab1
	rm -rf lab1

clean :
	rm -rf db incremental_db output_files lab1.qpf lab1.qsf lab1.sdc \
	lab1.qws c5_pin_model_dump.txt
