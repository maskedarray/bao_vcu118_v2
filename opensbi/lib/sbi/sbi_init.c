/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/riscv_asm.h>
#include <sbi/riscv_atomic.h>
#include <sbi/riscv_barrier.h>
#include <sbi/riscv_locks.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_domain.h>
#include <sbi/sbi_ecall.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_hartmask.h>
#include <sbi/sbi_hsm.h>
#include <sbi/sbi_ipi.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_pmu.h>
#include <sbi/sbi_system.h>
#include <sbi/sbi_string.h>
#include <sbi/sbi_timer.h>
#include <sbi/sbi_tlb.h>
#include <sbi/sbi_version.h>

#define BANNER                                              \
	"   ____                    _____ ____ _____\n"     \
	"  / __ \\                  / ____|  _ \\_   _|\n"  \
	" | |  | |_ __   ___ _ __ | (___ | |_) || |\n"      \
	" | |  | | '_ \\ / _ \\ '_ \\ \\___ \\|  _ < | |\n" \
	" | |__| | |_) |  __/ | | |____) | |_) || |_\n"     \
	"  \\____/| .__/ \\___|_| |_|_____/|____/_____|\n"  \
	"        | |\n"                                     \
	"        |_|\n\n"

static void sbi_boot_print_banner(struct sbi_scratch *scratch)
{
	if (scratch->options & SBI_SCRATCH_NO_BOOT_PRINTS)
		return;

#ifdef OPENSBI_VERSION_GIT
	sbi_printf("\nOpenSBI %s\n", OPENSBI_VERSION_GIT);
#else
	sbi_printf("\nOpenSBI v%d.%d\n", OPENSBI_VERSION_MAJOR,
		   OPENSBI_VERSION_MINOR);
#endif

#ifdef OPENSBI_BUILD_TIME_STAMP
	sbi_printf("Build time: %s\n", OPENSBI_BUILD_TIME_STAMP);
#endif

#ifdef OPENSBI_BUILD_COMPILER_VERSION
	sbi_printf("Build compiler: %s\n", OPENSBI_BUILD_COMPILER_VERSION);
#endif

	sbi_printf(BANNER);
}

static void sbi_boot_print_general(struct sbi_scratch *scratch)
{
	char str[128];
	const struct sbi_hsm_device *hdev;
	const struct sbi_ipi_device *idev;
	const struct sbi_timer_device *tdev;
	const struct sbi_console_device *cdev;
	const struct sbi_system_reset_device *srdev;
	const struct sbi_platform *plat = sbi_platform_ptr(scratch);

	if (scratch->options & SBI_SCRATCH_NO_BOOT_PRINTS)
		return;

	/* Platform details */
	sbi_printf("Platform Name             : %s\n",
		   sbi_platform_name(plat));
	sbi_platform_get_features_str(plat, str, sizeof(str));
	sbi_printf("Platform Features         : %s\n", str);
	sbi_printf("Platform HART Count       : %u\n",
		   sbi_platform_hart_count(plat));
	idev = sbi_ipi_get_device();
	sbi_printf("Platform IPI Device       : %s\n",
		   (idev) ? idev->name : "---");
	tdev = sbi_timer_get_device();
	sbi_printf("Platform Timer Device     : %s @ %luHz\n",
		   (tdev) ? tdev->name : "---",
		   (tdev) ? tdev->timer_freq : 0);
	cdev = sbi_console_get_device();
	sbi_printf("Platform Console Device   : %s\n",
		   (cdev) ? cdev->name : "---");
	hdev = sbi_hsm_get_device();
	sbi_printf("Platform HSM Device       : %s\n",
		   (hdev) ? hdev->name : "---");
	srdev = sbi_system_reset_get_device(SBI_SRST_RESET_TYPE_COLD_REBOOT, 0);
	sbi_printf("Platform Reboot Device    : %s\n",
		   (srdev) ? srdev->name : "---");
	srdev = sbi_system_reset_get_device(SBI_SRST_RESET_TYPE_SHUTDOWN, 0);
	sbi_printf("Platform Shutdown Device  : %s\n",
		   (srdev) ? srdev->name : "---");

	/* Firmware details */
	sbi_printf("Firmware Base             : 0x%lx\n", scratch->fw_start);
	sbi_printf("Firmware Size             : %d KB\n",
		   (u32)(scratch->fw_size / 1024));

	/* SBI details */
	sbi_printf("Runtime SBI Version       : %d.%d\n",
		   sbi_ecall_version_major(), sbi_ecall_version_minor());
	sbi_printf("\n");
}

