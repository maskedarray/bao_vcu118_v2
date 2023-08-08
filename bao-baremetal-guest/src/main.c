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

#include <core.h>
#include <stdlib.h>
#include <stdio.h>
#include <cpu.h>
#include <wfi.h>
#include <spinlock.h>
#include <plat.h>
#include <irq.h>
#include <uart.h>
#include <timer.h>

#define TIMER_INTERVAL (TIME_S(50))

spinlock_t print_lock = SPINLOCK_INITVAL;

void uart_rx_handler(){
    printf("cpu%d: %s\n",get_cpuid(), __func__);
    uart_clear_rxirq();
}

void ipi_handler(){
    printf("cpu%d: %s\n", get_cpuid(), __func__);
    irq_send_ipi(1ull << (get_cpuid() + 1));
}

void timer_handler(){
    printf("cpu%d: %s\n", get_cpuid(), __func__);
    timer_set(TIMER_INTERVAL);
    irq_send_ipi(1ull << (get_cpuid() + 1));
}

void pmu_handler(){
    printf("cpu%d: %s\n",get_cpuid(), __func__);
    //uart_clear_rxirq();
}

#define cfg_write(addr, val_)  (*(volatile unsigned int *)(long)(addr) = val_)
#define cfg_read(addr)         (*(volatile unsigned int *)(long)(addr))

#define NUM_COUNTER 4
#define TIMER_WIDTH 8
#define COUNTER_WIDTH 8
#define CONFIG_WIDTH 4
#define NUM_ELEMENT 1000
#define PMU_IRQ_ID 143

void read_counters(int num_counter, long long unsigned int BASE_ADDR, int REG_SIZE_IN_BYTES) {
  // Make sure the read variable is 32bit.
  // The AXI4-Lite interconnect is 32bit.
  unsigned int val;
  for (int i=0; i<num_counter; i++) {
    val = *(long*)(BASE_ADDR);
    printf("Read: %x: %x\n", BASE_ADDR, val);
   
    BASE_ADDR += REG_SIZE_IN_BYTES;
  }
}

