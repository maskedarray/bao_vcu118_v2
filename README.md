# Local Repo Instructions
Change paths correctly in setup.sh file
Change paths correctly in bao-hypervisor/configs/config-X/config.c file
# Prologue
Follow the same prologue as [here](https://github.com/ninolomata/bao-cva6-guide).

# Linux
Build the Linux image + dtb blob:
```
    make ARCH=rv64 IMAGE=../../linux_image/Image DTB=../occamy_bao.dtb TARGET=linux-rv64-alsaqr
```
# Bao-Hypervisor
```
    git clone https://github.com/ninolomata/bao-hypervisor.git
    git checkout openpiton-temp
    cp -r ../bao/configs/* ./configs
    cp -r ../bao/platform/* ./src/platform
    make PLATFORM=alsaqr CONFIG=alsaqr-linux CONFIG_BUILTIN=y
```
# Opensbi
```
    git clone https://github.com/AlSaqr-platform/opensbi.git
    make PLATFORM=fpga/alsaqr FW_PAYLOAD=y FW_PAYLOAD_PATH=../bao-hypervisor/bin/alsaqr/builtin-configs/alsaqr-linux/bao.bin FW_FDT_PATH=../linux/occamy.dtb
```

# Running code 

As of now, to interface with Alsaqr on FPGA we use [openocd](https://github.com/riscv/riscv-openocd). The debugger we use is [this one](https://www.olimex.com/Products/ARM/JTAG/ARM-USB-OCD-H/).

 * Open the hw manager and load the bitstream  **alsaqr_xilinx_llc_pc.bit**  (pre-built available on the folder **bitstream**)

 * Open 3 terminals:

### Terminal 1

```
openocd -f zcu-102-ariane.cfg
```
### Terminal 2

```
screen -L /dev/ttyUSBi 115200
```
### Terminal 3

```
riscv64-unknown-elf-gdb <path-to-compiled-elf>
from gdb terminal
(a) target remote :3333
(b) monitor reset halt
(c) load
(d) c
```
At this point you are supposed to see the uart prints on the screen.
