#!/bin/bash

echo "exporting RISCV"

export ARCH=riscv
export CROSS_COMPILE=riscv64-unknown-elf-
export RISCV=/opt/riscv
export PATH=/opt/riscv/bin:$PATH
export PLATFORM=alsaqr
export CONFIG=alsaqr-baremetal
export CONFIG_BUILTIN=y




cd bao-baremetal-guest
make CROSS_COMPILE=riscv64-unknown-elf- PLATFORM=alsaqr SINGLE_CORE=y
cd ../bao-hypervisor
rm -rf ./bin;make CROSS_COMPILE=riscv64-unknown-elf- PLATFORM=alsaqr CONFIG=alsaqr-linux CONFIG_BUILTIN=y 
cd ../opensbi
rm -rf ./build/platform;make PLATFORM=fpga/alsaqr FW_PAYLOAD=y FW_PAYLOAD_PATH=../bao-hypervisor/bin/alsaqr/builtin-configs/alsaqr-linux/bao.bin FW_FDT_PATH=../linux/occamy.dtb
cd ..
cp opensbi/build/platform/fpga/alsaqr/firmware/fw_payload.elf ./fw_payload.elf

cp bao-baremetal-guest/build/alsaqr/baremetal.elf .
cp bao-baremetal-guest/build/alsaqr/baremetal.asm .
cp bao-hypervisor/bin/alsaqr/builtin-configs/alsaqr-baremetal/bao.elf .
cp bao-hypervisor/bin/alsaqr/builtin-configs/alsaqr-baremetal/bao.asm .
riscv64-unknown-elf-objcopy --only-keep-debug baremetal.elf baremetal.debug
riscv64-unknown-elf-objcopy --only-keep-debug bao.elf bao.debug
riscv64-unknown-elf-objcopy --only-keep-debug fw_payload.elf fw_payload.debug
riscv64-unknown-elf-objdump -D fw_payload.elf > fw_payload.dump

alias gdb='riscv64-unknown-elf-gdb fw_payload.elf'