void write_counters(int num_counter, long long unsigned int BASE_ADDR, int REG_SIZE_IN_BYTES, long long val[]) {
  // The AXI4-Lite interconnect is 32bit.
  // Can only write to 32bits at a time.
  for (int i=0; i<num_counter; i++) {
    if (REG_SIZE_IN_BYTES==4) {
      cfg_write(BASE_ADDR, val[i]);
    } else if (REG_SIZE_IN_BYTES==8) {
      int val_l = val[i] & 0xFFFFFFFF;
      int val_h = val[i] >> 32;

      cfg_write(BASE_ADDR, val_l);
      cfg_write(BASE_ADDR+4, val_h);
    }
    printf("Write: %x: %x\n", BASE_ADDR, val[i]);
   
    BASE_ADDR += REG_SIZE_IN_BYTES;
  }
}
void pmu_setup(){
    long long unsigned int PMU_COUNTER_BASE_ADDR      = 0x10404000;
    long long unsigned int PMU_EVENT_SEL_BASE_ADDR    = 0x10404000 + 1*NUM_COUNTER*8 + 0*NUM_COUNTER*4;
    long long unsigned int PMU_EVENT_INFO_BASE_ADDR   = 0x10404000 + 1*NUM_COUNTER*8 + 1*NUM_COUNTER*4;
    long long unsigned int PMU_INIT_BUDGET_BASE_ADDR  = 0x10404000 + 1*NUM_COUNTER*8 + 2*NUM_COUNTER*4;
    long long unsigned int PMU_PERIOD_REG_BASE_ADDR   = 0x10404000 + 2*NUM_COUNTER*8 + 2*NUM_COUNTER*4;
    long long unsigned int PMU_TIMER_BASE_ADDR        = 0x10404000 + 2*NUM_COUNTER*8 + 2*NUM_COUNTER*4 + 1*TIMER_WIDTH;

    long long int counter_val[]      = {0x100, 0x200, 0x300, 0x400};
    long long int event_sel_val[]    = {0x3F, 0x2F, 0x4F, 0x5F};
    long long int event_info_val[]   = {0xB00, 0xA00, 0xC00, 0xD00};
    long long int init_budget_val[]  = {0xFFFFFFFFFFFFFFFE, 0xFFFFFA000, 0xFFFFFB000, 0xFFFFFC000};
    long long int period_val[]       = {0x100};
    long long int timer;

      printf("Hello PMU!\n");
      

      printf("Counter\n");
      write_counters(NUM_COUNTER, PMU_COUNTER_BASE_ADDR, COUNTER_WIDTH, counter_val);
      printf("EventSel Config\n");
      write_counters(NUM_COUNTER, PMU_EVENT_SEL_BASE_ADDR, CONFIG_WIDTH, event_sel_val);
      printf("EventInfo Config\n");
      write_counters(NUM_COUNTER, PMU_EVENT_INFO_BASE_ADDR, CONFIG_WIDTH, event_info_val);
      printf("Initital Budget\n");
      write_counters(NUM_COUNTER, PMU_INIT_BUDGET_BASE_ADDR, COUNTER_WIDTH, init_budget_val);
      printf("Period Register\n");
      write_counters(1, PMU_PERIOD_REG_BASE_ADDR, COUNTER_WIDTH, period_val);

      printf("Counters initialized!\n");
      

      volatile uint32_t comp_array[NUM_ELEMENT] = {0};
      for (int i=0; i<NUM_ELEMENT; i++) {
        comp_array[i] = comp_array[i] + i;
      }

      printf("Array traversed!\n");
      
      unsigned int val_64;

      printf("Counter\n");
      read_counters(NUM_COUNTER, PMU_COUNTER_BASE_ADDR, COUNTER_WIDTH);
      printf("EventSel Config\n");
      read_counters(NUM_COUNTER, PMU_EVENT_SEL_BASE_ADDR, CONFIG_WIDTH);
      printf("EventInfo Config\n");
      read_counters(NUM_COUNTER, PMU_EVENT_INFO_BASE_ADDR, CONFIG_WIDTH);
      printf("Initital Budget\n");
      read_counters(NUM_COUNTER, PMU_INIT_BUDGET_BASE_ADDR, COUNTER_WIDTH);
      printf("Period Register\n");
      read_counters(1, PMU_PERIOD_REG_BASE_ADDR, TIMER_WIDTH);
      printf("Timer Register\n");
      read_counters(1, PMU_TIMER_BASE_ADDR, TIMER_WIDTH);

      printf("The test is over!\n");
}


void main(void){

    static volatile bool master_done = false;

    if(cpu_is_master()){
        spin_lock(&print_lock);
        printf("Bao bare-metal test guest\n");
        spin_unlock(&print_lock);

        //irq_set_handler(UART_IRQ_ID, uart_rx_handler);
        irq_set_handler(TIMER_IRQ_ID, timer_handler);
        irq_set_handler(IPI_IRQ_ID, ipi_handler);
        //irq_set_handler(PMU_IRQ_ID, pmu_handler);

        uart_enable_rxirq();

        //timer_set(TIMER_INTERVAL);
        //irq_enable(TIMER_IRQ_ID);
        //irq_set_prio(TIMER_IRQ_ID, IRQ_MAX_PRIO);
        //irq_enable(PMU_IRQ_ID);
        //irq_set_prio(PMU_IRQ_ID, IRQ_MAX_PRIO);

        master_done = true;
    }

    //irq_enable(UART_IRQ_ID);
    //irq_set_prio(UART_IRQ_ID, IRQ_MAX_PRIO);
    irq_enable(IPI_IRQ_ID);
    irq_set_prio(IPI_IRQ_ID, IRQ_MAX_PRIO);

    while(!master_done);

    //pmu_setup();

    spin_lock(&print_lock);
    printf("cpu %d up\n", get_cpuid());
    spin_unlock(&print_lock);

    volatile uint32_t comp_array[NUM_ELEMENT] = {0};
    for (int j = 0; j<1000; j++){
      for (int i=0; i<NUM_ELEMENT; i++) {
        comp_array[i] = comp_array[i] + i;
      }
      printf("Bao bare-metal checkpoint!\n");
    }

    while(1) wfi();
}