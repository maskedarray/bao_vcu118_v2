
#include <bao.h>
#include <pmu_v1.h>

#include <platform.h>
#include <cpu.h>
#include <mem.h>
#include <fences.h>
#include <spinlock.h>
#include <printk.h>
#include <arch/csrs.h>
#include <arch/opcodes.h>

// #define PROFILE_PMU_ENTRY
#define PROFILE_PMU_EXIT

// All Widths are in Bytes
#define NUM_COUNTER 4
#define COUNTER_WIDTH 8
#define CONFIG_WIDTH 4
#define TIMER_WIDTH 8
#define MEMGUARD_PERIOD 2500000
#define COUNTER1_INIT_BUDGET 0xFFFFFFFFFFFFE400

// For the array traversal
#define NUM_ELEMENT 100

#define read_32b(addr)         (*(volatile uint32_t *)(long)(addr))
#define write_32b(addr, val_)  (*(volatile uint32_t *)(long)(addr) = val_)

#ifdef PROFILE_PMU_ENTRY
volatile uint32_t pmu_entry_saved[64];
volatile uint32_t pmu_entry_saved_counter = 0;
#endif
#ifdef PROFILE_PMU_EXIT
volatile uint32_t pmu_exit_saved[64];
volatile uint32_t pmu_exit_saved_counter = 0;
#endif

#ifdef PROFILE_PMU_EXIT
void pmu_save_exit_data(){
  uint64_t value;
  asm volatile ("lw %0, 0(%1)" : "=r" (value) : "r" (&pmu_v1_global.counters[0]));
  pmu_exit_saved[pmu_exit_saved_counter++] = value - COUNTER1_INIT_BUDGET;
  // printk("interrupt occurred %d\r\n",(uint32_t)(value));
}
#endif

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

    uint64_t counter_val[]      = {COUNTER1_INIT_BUDGET , 0x1, 0x1, 0x1};
    uint32_t event_sel_val[]    = {0x1F, 0x2F, 0x3F, 0x4F};
    uint32_t event_info_val[]   = {0xA00, 0xB00, 0xC00, 0xD00};
    uint64_t init_budget_val[]  = {COUNTER1_INIT_BUDGET , 0x1, 0x1, 0x1};
    uint64_t period_val[]       = {MEMGUARD_PERIOD};



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
    
    uint64_t present_time = CSRR(CSR_CYCLE);
    
    uint64_t end_time;
    asm volatile ("lw %0, 0(%1)" : "=r" (end_time) : "r" (&pmu_v1_global.timer));
    
    end_time = present_time + (MEMGUARD_PERIOD - end_time);
    // printk("interrupt occurred %x-%x\r\n",(uint32_t)(end_time), (uint32_t)present_time);

    #ifdef PROFILE_PMU_ENTRY
    uint64_t value;
    asm volatile ("lw %0, 0(%1)" : "=r" (value) : "r" (&pmu_v1_global.counters[0]));
    pmu_entry_saved[pmu_entry_saved_counter++] = value;
    // printk("interrupt occurred %d\r\n",(uint32_t)(value));
    if(pmu_entry_saved_counter == 32){
      pmu_entry_saved_counter = 0;
      printk("Entry profiled data: \r\n");
      for (int i = 0; i < 32; i++){
        printk("%d, ", pmu_entry_saved[i]);
      }
      printk("\r\n");
    }
    #endif
    
    #ifdef PROFILE_PMU_EXIT
    if(pmu_exit_saved_counter == 32){
      pmu_exit_saved_counter = 0;
      printk("Exit profiled data: \r\n");
      for (int i = 0; i < 32; i++){
        printk("%d, ", pmu_exit_saved[i]);
      }
      printk("\r\n");
    }
    #endif

    
    while(present_time < end_time){
      present_time = CSRR(CSR_CYCLE);
    }

    //asm volatile ("lw %0, 0(%1)" : "=r" (value) : "r" (&pmu_v1_global.counters[0]));
    //printk("interrupt occurred %d\r\n",(uint32_t)(value));
}







