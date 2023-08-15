
#include <bao.h>
#include <pmu_v1.h>

#include <platform.h>
#include <cpu.h>
#include <mem.h>
#include <fences.h>
#include <spinlock.h>
#include <printk.h>

// All Widths are in Bytes
#define NUM_COUNTER 4
#define COUNTER_WIDTH 8
#define CONFIG_WIDTH 4
#define TIMER_WIDTH 8

// For the array traversal
#define NUM_ELEMENT 100

#define read_32b(addr)         (*(volatile uint32_t *)(long)(addr))
#define write_32b(addr, val_)  (*(volatile uint32_t *)(long)(addr) = val_)

volatile pmu_v1_global_t pmu_v1_global
    __attribute__((section(".devices")));


void read_32b_regs(uint32_t num_regs, uint64_t base_addr) {
  uint32_t val;
  for (uint32_t i=0; i<num_regs; i++) {
    val = read_32b(base_addr);
    printk("Read: %x: %x\n", base_addr, val);
    
    base_addr += 4;
  }
}

void write_32b_regs(uint32_t num_regs, uint64_t base_addr, uint32_t val[]) {
  for (uint32_t i=0; i<num_regs; i++) {
    write_32b(base_addr, val[i]);
    printk("Write: %x: %x\n", base_addr, val[i]);
    
    base_addr += 4;
  }
}

void read_64b_regs(uint32_t num_regs, uint64_t base_addr) {
  uint32_t val_l, val_h;
  uint64_t val;
  for (uint32_t i=0; i<num_regs; i++) {
    val_l = read_32b(base_addr);
    val_h = read_32b(base_addr+4);
    val = (uint32_t)((uint64_t)val_h << 32) | val_l;
    printk("Read: %x: %x\n", base_addr, val);
    
    base_addr += 8;
  }
}

void write_64b_regs(uint32_t num_regs, uint64_t base_addr, uint64_t val[]) {
  uint32_t val_l, val_h;
  for (uint32_t i=0; i<num_regs; i++) {
    val_l = val[i] & 0xFFFFFFFF;
    val_h = val[i] >> 32;
    write_32b(base_addr, val_l);
    write_32b(base_addr+4, val_h);
    printk("Write: %x: %x\n", base_addr, val[i]);
    
    base_addr += 8;
  }
} 


void pmu_v1_init(){
    mem_map_dev(&cpu.as, (void*)&pmu_v1_global, 0x10404000,1);
    fence_sync_write();
}

void pmu_v1_run(){
    uint64_t PMU_COUNTER_BASE_ADDR      = (uint64_t)&pmu_v1_global;
    uint64_t PMU_EVENT_SEL_BASE_ADDR    = PMU_COUNTER_BASE_ADDR + 1*NUM_COUNTER*COUNTER_WIDTH + 0*NUM_COUNTER*CONFIG_WIDTH;
    uint64_t PMU_EVENT_INFO_BASE_ADDR   = PMU_COUNTER_BASE_ADDR + 1*NUM_COUNTER*COUNTER_WIDTH + 1*NUM_COUNTER*CONFIG_WIDTH;
    uint64_t PMU_INIT_BUDGET_BASE_ADDR  = PMU_COUNTER_BASE_ADDR + 1*NUM_COUNTER*COUNTER_WIDTH + 2*NUM_COUNTER*CONFIG_WIDTH;
    uint64_t PMU_PERIOD_REG_BASE_ADDR   = PMU_COUNTER_BASE_ADDR + 2*NUM_COUNTER*COUNTER_WIDTH + 2*NUM_COUNTER*CONFIG_WIDTH;
    uint64_t PMU_TIMER_BASE_ADDR        = PMU_COUNTER_BASE_ADDR + 2*NUM_COUNTER*COUNTER_WIDTH + 2*NUM_COUNTER*CONFIG_WIDTH + 1*TIMER_WIDTH;

    uint64_t counter_val[]      = {0x100, 0x200, 0x300, 0x400};
    uint32_t event_sel_val[]    = {0x1F002F, 0x1F003F, 0x1F004F, 0x1F005F};
    uint32_t event_info_val[]   = {0xA00, 0xB00, 0xC00, 0xD00};
    uint64_t init_budget_val[]  = {0xFFFFFFFFFFFFFFFE, 0xFFFFFA000, 0xFFFFFB000, 0xFFFFFC000};
    uint64_t period_val[]       = {0x0};



    printk("Hello PMU!\n");
    

    printk("Counter\n");
    write_64b_regs(NUM_COUNTER, PMU_COUNTER_BASE_ADDR, counter_val);
    printk("EventSel Config\n");
    write_32b_regs(NUM_COUNTER, PMU_EVENT_SEL_BASE_ADDR, event_sel_val);
    printk("EventInfo Config\n");
    write_32b_regs(NUM_COUNTER, PMU_EVENT_INFO_BASE_ADDR, event_info_val);
    printk("Initital Budget\n");
    write_64b_regs(NUM_COUNTER, PMU_INIT_BUDGET_BASE_ADDR, init_budget_val);
    printk("Period Register\n");
    write_64b_regs(1, PMU_PERIOD_REG_BASE_ADDR, period_val);
    printk("Counters initialized!\n");
    
    volatile uint32_t comp_array[NUM_ELEMENT] = {0};

    for (uint32_t i=0; i<NUM_ELEMENT; i++) {
        comp_array[i] = comp_array[i] + i;
    }

    printk("Array traversed!\n");


    printk("Counter\n");
    read_64b_regs(NUM_COUNTER, PMU_COUNTER_BASE_ADDR);
    printk("EventSel Config\n");
    read_32b_regs(NUM_COUNTER, PMU_EVENT_SEL_BASE_ADDR);
    printk("EventInfo Config\n");
    read_32b_regs(NUM_COUNTER, PMU_EVENT_INFO_BASE_ADDR);
    printk("Initital Budget\n");
    read_64b_regs(NUM_COUNTER, PMU_INIT_BUDGET_BASE_ADDR);
    printk("Period Register\n");
    read_64b_regs(1, PMU_PERIOD_REG_BASE_ADDR);
    printk("Timer Register\n");
    read_64b_regs(1, PMU_TIMER_BASE_ADDR);
    printk("The test is over!\n");
}

void pmu_v1_interrupt_handler(){
    
    // printk("PMU interrupt in hypervisor\n\r");
    
}