static void sbi_boot_print_domains(struct sbi_scratch *scratch)
{
	if (scratch->options & SBI_SCRATCH_NO_BOOT_PRINTS)
		return;

	/* Domain details */
	sbi_domain_dump_all("      ");
}

static void sbi_boot_print_hart(struct sbi_scratch *scratch, u32 hartid)
{
	int xlen;
	char str[128];
	const struct sbi_domain *dom = sbi_domain_thishart_ptr();

	if (scratch->options & SBI_SCRATCH_NO_BOOT_PRINTS)
		return;

	/* Determine MISA XLEN and MISA string */
	xlen = misa_xlen();
	if (xlen < 1) {
		sbi_printf("Error %d getting MISA XLEN\n", xlen);
		sbi_hart_hang();
	}

	/* Boot HART details */
	sbi_printf("Boot HART ID              : %u\n", hartid);
	sbi_printf("Boot HART Domain          : %s\n", dom->name);
	misa_string(xlen, str, sizeof(str));
	sbi_printf("Boot HART ISA             : %s\n", str);
	sbi_hart_get_features_str(scratch, str, sizeof(str));
	sbi_printf("Boot HART Features        : %s\n", str);
	sbi_printf("Boot HART PMP Count       : %d\n",
		   sbi_hart_pmp_count(scratch));
	sbi_printf("Boot HART PMP Granularity : %lu\n",
		   sbi_hart_pmp_granularity(scratch));
	sbi_printf("Boot HART PMP Address Bits: %d\n",
		   sbi_hart_pmp_addrbits(scratch));
	sbi_printf("Boot HART MHPM Count      : %d\n",
		   sbi_hart_mhpm_count(scratch));
	sbi_hart_delegation_dump(scratch, "Boot HART ", "         ");
}

static spinlock_t coldboot_lock = SPIN_LOCK_INITIALIZER;
static struct sbi_hartmask coldboot_wait_hmask = { 0 };

static unsigned long coldboot_done;

static void wait_for_coldboot(struct sbi_scratch *scratch, u32 hartid)
{
	unsigned long saved_mie, cmip;

	/* Save MIE CSR */
	saved_mie = csr_read(CSR_MIE);

	/* Set MSIE bit to receive IPI */
	csr_set(CSR_MIE, MIP_MSIP);

	/* Acquire coldboot lock */
	spin_lock(&coldboot_lock);

	/* Mark current HART as waiting */
	sbi_hartmask_set_hart(hartid, &coldboot_wait_hmask);

	/* Release coldboot lock */
	spin_unlock(&coldboot_lock);

	/* Wait for coldboot to finish using WFI */
	while (!__smp_load_acquire(&coldboot_done)) {
		do {
			wfi();
			cmip = csr_read(CSR_MIP);
		 } while (!(cmip & MIP_MSIP));
	};

	/* Acquire coldboot lock */
	spin_lock(&coldboot_lock);

	/* Unmark current HART as waiting */
	sbi_hartmask_clear_hart(hartid, &coldboot_wait_hmask);

	/* Release coldboot lock */
	spin_unlock(&coldboot_lock);

	/* Restore MIE CSR */
	csr_write(CSR_MIE, saved_mie);

	/*
	 * The wait for coldboot is common for both warm startup and
	 * warm resume path so clearing IPI here would result in losing
	 * an IPI in warm resume path.
	 *
	 * Also, the sbi_platform_ipi_init() called from sbi_ipi_init()
	 * will automatically clear IPI for current HART.
	 */
}

