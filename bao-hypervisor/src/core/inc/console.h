/**
 * Bao, a Lightweight Static Partitioning Hypervisor
 *
 * Copyright (c) Bao Project (www.bao-project.org), 2019-
 *
 * Authors:
 *      Jose Martins <jose.martins@bao-project.org>
 *
 * Bao is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation, with a special exception exempting guest code from such
 * license. See the COPYING file in the top-level directory for details.
 *
 */

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <bao.h>

typedef struct {
    uint32_t counters[8];
    uint32_t event_sel[4];
    uint32_t event_info[4];
    uint32_t init_budget[8];
    uint32_t period[2];
    uint32_t timer[2];
} __attribute__((__packed__, aligned(PAGE_SIZE))) pmu_v1_global_t;

extern volatile pmu_v1_global_t pmu_v1_global
    __attribute__((section(".devices")));

void console_init();
void console_write(char const* const str);

#endif /* __CONSOLE_H__ */