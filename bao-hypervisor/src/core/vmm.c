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

#include <vmm.h>
#include <vm.h>
#include <config.h>
#include <cpu.h>
#include <iommu.h>
#include <spinlock.h>
#include <fences.h>
#include <string.h>
#include <ipc.h>
#include <arch/csrs.h>
#include <arch/opcodes.h>
#include <console.h>

///////////////////////////////////////////////////////
#define PMU_cfg_write(addr, val_)  (*(volatile unsigned int *)(long)(addr) = val_)
#define PMU_cfg_read(addr)         (*(volatile unsigned int *)(long)(addr))
#define PMU_NUM_COUNTER 4
#define PMU_TIMER_WIDTH 8
#define PMU_COUNTER_WIDTH 8
#define PMU_CONFIG_WIDTH 4
#define PMU_NUM_ELEMENT 10
#define PMU_IRQ_ID 143

void PMU_read_counters(int num_counter, long long unsigned int BASE_ADDR, int REG_SIZE_IN_BYTES) {
  // Make sure the read variable is 32bit.
  // The AXI4-Lite interconnect is 32bit.
  unsigned int val;
  for (int i=0; i<num_counter; i++) {
    val = *(long*)(BASE_ADDR);
    printk("Read: %x: %x\n", BASE_ADDR, val);
   
    BASE_ADDR += REG_SIZE_IN_BYTES;
  }
}

void PMU_write_counters(int num_counter, long long unsigned int BASE_ADDR, int REG_SIZE_IN_BYTES, long long val[]) {
  // The AXI4-Lite interconnect is 32bit.
  // Can only write to 32bits at a time.
  for (int i=0; i<num_counter; i++) {
    if (REG_SIZE_IN_BYTES==4) {
      PMU_cfg_write(BASE_ADDR, val[i]);
    } else if (REG_SIZE_IN_BYTES==8) {
      int val_l = val[i] & 0xFFFFFFFF;
      int val_h = val[i] >> 32;

      PMU_cfg_write(BASE_ADDR, val_l);
      PMU_cfg_write(BASE_ADDR+4, val_h);
    }
    printk("Write: %x: %x\n", BASE_ADDR, val[i]);
   
    BASE_ADDR += REG_SIZE_IN_BYTES;
  }
}

void pmu_v1_run_localrun(){

    long long unsigned int pmu_struct_base_addr = (uint64_t)&pmu_v1_global;

    long long unsigned int PMU_COUNTER_BASE_ADDR      = pmu_struct_base_addr;
    long long unsigned int PMU_EVENT_SEL_BASE_ADDR    = pmu_struct_base_addr + 1*PMU_NUM_COUNTER*8 + 0*PMU_NUM_COUNTER*4;
    long long unsigned int PMU_EVENT_INFO_BASE_ADDR   = pmu_struct_base_addr + 1*PMU_NUM_COUNTER*8 + 1*PMU_NUM_COUNTER*4;
    long long unsigned int PMU_INIT_BUDGET_BASE_ADDR  = pmu_struct_base_addr + 1*PMU_NUM_COUNTER*8 + 2*PMU_NUM_COUNTER*4;
    long long unsigned int PMU_PERIOD_REG_BASE_ADDR   = pmu_struct_base_addr + 2*PMU_NUM_COUNTER*8 + 2*PMU_NUM_COUNTER*4;
    long long unsigned int PMU_TIMER_BASE_ADDR        = pmu_struct_base_addr + 2*PMU_NUM_COUNTER*8 + 2*PMU_NUM_COUNTER*4 + 1*PMU_TIMER_WIDTH;

    long long int counter_val[]      = {0x100, 0x200, 0x300, 0x400};
    long long int event_sel_val[]    = {0x3F, 0x2F, 0x4F, 0x5F};
    long long int event_info_val[]   = {0xB00, 0xA00, 0xC00, 0xD00};
    long long int init_budget_val[]  = {0xFFFFFFFFFFFFFFFE, 0xFFFFFA000, 0xFFFFFB000, 0xFFFFFC000};
    long long int period_val[]       = {0x100};


    printk("Hello PMU!\n");
    

    printk("Counter\n");
    PMU_write_counters(PMU_NUM_COUNTER, PMU_COUNTER_BASE_ADDR, PMU_COUNTER_WIDTH, counter_val);
    printk("EventSel Config\n");
    PMU_write_counters(PMU_NUM_COUNTER, PMU_EVENT_SEL_BASE_ADDR, PMU_CONFIG_WIDTH, event_sel_val);
    printk("EventInfo Config\n");
    PMU_write_counters(PMU_NUM_COUNTER, PMU_EVENT_INFO_BASE_ADDR, PMU_CONFIG_WIDTH, event_info_val);
    printk("Initital Budget\n");
    PMU_write_counters(PMU_NUM_COUNTER, PMU_INIT_BUDGET_BASE_ADDR, PMU_COUNTER_WIDTH, init_budget_val);
    printk("Period Register\n");
    PMU_write_counters(1, PMU_PERIOD_REG_BASE_ADDR, PMU_COUNTER_WIDTH, period_val);

    printk("Counters initialized!\n");
    

    volatile uint32_t comp_array[PMU_NUM_ELEMENT] = {0};
    for (int i=0; i<PMU_NUM_ELEMENT; i++) {
    comp_array[i] = comp_array[i] + i;
    }

    printk("Array traversed!\n");
    


    printk("Counter\n");
    PMU_read_counters(PMU_NUM_COUNTER, PMU_COUNTER_BASE_ADDR, PMU_COUNTER_WIDTH);
    printk("EventSel Config\n");
    PMU_read_counters(PMU_NUM_COUNTER, PMU_EVENT_SEL_BASE_ADDR, PMU_CONFIG_WIDTH);
    printk("EventInfo Config\n");
    PMU_read_counters(PMU_NUM_COUNTER, PMU_EVENT_INFO_BASE_ADDR, PMU_CONFIG_WIDTH);
    printk("Initital Budget\n");
    PMU_read_counters(PMU_NUM_COUNTER, PMU_INIT_BUDGET_BASE_ADDR, PMU_COUNTER_WIDTH);
    printk("Period Register\n");
    PMU_read_counters(1, PMU_PERIOD_REG_BASE_ADDR, PMU_TIMER_WIDTH);
    printk("Timer Register\n");
    PMU_read_counters(1, PMU_TIMER_BASE_ADDR, PMU_TIMER_WIDTH);

    printk("The test is over!\n");


}



