/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <misc/printk.h>
#include <shell/shell.h>
#include <init.h>
#include <debug/object_tracing.h>
#include <misc/reboot.h>
#include <misc/stack.h>
#include <string.h>

static void cmd_kernel_version(const struct shell *shell,
			      size_t argc, char **argv)
{
	u32_t version = sys_kernel_version_get();

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_fprintf(shell, SHELL_NORMAL, "Zephyr version %d.%d.%d\r\n",
		      SYS_KERNEL_VER_MAJOR(version),
		      SYS_KERNEL_VER_MINOR(version),
		      SYS_KERNEL_VER_PATCHLEVEL(version));
}

static void cmd_kernel_uptime(const struct shell *shell,
			      size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_fprintf(shell, SHELL_NORMAL, "Uptime: %u ms\r\n",
			k_uptime_get_32());
}

static void cmd_kernel_cycles(const struct shell *shell,
			      size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_fprintf(shell, SHELL_NORMAL, "cycles: %u hw cycles\r\n",
			k_cycle_get_32());
}

#if defined(CONFIG_OBJECT_TRACING) && defined(CONFIG_THREAD_MONITOR)
static void shell_tdata_dump(const struct k_thread *thread, void *user_data)
{
	shell_fprintf((const struct shell *)user_data, SHELL_NORMAL,
		      "%s%p:   options: 0x%x priority: %d\r\n",
		      (thread == k_current_get()) ? "*" : " ",
		      thread,
		      thread->base.user_options,
		      thread->base.prio);
}

static void cmd_kernel_threads(const struct shell *shell,
			      size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_fprintf(shell, SHELL_NORMAL, "Threads:\r\n");
	k_thread_foreach(shell_tdata_dump, (void *)shell);
}
#endif


#if defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_MONITOR) \
				&& defined(CONFIG_THREAD_STACK_INFO)
static void shell_stack_dump(const struct k_thread *thread, void *user_data)
{
	unsigned int pcnt, unused = 0;
	unsigned int size = thread->stack_info.size;

	unused = stack_unused_space_get((char *)thread->stack_info.start,
					size);

	/* Calculate the real size reserved for the stack */
	pcnt = ((size - unused) * 100) / size;

	shell_fprintf((const struct shell *)user_data, SHELL_NORMAL,
		"0x%08X (real size %u):\tunused %u\tusage %u / %u (%u %%)\r\n",
		      (u32_t)thread, size, unused, size - unused, size, pcnt);
}

static void cmd_kernel_stacks(const struct shell *shell,
			      size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	k_thread_foreach(shell_stack_dump, (void *)shell);
}
#endif

#if defined(CONFIG_REBOOT)
static void cmd_kernel_reboot_warm(const struct shell *shell,
				   size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	sys_reboot(SYS_REBOOT_WARM);
}

static void cmd_kernel_reboot_cold(const struct shell *shell,
				   size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	sys_reboot(SYS_REBOOT_COLD);
}

SHELL_CREATE_STATIC_SUBCMD_SET(sub_kernel_reboot)
{
	/* Alphabetically sorted. */
	SHELL_CMD(cold, NULL, "Cold reboot.", cmd_kernel_reboot_cold),
	SHELL_CMD(warm, NULL, "Warm reboot.", cmd_kernel_reboot_warm),
	SHELL_SUBCMD_SET_END /* Array terminated. */
};
#endif

SHELL_CREATE_STATIC_SUBCMD_SET(sub_kernel)
{
	/* Alphabetically sorted. */
	SHELL_CMD(cycles, NULL, "Kernel cycles.", cmd_kernel_cycles),
#if defined(CONFIG_REBOOT)
	SHELL_CMD(reboot, &sub_kernel_reboot, "Reboot.", NULL),
#endif
#if defined(CONFIG_INIT_STACKS) && defined(CONFIG_THREAD_MONITOR) \
				&& defined(CONFIG_THREAD_STACK_INFO)
	SHELL_CMD(stacks, NULL, "List threads stack usage.", cmd_kernel_stacks),
#endif
#if defined(CONFIG_OBJECT_TRACING) && defined(CONFIG_THREAD_MONITOR)
	SHELL_CMD(threads, NULL, "List kernel threads.", cmd_kernel_threads),
#endif
	SHELL_CMD(uptime, NULL, "Kernel uptime.", cmd_kernel_uptime),
	SHELL_CMD(version, NULL, "Kernel version.", cmd_kernel_version),
	SHELL_SUBCMD_SET_END /* Array terminated. */
};

SHELL_CMD_REGISTER(kernel, &sub_kernel, "Kernel commands", NULL);