static void wake_coldboot_harts(struct sbi_scratch *scratch, u32 hartid)
{
	/* Mark coldboot done */
	__smp_store_release(&coldboot_done, 1);

	/* Acquire coldboot lock */
	spin_lock(&coldboot_lock);

	/* Send an IPI to all HARTs waiting for coldboot */
	for (u32 i = 0; i <= sbi_scratch_last_hartid(); i++) {
		if ((i != hartid) &&
		    sbi_hartmask_test_hart(i, &coldboot_wait_hmask))
			sbi_ipi_raw_send(i);
	}

	/* Release coldboot lock */
	spin_unlock(&coldboot_lock);
}

static unsigned long init_count_offset;
///////////////////
/*
#define cfg_write(addr, val_)  (*(volatile unsigned int *)(long)(addr) = val_)
#define cfg_read(addr)         (*(volatile unsigned int *)(long)(addr))

#define NUM_COUNTER 4
#define TIMER_WIDTH 8
#define COUNTER_WIDTH 8
#define CONFIG_WIDTH 4
#define NUM_ELEMENT 10

void read_counters(int num_counter, uint64_t BASE_ADDR, int REG_SIZE_IN_BYTES);
void write_counters(int num_counter, uint64_t BASE_ADDR, int REG_SIZE_IN_BYTES, long long int val[]);
*/

