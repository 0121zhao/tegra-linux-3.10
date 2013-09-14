/*
 *
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *  Copyright (c) 2010, 2012 NVIDIA Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/smp.h>
#include <linux/clk/tegra.h>
#include <linux/cpu_pm.h>

#include <asm/cacheflush.h>
#include <asm/smp_plat.h>

#include "gic.h"
#include "sleep.h"

static void (*tegra_hotplug_shutdown)(void);

int tegra_cpu_kill(unsigned int cpu)
{
	cpu = cpu_logical_map(cpu);

	tegra_wait_cpu_in_reset(cpu);

	tegra_disable_cpu_clock(cpu);

	return 1;
}

/*
 * platform-specific code to shutdown a CPU
 *
 * Called with IRQs disabled
 */
void tegra_cpu_die(unsigned int cpu)
{
	cpu = cpu_logical_map(cpu);

	/* Flush the L1 data cache. */
	flush_cache_all();

#ifndef CONFIG_ARCH_TEGRA_2x_SOC
	/* Disable GIC CPU interface for this CPU. */
	tegra_gic_cpu_disable();

	/* Tegra3 enters LPx states via WFI - do not propagate legacy IRQs
	   to CPU core to avoid fall through WFI; then GIC output will be
	   enabled, however at this time - CPU is dying - no interrupt should
	   have affinity to this CPU. */
	tegra_gic_pass_through_disable();
#endif

	/* Shut down the current CPU. */
	tegra_hotplug_shutdown();

	/* Clock gate the CPU */
	tegra_wait_cpu_in_reset(cpu);
	tegra_disable_cpu_clock(cpu);

	/* Should never return here. */
	BUG();
}

#ifdef CONFIG_ARCH_TEGRA_2x_SOC
extern void tegra20_hotplug_shutdown(void);
void __init tegra20_hotplug_init(void)
{
	tegra_hotplug_shutdown = tegra20_hotplug_shutdown;
}
#endif

#ifdef CONFIG_ARCH_TEGRA_3x_SOC
extern void tegra30_hotplug_shutdown(void);
void __init tegra30_hotplug_init(void)
{
	tegra_hotplug_shutdown = tegra30_hotplug_shutdown;
}
#endif
