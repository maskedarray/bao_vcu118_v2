#include <config.h>

#include <config.h>

VM_IMAGE(linux_image, XSTR(/home/ninolomata/Desktop/Development/openpiton-hyp-guide/BAO-VCU-118/linux/lloader/linux-rv64-alsaqr.bin));

struct config config = {
    
    CONFIG_HEADER

    .vmlist_size = 1,
    .vmlist = {
        { 
            .image = {
                .base_addr = 0x80200000,
                .load_addr = VM_IMAGE_OFFSET(linux_image),
                .size = VM_IMAGE_SIZE(linux_image)
            },

            .entry = 0x80200000,

            .platform = {
                .cpu_num = 1,
                
                .region_num = 1,
                .regions =  (struct mem_region[]) {
                    {
                        .base = 0x80200000,
                        .size = 0x10E00000,
                    }
                },

                .dev_num = 2,
                .devs =  (struct dev_region[]) {
                    {
                        .pa = 0x40000000,   
                        .va = 0x40000000,  
                        .size = 0x10000,  
                        .interrupt_num = 2,
                        .interrupts = (uint64_t[]) {2}
                    },
                    {
                        .pa = 0x18000000,   
                        .va = 0x18000000,  
                        .size = 0x00001000,  
                        .interrupt_num = 4,
                        .interrupts = (uint64_t[]) {4,5,6,7}
                    },
                },

                .arch = {
                   .plic_base = 0xc000000,
                }
            },
        }
     }
};