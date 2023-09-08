#!/bin/bash

echo "exporting RISCV"

export ARCH=riscv
export CROSS_COMPILE=riscv64-unknown-elf-
export RISCV=/opt/riscv
export PATH=/opt/riscv/bin:$PATH
export PLATFORM=alsaqr
export CONFIG=alsaqr-baremetal
export CONFIG_BUILTIN=y



alias make1='make clean;make CROSS_COMPILE=riscv64-unknown-elf- PLATFORM=alsaqr'
alias make2='make clean;make CROSS_COMPILE=riscv64-unknown-elf- PLATFORM=alsaqr CONFIG=alsaqr-baremetal CONFIG_BUILTIN=y'
alias make3='rm -rf ./build; make PLATFORM=fpga/alsaqr FW_PAYLOAD=y FW_PAYLOAD_PATH=../bao-hypervisor/bin/alsaqr/builtin-configs/alsaqr-baremetal/bao.bin FW_FDT_PATH=../linux/occamy.dtb'

cd bao-baremetal-guest
make1
cd ../bao-hypervisor
make2
cd ../opensbi
make3
cd ..
cp opensbi/build/platform/fpga/alsaqr/firmware/fw_payload.elf ./fw_payload.elf

alias gdb='riscv64-unknown-elf-gdb fw_payload.elf'
