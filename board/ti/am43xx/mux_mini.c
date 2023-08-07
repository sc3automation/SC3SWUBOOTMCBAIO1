// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Texas Instruments Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mux.h>
#include "board_mini.h"

static struct module_pin_mux uart0_pin_mux[] = {
	{OFFSET(uart0_rxd), (MODE(0) | PULLUP_EN | RXACTIVE | SLEWCTRL)},
	{OFFSET(uart0_txd), (MODE(0) | PULLUDDIS | PULLUP_EN | SLEWCTRL)},
	{-1},
};

static struct module_pin_mux mmc0_pin_mux[] = {
	{OFFSET(mmc0_clk), (MODE(0) | PULLUDDIS | RXACTIVE)},  /* MMC0_CLK */
	{OFFSET(mmc0_cmd), (MODE(0) | PULLUP_EN | RXACTIVE)},  /* MMC0_CMD */
	{OFFSET(mmc0_dat0), (MODE(0) | PULLUP_EN | RXACTIVE)}, /* MMC0_DAT0 */
	{OFFSET(mmc0_dat1), (MODE(0) | PULLUP_EN | RXACTIVE)}, /* MMC0_DAT1 */
	{OFFSET(mmc0_dat2), (MODE(0) | PULLUP_EN | RXACTIVE)}, /* MMC0_DAT2 */
	{OFFSET(mmc0_dat3), (MODE(0) | PULLUP_EN | RXACTIVE)}, /* MMC0_DAT3 */
	{-1},
};

static struct module_pin_mux mmc1_pin_mux[] = {
    {OFFSET(gpmc_csn1), (MODE(2) | PULLUDDIS | RXACTIVE)},
    {OFFSET(gpmc_csn2), (MODE(2) | PULLUP_EN | RXACTIVE)},
    {OFFSET(gpmc_ad8),  (MODE(2) | PULLUP_EN | RXACTIVE)}, /* mmc1_dat0 */
    {OFFSET(gpmc_ad9),  (MODE(2) | PULLUP_EN | RXACTIVE)}, /* mmc1_dat1 */
    {OFFSET(gpmc_ad10), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* mmc1_dat2 */
    {OFFSET(gpmc_ad11), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* mmc1_dat3 */
    {OFFSET(gpmc_ad12), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* mmc1_dat4 */
    {OFFSET(gpmc_ad13), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* mmc1_dat5 */
    {OFFSET(gpmc_ad14), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* mmc1_dat6 */
    {OFFSET(gpmc_ad15), (MODE(2) | PULLUP_EN | RXACTIVE)}, /* mmc1_dat7 */
    {-1},
};

static struct module_pin_mux i2c0_pin_mux[] = {
	{OFFSET(i2c0_sda), (MODE(0) | PULLUP_EN | RXACTIVE | SLEWCTRL)},
	{OFFSET(i2c0_scl), (MODE(0) | PULLUP_EN | RXACTIVE | SLEWCTRL)},
	{-1},
};

static struct module_pin_mux gpio5_7_pin_mux[] = {
	{OFFSET(spi0_cs0), (MODE(7) | PULLUP_EN)}, /* GPIO5_7 */
	{-1},
};

void enable_uart0_pin_mux(void)
{
	configure_module_pin_mux(uart0_pin_mux);
}

void enable_board_pin_mux(void)
{
	configure_module_pin_mux(mmc0_pin_mux);
	configure_module_pin_mux(mmc1_pin_mux);
	configure_module_pin_mux(i2c0_pin_mux);
	configure_module_pin_mux(gpio5_7_pin_mux); /* DDR VTT Enable */
}

void enable_i2c0_pin_mux(void)
{
	configure_module_pin_mux(i2c0_pin_mux);
}
