/**
 * Bao, a Lightweight Static Partitioning Hypervisor
 *
 * Copyright (c) Bao Project (www.bao-project.org), 2019-
 *
 * Authors:
 *      Jose Martins <jose.martins@bao-project.org>
 *      Sandro Pinto <sandro.pinto@bao-project.org>
 *
 * Bao is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation, with a special exception exempting guest code from such
 * license. See the COPYING file in the top-level directory for details.
 *
 */

#include <bao.h>

#include <cpu.h>
#include <mem.h>
#include <interrupts.h>
#include <console.h>
#include <printk.h>
#include <platform.h>
#include <vmm.h>
#include <arch/csrs.h>
#include <arch/opcodes.h>
#include <fences.h>


//////////////////////////////////////////////


volatile pmu_v1_global_t pmu_v1_global
    __attribute__((section(".devices")));


void pmu_v1_init_localinit(){
    mem_map_dev(&cpu.as, (void*)&pmu_v1_global, 0x10404000,1);
    fence_sync_write();
}

//////////////////////////////////////////////



void init(uint64_t cpu_id, uint64_t load_addr, uint64_t config_addr)
{
    /**
     * These initializations must be executed first and in fixed order.
     */

    
        {
        unsigned long _stime = CSRR(CSR_TIME);
        unsigned long _stime2 = CSRR(CSR_TIME);
        printk("The time is: %lu\r\n", _stime);
        
        volatile uint32_t comp_array[100] = {0};
        for (int i=0; i<100; i++) {
            comp_array[i] = comp_array[i] + i;
            //printf("Array: %d\r\n", comp_array[i]);
        }

        _stime2 = CSRR(CSR_TIME);
        printk("The time is: %lu\r\n", _stime2);

    }


    cpu_init(cpu_id, load_addr);
    mem_init(load_addr, config_addr);

    /* -------------------------------------------------------------- */
    
    if (cpu.id == CPU_MASTER) {
        console_init();
        printk("Bao Hypervisor\n\r");
    }
    printk("Uart page size is: \n\r");
    pmu_v1_init_localinit();
    printk("Uart page size is: \n\r");

    interrupts_init();


    vmm_init();

    /* Should never reach here */
    while (1);
}