static void __noreturn init_coldboot(struct sbi_scratch *scratch, u32 hartid)
{
	int rc;
	unsigned long *init_count;
	const struct sbi_platform *plat = sbi_platform_ptr(scratch);

	/* Note: This has to be first thing in coldboot init sequence */
	rc = sbi_scratch_init(scratch);
	if (rc)
		sbi_hart_hang();

	/* Note: This has to be second thing in coldboot init sequence */
	rc = sbi_domain_init(scratch, hartid);
	if (rc)
		sbi_hart_hang();

	init_count_offset = sbi_scratch_alloc_offset(__SIZEOF_POINTER__);
	if (!init_count_offset)
		sbi_hart_hang();

	rc = sbi_hsm_init(scratch, hartid, TRUE);
	if (rc)
		sbi_hart_hang();

	rc = sbi_platform_early_init(plat, TRUE);
	if (rc)
		sbi_hart_hang();

	rc = sbi_hart_init(scratch, TRUE);
	if (rc)
		sbi_hart_hang();

	rc = sbi_console_init(scratch);
	if (rc)
		sbi_hart_hang();

	rc = sbi_pmu_init(scratch, TRUE);
	if (rc)
		sbi_hart_hang();

	sbi_boot_print_banner(scratch);

	rc = sbi_platform_irqchip_init(plat, TRUE);
	if (rc) {
		sbi_printf("%s: platform irqchip init failed (error %d)\n",
			   __func__, rc);
		sbi_hart_hang();
	}

	rc = sbi_ipi_init(scratch, TRUE);
	if (rc) {
		sbi_printf("%s: ipi init failed (error %d)\n", __func__, rc);
		sbi_hart_hang();
	}

	rc = sbi_tlb_init(scratch, TRUE);
	if (rc) {
		sbi_printf("%s: tlb init failed (error %d)\n", __func__, rc);
		sbi_hart_hang();
	}

	rc = sbi_timer_init(scratch, TRUE);
	if (rc) {
		sbi_printf("%s: timer init failed (error %d)\n", __func__, rc);
		sbi_hart_hang();
	}

	rc = sbi_ecall_init();
	if (rc) {
		sbi_printf("%s: ecall init failed (error %d)\n", __func__, rc);
		sbi_hart_hang();
	}

	/*
	 * Note: Finalize domains after HSM initialization so that we
	 * can startup non-root domains.
	 * Note: Finalize domains before HART PMP configuration so
	 * that we use correct domain for configuring PMP.
	 */
	rc = sbi_domain_finalize(scratch, hartid);
	if (rc) {
		sbi_printf("%s: domain finalize failed (error %d)\n",
			   __func__, rc);
		sbi_hart_hang();
	}

	rc = sbi_hart_pmp_configure(scratch);
	if (rc) {
		sbi_printf("%s: PMP configure failed (error %d)\n",
			   __func__, rc);
		sbi_hart_hang();
	}

	/*
	 * Note: Platform final initialization should be last so that
	 * it sees correct domain assignment and PMP configuration.
	 */
	rc = sbi_platform_final_init(plat, TRUE);
	if (rc) {
		sbi_printf("%s: platform final init failed (error %d)\n",
			   __func__, rc);
		sbi_hart_hang();
	}

	sbi_boot_print_general(scratch);

	sbi_boot_print_domains(scratch);

	sbi_boot_print_hart(scratch, hartid);

	wake_coldboot_harts(scratch, hartid);

	init_count = sbi_scratch_offset_ptr(scratch, init_count_offset);
	(*init_count)++;


//////////////////////////
/*
  uint64_t PMU_COUNTER_BASE_ADDR      = 0x10404000;
  uint64_t PMU_EVENT_SEL_BASE_ADDR    = 0x10404000 + 1*NUM_COUNTER*8 + 0*NUM_COUNTER*4;
  uint64_t PMU_EVENT_INFO_BASE_ADDR   = 0x10404000 + 1*NUM_COUNTER*8 + 1*NUM_COUNTER*4;
  uint64_t PMU_INIT_BUDGET_BASE_ADDR  = 0x10404000 + 1*NUM_COUNTER*8 + 2*NUM_COUNTER*4;
  uint64_t PMU_PERIOD_REG_BASE_ADDR   = 0x10404000 + 2*NUM_COUNTER*8 + 2*NUM_COUNTER*4;
  uint64_t PMU_TIMER_BASE_ADDR        = 0x10404000 + 2*NUM_COUNTER*8 + 2*NUM_COUNTER*4 + 1*TIMER_WIDTH;

  long long int counter_val[]      = {0xfffffffffffffe00, 0x200, 0x300, 0x400};
  long long int event_sel_val[]    = {0x2F, 0x3F, 0x4F, 0x5F};
  long long int event_info_val[]   = {0xA00, 0xB00, 0xC00, 0xD00};
  //long long int init_budget_val[]  = {0xFFFFFFFFFFFFFFFE, 0xFFFFFA000, 0xFFFFFB000, 0xFFFFFC000};
  long long int init_budget_val[]  = {0xfffffffffffffe00, 0xFFFFFA000, 0xFFFFFB000, 0xFFFFFC000};
  long long int period_val[]       = {0xb100};


  sbi_printf("Hello PMU!\r\n");


  sbi_printf("Counter\r\n");
  write_counters(NUM_COUNTER, PMU_COUNTER_BASE_ADDR, COUNTER_WIDTH, counter_val);
  sbi_printf("EventSel Config\r\n");
  write_counters(NUM_COUNTER, PMU_EVENT_SEL_BASE_ADDR, CONFIG_WIDTH, event_sel_val);
  sbi_printf("EventInfo Config\r\n");
  write_counters(NUM_COUNTER, PMU_EVENT_INFO_BASE_ADDR, CONFIG_WIDTH, event_info_val);
  sbi_printf("Initital Budget\r\n");
  write_counters(NUM_COUNTER, PMU_INIT_BUDGET_BASE_ADDR, COUNTER_WIDTH, init_budget_val);
  sbi_printf("Period Register\r\n");
  write_counters(1, PMU_PERIOD_REG_BASE_ADDR, COUNTER_WIDTH, period_val);

  sbi_printf("Counters initialized!\r\n");


  volatile uint32_t comp_array[NUM_ELEMENT] = {0};
  for (int i=0; i<NUM_ELEMENT; i++) {
    comp_array[i] = comp_array[i] + i;
  }

  sbi_printf("Array traversed!\r\n");



  sbi_printf("Counter\r\n");
  read_counters(NUM_COUNTER, PMU_COUNTER_BASE_ADDR, COUNTER_WIDTH);
  sbi_printf("EventSel Config\r\n");
  read_counters(NUM_COUNTER, PMU_EVENT_SEL_BASE_ADDR, CONFIG_WIDTH);
  sbi_printf("EventInfo Config\r\n");
  read_counters(NUM_COUNTER, PMU_EVENT_INFO_BASE_ADDR, CONFIG_WIDTH);
  sbi_printf("Initital Budget\r\n");
  read_counters(NUM_COUNTER, PMU_INIT_BUDGET_BASE_ADDR, COUNTER_WIDTH);
  sbi_printf("Period Register\r\n");
  read_counters(1, PMU_PERIOD_REG_BASE_ADDR, TIMER_WIDTH);
  sbi_printf("Timer Register\r\n");
  read_counters(1, PMU_TIMER_BASE_ADDR, TIMER_WIDTH);

  sbi_printf("The test is over!\r\n");



  for (int i=0; i<NUM_ELEMENT; i++) {
    comp_array[i] = comp_array[i] + i;
  }

  sbi_printf("Array traversed!\r\n");



  sbi_printf("Counter\r\n");
  read_counters(NUM_COUNTER, PMU_COUNTER_BASE_ADDR, COUNTER_WIDTH);
  sbi_printf("EventSel Config\r\n");
  read_counters(NUM_COUNTER, PMU_EVENT_SEL_BASE_ADDR, CONFIG_WIDTH);
  sbi_printf("EventInfo Config\r\n");
  read_counters(NUM_COUNTER, PMU_EVENT_INFO_BASE_ADDR, CONFIG_WIDTH);
  sbi_printf("Initital Budget\r\n");
  read_counters(NUM_COUNTER, PMU_INIT_BUDGET_BASE_ADDR, COUNTER_WIDTH);
  sbi_printf("Period Register\r\n");
  read_counters(1, PMU_PERIOD_REG_BASE_ADDR, TIMER_WIDTH);
  sbi_printf("Timer Register\r\n");
  read_counters(1, PMU_TIMER_BASE_ADDR, TIMER_WIDTH);

  sbi_printf("The test is over!\r\n");

//////////////
*/




	sbi_hsm_prepare_next_jump(scratch, hartid);
	sbi_hart_switch_mode(hartid, scratch->next_arg1, scratch->next_addr,
			     scratch->next_mode, FALSE);
}

