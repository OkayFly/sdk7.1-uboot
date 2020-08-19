/*
 * (C) Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd
 * Peter, Software Engineering, <superpeter.cai@gmail.com>.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/rkplat.h>

#ifndef BIT
#define BIT(n)			(1 << (n))
#endif

#define RK_CHARGE_TIMER_BASE	(RKIO_TIMER_BASE + 0x20)
#define RK_CHARGE_TIMER_IRQ	RKIRQ_TIMER1

#if defined(CONFIG_RKCHIP_RK3399)
#define TIMER_CTRL		0x1c
#else
#define TIMER_CTRL		0x10
#endif

#define TIMER_LOAD_COUNT0	0x00
#define TIMER_LOAD_COUNT1	0x04
#define TIMER_INTSTATUS		0x18
#define TIMER_EN		BIT(0)
#define TIMER_INT_EN		BIT(2)
#define TIMER_CLR_INT		BIT(0)

static int volatile irq_exit;
static ulong seconds;

static void timer_irq_handler(void *data)
{
	int period;

	writel(TIMER_CLR_INT, RK_CHARGE_TIMER_BASE + TIMER_INTSTATUS);
	irq_handler_disable(RK_CHARGE_TIMER_IRQ);
	period = get_timer(seconds);
	printf("    Hello, this is timer isr, period=%dms\n", period);

	irq_exit = 1;
}

static int timer_interrupt_test(void)
{
	printf("Timer interrupt:\n");

	/* Disable before config */
	writel(0, RK_CHARGE_TIMER_BASE + TIMER_CTRL);

	/* Configure 1s */
	writel(24000000, RK_CHARGE_TIMER_BASE + TIMER_LOAD_COUNT0);
	writel(0, RK_CHARGE_TIMER_BASE + TIMER_LOAD_COUNT1);
	writel(TIMER_CLR_INT, RK_CHARGE_TIMER_BASE + TIMER_INTSTATUS);
	writel(TIMER_EN | TIMER_INT_EN, RK_CHARGE_TIMER_BASE + TIMER_CTRL);

	/* Request irq */
	irq_install_handler(RK_CHARGE_TIMER_IRQ, timer_irq_handler, NULL);
	irq_handler_enable(RK_CHARGE_TIMER_IRQ);

	seconds = get_timer(0);
	while (!irq_exit)
		;

	irq_uninstall_handler(RK_CHARGE_TIMER_IRQ);

	return 0;
}

static int do_timer_irq(cmd_tbl_t *cmdtp, int flag,
			int argc, char * const argv[])
{
	return timer_interrupt_test();
}

U_BOOT_CMD(timer, 1, 1, do_timer_irq, "timer irq test", "");