////////////////////////////////////////////////////////

struct config* vm_config_ptr;

void vmm_init()
{
    if(vm_config_ptr->vmlist_size == 0){
        if(cpu.id == CPU_MASTER)
            INFO("No virtual machines to run.");
        cpu_idle();
    } 
    
    vmm_arch_init();

    volatile static struct vm_assignment {
        spinlock_t lock;
        bool master;
        size_t ncpus;
        uint64_t cpus;
        pte_t vm_shared_table;
    } * vm_assign;

    size_t vmass_npages = 0;
    if (cpu.id == CPU_MASTER) {
        iommu_init();

        vmass_npages =
            ALIGN(sizeof(struct vm_assignment) * vm_config_ptr->vmlist_size,
                  PAGE_SIZE) /
            PAGE_SIZE;
        vm_assign = mem_alloc_page(vmass_npages, SEC_HYP_GLOBAL, false);
        if (vm_assign == NULL) ERROR("cant allocate vm assignemnt pages");
        memset((void*)vm_assign, 0, vmass_npages * PAGE_SIZE);
    }

    cpu_sync_barrier(&cpu_glb_sync);

    bool master = false;
    bool assigned = false;
    size_t vm_id = 0;
    vm_config_t *vm_config = NULL;

    /**
     * Assign cpus according to vm affinity.
     */
    for (int i = 0; i < vm_config_ptr->vmlist_size && !assigned; i++) {
        if (vm_config_ptr->vmlist[i].cpu_affinity & (1UL << cpu.id)) {
            spin_lock((spinlock_t *) &vm_assign[i].lock);
            if (!vm_assign[i].master) {
                vm_assign[i].master = true;
                vm_assign[i].ncpus++;
                vm_assign[i].cpus |= (1UL << cpu.id);
                master = true;
                assigned = true;
                vm_id = i;
            } else if (vm_assign[i].ncpus <
                       vm_config_ptr->vmlist[i].platform.cpu_num) {
                assigned = true;
                vm_assign[i].ncpus++;
                vm_assign[i].cpus |= (1UL << cpu.id);
                vm_id = i;
            }
            spin_unlock((spinlock_t *) &vm_assign[i].lock);
        }
    }

    cpu_sync_barrier(&cpu_glb_sync);

    /**
     * Assign remaining cpus not assigned by affinity.
     */
    if (assigned == false) {
        for (int i = 0; i < vm_config_ptr->vmlist_size && !assigned; i++) {
            spin_lock((spinlock_t *)  (&vm_assign[i].lock));
            if (vm_assign[i].ncpus <
                vm_config_ptr->vmlist[i].platform.cpu_num) {
                if (!vm_assign[i].master) {
                    vm_assign[i].master = true;
                    vm_assign[i].ncpus++;
                    master = true;
                    assigned = true;
                    vm_assign[i].cpus |= (1UL << cpu.id);
                    vm_id = i;
                } else {
                    assigned = true;
                    vm_assign[i].ncpus++;
                    vm_assign[i].cpus |= (1UL << cpu.id);
                    vm_id = i;
                }
            }
            spin_unlock((spinlock_t *) (&vm_assign[i].lock));
        }
    }

    cpu_sync_barrier(&cpu_glb_sync);

    if (assigned) {
        vm_config = &vm_config_ptr->vmlist[vm_id];
        if (master) {
            size_t vm_npages = NUM_PAGES(sizeof(vm_t));
            void* va = mem_alloc_vpage(&cpu.as, SEC_HYP_VM, (void*)BAO_VM_BASE,
                                       vm_npages);
            mem_map(&cpu.as, va, NULL, vm_npages, PTE_HYP_FLAGS);
            memset(va, 0, vm_npages * PAGE_SIZE);
            fence_ord_write();
            vm_assign[vm_id].vm_shared_table =
                *pt_get_pte(&cpu.as.pt, 0, (void*)BAO_VM_BASE);
        } else {
            while (vm_assign[vm_id].vm_shared_table == 0);
            pte_t* pte = pt_get_pte(&cpu.as.pt, 0, (void*)BAO_VM_BASE);
            *pte = vm_assign[vm_id].vm_shared_table;
            fence_sync_write();
        }
    }

    cpu_sync_barrier(&cpu_glb_sync);

    if (cpu.id == CPU_MASTER) {
        mem_free_vpage(&cpu.as, (void*)vm_assign, vmass_npages, true);
    }

    ipc_init(vm_config, master);

    if (assigned) {
        vm_init((void*)BAO_VM_BASE, vm_config, master, vm_id);
        unsigned long _stime = CSRR(CSR_TIME);
        printk("The time is: %lu\r\n", _stime);
        pmu_v1_run_localrun();
        vcpu_run(cpu.vcpu);
    } else {
        cpu_idle();
    }
}