/*
void read_counters(int num_counter, uint64_t BASE_ADDR, int REG_SIZE_IN_BYTES) {
  // Make sure the read variable is 32bit.
  // The AXI4-Lite interconnect is 32bit.
  unsigned int val;
  for (int i=0; i<num_counter; i++) {
    val = *(long*)(BASE_ADDR);
    sbi_printf("Read: %x: %x\r\n", (unsigned int)BASE_ADDR, (unsigned int)val);

    BASE_ADDR += REG_SIZE_IN_BYTES;
  }
}

void write_counters(int num_counter, uint64_t BASE_ADDR, int REG_SIZE_IN_BYTES, long long int val[]) {
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
    sbi_printf("Write: %x: %x\r\n", (unsigned int)BASE_ADDR, (unsigned int)val[i]);

    BASE_ADDR += REG_SIZE_IN_BYTES;
  }
}
*/

static void init_warm_startup(struct sbi_scratch *scratch, u32 hartid)
{
	int rc;
	unsigned long *init_count;
	const struct sbi_platform *plat = sbi_platform_ptr(scratch);

	if (!init_count_offset)
		sbi_hart_hang();

	rc = sbi_hsm_init(scratch, hartid, FALSE);
	if (rc)
		sbi_hart_hang();

	rc = sbi_platform_early_init(plat, FALSE);
	if (rc)
		sbi_hart_hang();

	rc = sbi_hart_init(scratch, FALSE);
	if (rc)
		sbi_hart_hang();

	rc = sbi_pmu_init(scratch, FALSE);
	if (rc)
		sbi_hart_hang();

	rc = sbi_platform_irqchip_init(plat, FALSE);
	if (rc)
		sbi_hart_hang();

	rc = sbi_ipi_init(scratch, FALSE);
	if (rc)
		sbi_hart_hang();

	rc = sbi_tlb_init(scratch, FALSE);
	if (rc)
		sbi_hart_hang();

	rc = sbi_timer_init(scratch, FALSE);
	if (rc)
		sbi_hart_hang();

	rc = sbi_hart_pmp_configure(scratch);
	if (rc)
		sbi_hart_hang();

	rc = sbi_platform_final_init(plat, FALSE);
	if (rc)
		sbi_hart_hang();

	init_count = sbi_scratch_offset_ptr(scratch, init_count_offset);
	(*init_count)++;

	sbi_hsm_prepare_next_jump(scratch, hartid);
}

