// SPDX-License-Identifier: GPL-2.0
/*
 * AM625: SoC specific initialization
 *
 * Copyright (C) 2020-2022 Texas Instruments Incorporated - https://www.ti.com/
 *	Suman Anna <s-anna@ti.com>
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sysfw-loader.h>
#include "common.h"
#include <dm.h>
#include <dm/uclass-internal.h>
#include <dm/pinctrl.h>

#if defined(CONFIG_SPL_BUILD)

/*
 * This uninitialized global variable would normal end up in the .bss section,
 * but the .bss is cleared between writing and reading this variable, so move
 * it to the .data section.
 */
u32 bootindex __section(".data");
static struct rom_extended_boot_data bootdata __section(".data");

static void store_boot_info_from_rom(void)
{
	bootindex = *(u32 *)(CONFIG_SYS_K3_BOOT_PARAM_TABLE_INDEX);
	memcpy(&bootdata, (uintptr_t *)ROM_ENTENDED_BOOT_DATA_INFO,
	       sizeof(struct rom_extended_boot_data));
}

static void ctrl_mmr_unlock(void)
{
	/* Unlock all WKUP_CTRL_MMR0 module registers */
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 0);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 1);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 2);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 3);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 4);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 5);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 6);
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 7);

	/* Unlock all CTRL_MMR0 module registers */
	mmr_unlock(CTRL_MMR0_BASE, 0);
	mmr_unlock(CTRL_MMR0_BASE, 1);
	mmr_unlock(CTRL_MMR0_BASE, 2);
	mmr_unlock(CTRL_MMR0_BASE, 4);
	mmr_unlock(CTRL_MMR0_BASE, 6);

	/* Unlock all MCU_CTRL_MMR0 module registers */
	mmr_unlock(MCU_CTRL_MMR0_BASE, 0);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 1);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 2);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 3);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 4);
	mmr_unlock(MCU_CTRL_MMR0_BASE, 6);

	/* Unlock PADCFG_CTRL_MMR padconf registers */
	mmr_unlock(PADCFG_MMR0_BASE, 1);
	mmr_unlock(PADCFG_MMR1_BASE, 1);
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

#if defined(CONFIG_CPU_V7R)
	setup_k3_mpu_regions();
#endif

	/*
	 * Cannot delay this further as there is a chance that
	 * K3_BOOT_PARAM_TABLE_INDEX can be over written by SPL MALLOC section.
	 */
	store_boot_info_from_rom();

	ctrl_mmr_unlock();

	/* Init DM early */
	spl_early_init();

	/*
	 * Process pinctrl for the serial0 and serial3, aka WKUP_UART0 and
	 * MAIN_UART1 modules and continue regardless of the result of pinctrl.
	 * Do this without probing the device, but instead by searching the
	 * device that would request the given sequence number if probed. The
	 * UARTs will be used by the DM firmware and TIFS firmware images
	 * respectively and the firmware depend on SPL to initialize the pin
	 * settings.
	 */
	ret = uclass_find_device_by_seq(UCLASS_SERIAL, 0, true, &dev);
	if (!ret)
		pinctrl_select_state(dev, "default");

	ret = uclass_find_device_by_seq(UCLASS_SERIAL, 3, true, &dev);
	if (!ret)
		pinctrl_select_state(dev, "default");

	preloader_console_init();

#ifdef CONFIG_K3_EARLY_CONS
	/*
	 * Allow establishing an early console as required for example when
	 * doing a UART-based boot. Note that this console may not "survive"
	 * through a SYSFW PM-init step and will need a re-init in some way
	 * due to changing module clock frequencies.
	 */
	early_console_init();
#endif

#if defined(CONFIG_K3_LOAD_SYSFW)
	/*
	 * Configure and start up system controller firmware. Provide
	 * the U-Boot console init function to the SYSFW post-PM configuration
	 * callback hook, effectively switching on (or over) the console
	 * output.
	 */
	ret = is_rom_loaded_sysfw(&bootdata);
	if (!ret)
		panic("ROM has not loaded TIFS firmware\n");

	k3_sysfw_loader(true, NULL, NULL);
#endif

	/*
	 * Force probe of clk_k3 driver here to ensure basic default clock
	 * configuration is always done.
	 */
	if (IS_ENABLED(CONFIG_SPL_CLK_K3)) {
		ret = uclass_get_device_by_driver(UCLASS_CLK,
						  DM_GET_DRIVER(ti_clk),
						  &dev);
		if (ret)
			printf("Failed to initialize clk-k3!\n");
	}

	/* Output System Firmware version info */
	k3_sysfw_print_ver();

#if defined(CONFIG_K3_AM64_DDRSS)
	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret)
		panic("DRAM init failed: %d\n", ret);
#endif
}

u32 spl_boot_mode(const u32 boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		return MMCSD_MODE_EMMCBOOT;

	case BOOT_DEVICE_MMC2:
		return MMCSD_MODE_FS;

	default:
		return MMCSD_MODE_RAW;
	}
}

static u32 __get_backup_bootmedia(u32 devstat)
{
	u32 bkup_bootmode = (devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_MASK) >>
				MAIN_DEVSTAT_BACKUP_BOOTMODE_SHIFT;
	u32 bkup_bootmode_cfg =
			(devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_MASK) >>
				MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_SHIFT;

	switch (bkup_bootmode) {
	case BACKUP_BOOT_DEVICE_UART:
		return BOOT_DEVICE_UART;

	case BACKUP_BOOT_DEVICE_USB:
		return BOOT_DEVICE_USB;

	case BACKUP_BOOT_DEVICE_ETHERNET:
		return BOOT_DEVICE_ETHERNET;

	case BACKUP_BOOT_DEVICE_MMC:
		if (bkup_bootmode_cfg)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BACKUP_BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BACKUP_BOOT_DEVICE_I2C:
		return BOOT_DEVICE_I2C;
	};

	return BOOT_DEVICE_RAM;
}

static u32 __get_primary_bootmedia(u32 devstat)
{
	u32 bootmode = (devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
				MAIN_DEVSTAT_PRIMARY_BOOTMODE_SHIFT;
	u32 bootmode_cfg = (devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_MASK) >>
				MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_SHIFT;

	switch (bootmode) {
	case BOOT_DEVICE_OSPI:
		fallthrough;
	case BOOT_DEVICE_QSPI:
		fallthrough;
	case BOOT_DEVICE_XSPI:
		fallthrough;
	case BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BOOT_DEVICE_ETHERNET_RGMII:
		fallthrough;
	case BOOT_DEVICE_ETHERNET_RMII:
		return BOOT_DEVICE_ETHERNET;

	case BOOT_DEVICE_EMMC:
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_MMC:
		if ((bootmode_cfg & MAIN_DEVSTAT_PRIMARY_MMC_PORT_MASK) >>
				MAIN_DEVSTAT_PRIMARY_MMC_PORT_SHIFT)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_NOBOOT:
		return BOOT_DEVICE_RAM;
	}

	return bootmode;
}

u32 spl_boot_device(void)
{
	u32 devstat = readl(CTRLMMR_MAIN_DEVSTAT);
	u32 bootmedia;

	if (bootindex == K3_PRIMARY_BOOTMODE)
		bootmedia = __get_primary_bootmedia(devstat);
	else
		bootmedia = __get_backup_bootmedia(devstat);

	debug("am625_init: %s: devstat = 0x%x bootmedia = 0x%x bootindex = %d\n",
	      __func__, devstat, bootmedia, bootindex);

	return bootmedia;
}

#endif /* CONFIG_SPL_BUILD */