static void init_warm_resume(struct sbi_scratch *scratch)
{
	int rc;

	sbi_hsm_hart_resume_start(scratch);

	rc = sbi_hart_reinit(scratch);
	if (rc)
		sbi_hart_hang();

	rc = sbi_hart_pmp_configure(scratch);
	if (rc)
		sbi_hart_hang();

	sbi_hsm_hart_resume_finish(scratch);
}

static void __noreturn init_warmboot(struct sbi_scratch *scratch, u32 hartid)
{
	int hstate;

	wait_for_coldboot(scratch, hartid);

	hstate = sbi_hsm_hart_get_state(sbi_domain_thishart_ptr(), hartid);
	if (hstate < 0)
		sbi_hart_hang();

	if (hstate == SBI_HSM_STATE_SUSPENDED)
		init_warm_resume(scratch);
	else
		init_warm_startup(scratch, hartid);

	sbi_hart_switch_mode(hartid, scratch->next_arg1,
			     scratch->next_addr,
			     scratch->next_mode, FALSE);
}

static atomic_t coldboot_lottery = ATOMIC_INITIALIZER(0);

/**
 * Initialize OpenSBI library for current HART and jump to next
 * booting stage.
 *
 * The function expects following:
 * 1. The 'mscratch' CSR is pointing to sbi_scratch of current HART
 * 2. Stack pointer (SP) is setup for current HART
 * 3. Interrupts are disabled in MSTATUS CSR
 * 4. All interrupts are disabled in MIE CSR
 *
 * @param scratch pointer to sbi_scratch of current HART
 */
void __noreturn sbi_init(struct sbi_scratch *scratch)
{
	bool next_mode_supported	= FALSE;
	bool coldboot			= FALSE;
	u32 hartid			= current_hartid();
	const struct sbi_platform *plat = sbi_platform_ptr(scratch);

	if ((SBI_HARTMASK_MAX_BITS <= hartid) ||
	    sbi_platform_hart_invalid(plat, hartid))
		sbi_hart_hang();

	switch (scratch->next_mode) {
	case PRV_M:
		next_mode_supported = TRUE;
		break;
	case PRV_S:
		if (misa_extension('S'))
			next_mode_supported = TRUE;
		break;
	case PRV_U:
		if (misa_extension('U'))
			next_mode_supported = TRUE;
		break;
	default:
		sbi_hart_hang();
	}

	/*
	 * Only the HART supporting privilege mode specified in the
	 * scratch->next_mode should be allowed to become the coldboot
	 * HART because the coldboot HART will be directly jumping to
	 * the next booting stage.
	 *
	 * We use a lottery mechanism to select coldboot HART among
	 * HARTs which satisfy above condition.
	 */

	if (next_mode_supported && atomic_xchg(&coldboot_lottery, 1) == 0)
		coldboot = TRUE;

	if (coldboot)
		init_coldboot(scratch, hartid);
	else
		init_warmboot(scratch, hartid);
}

unsigned long sbi_init_count(u32 hartid)
{
	struct sbi_scratch *scratch;
	unsigned long *init_count;

	if (!init_count_offset)
		return 0;

	scratch = sbi_hartid_to_scratch(hartid);
	if (!scratch)
		return 0;

	init_count = sbi_scratch_offset_ptr(scratch, init_count_offset);

	return *init_count;
}

/**
 * Exit OpenSBI library for current HART and stop HART
 *
 * The function expects following:
 * 1. The 'mscratch' CSR is pointing to sbi_scratch of current HART
 * 2. Stack pointer (SP) is setup for current HART
 *
 * @param scratch pointer to sbi_scratch of current HART
 */
void __noreturn sbi_exit(struct sbi_scratch *scratch)
{
	u32 hartid			= current_hartid();
	const struct sbi_platform *plat = sbi_platform_ptr(scratch);

	if (sbi_platform_hart_invalid(plat, hartid))
		sbi_hart_hang();

	sbi_platform_early_exit(plat);

	sbi_pmu_exit(scratch);

	sbi_timer_exit(scratch);

	sbi_ipi_exit(scratch);

	sbi_platform_irqchip_exit(plat);

	sbi_platform_final_exit(plat);

	sbi_hsm_exit(scratch);
}
