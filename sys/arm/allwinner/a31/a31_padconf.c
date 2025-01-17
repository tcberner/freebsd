/*-
 * Copyright (c) 2016 Emmanuel Vadot <manu@bidouilliste.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/types.h>

#include <arm/allwinner/allwinner_pinctrl.h>

const static struct allwinner_pins a31_pins[] = {
	{"PA0",  0, 0,  {"gpio_in", "gpio_out", "gmac", "lcd1", "uart1", NULL, NULL, NULL}},
	{"PA1",  0, 1,  {"gpio_in", "gpio_out", "gmac", "lcd1", "uart1", NULL, NULL, NULL}},
	{"PA2",  0, 2,  {"gpio_in", "gpio_out", "gmac", "lcd1", "uart1", NULL, NULL, NULL}},
	{"PA3",  0, 3,  {"gpio_in", "gpio_out", "gmac", "lcd1", "uart1", NULL, NULL, NULL}},
	{"PA4",  0, 4,  {"gpio_in", "gpio_out", "gmac", "lcd1", "uart1", NULL, NULL, NULL}},
	{"PA5",  0, 5,  {"gpio_in", "gpio_out", "gmac", "lcd1", "uart1", NULL, NULL, NULL}},
	{"PA6",  0, 6,  {"gpio_in", "gpio_out", "gmac", "lcd1", "uart1", NULL, NULL, NULL}},
	{"PA7",  0, 7,  {"gpio_in", "gpio_out", "gmac", "lcd1", "uart1", NULL, NULL, NULL}},
	{"PA8",  0, 8,  {"gpio_in", "gpio_out", "gmac", "lcd1", NULL, NULL, NULL, NULL}},
	{"PA9",  0, 9,  {"gpio_in", "gpio_out", "gmac", "lcd1", NULL, NULL, NULL, NULL}},
	{"PA10", 0, 10, {"gpio_in", "gpio_out", "gmac", "lcd1", "mmc3", "mmc2", NULL, NULL}},
	{"PA11", 0, 11, {"gpio_in", "gpio_out", "gmac", "lcd1", "mmc3", "mmc2", NULL, NULL}},
	{"PA12", 0, 12, {"gpio_in", "gpio_out", "gmac", "lcd1", "mmc3", "mmc2", NULL, NULL}},
	{"PA13", 0, 13, {"gpio_in", "gpio_out", "gmac", "lcd1", "mmc3", "mmc2", NULL, NULL}},
	{"PA14", 0, 14, {"gpio_in", "gpio_out", "gmac", "lcd1", "mmc3", "mmc2", NULL, NULL}},
	{"PA15", 0, 15, {"gpio_in", "gpio_out", "gmac", "lcd1", "clk_out_a", NULL, NULL, NULL}},
	{"PA16", 0, 16, {"gpio_in", "gpio_out", "gmac", "lcd1", "dmic", NULL, NULL, NULL}},
	{"PA17", 0, 17, {"gpio_in", "gpio_out", "gmac", "lcd1", "dmic", NULL, NULL, NULL}},
	{"PA18", 0, 18, {"gpio_in", "gpio_out", "gmac", "lcd1", "clk_out_b", NULL, NULL, NULL}},
	{"PA19", 0, 19, {"gpio_in", "gpio_out", "gmac", "lcd1", "pwm3", NULL, NULL, NULL}},
	{"PA20", 0, 20, {"gpio_in", "gpio_out", "gmac", "lcd1", "pwm3", NULL, NULL, NULL}},
	{"PA21", 0, 21, {"gpio_in", "gpio_out", "gmac", "lcd1", "spi3", NULL, NULL, NULL}},
	{"PA22", 0, 22, {"gpio_in", "gpio_out", "gmac", "lcd1", "spi3", NULL, NULL, NULL}},
	{"PA23", 0, 23, {"gpio_in", "gpio_out", "gmac", "lcd1", "spi3", NULL, NULL, NULL}},
	{"PA24", 0, 24, {"gpio_in", "gpio_out", "gmac", "lcd1", "spi3", NULL, NULL, NULL}},
	{"PA25", 0, 25, {"gpio_in", "gpio_out", "gmac", "lcd1", "spi3", NULL, NULL, NULL}},
	{"PA26", 0, 26, {"gpio_in", "gpio_out", "gmac", "lcd1", "clk_out_c", NULL, NULL, NULL}},
	{"PA27", 0, 27, {"gpio_in", "gpio_out", "gmac", "lcd1", NULL, NULL, NULL, NULL}},

	{"PB0",  1, 0,  {"gpio_in", "gpio_out", "i2s0", "uart3", "csi", NULL, NULL, NULL}},
	{"PB1",  1, 1,  {"gpio_in", "gpio_out", "i2s0", NULL, NULL, NULL, NULL, NULL}},
	{"PB2",  1, 2,  {"gpio_in", "gpio_out", "i2s0", NULL, NULL, NULL, NULL, NULL}},
	{"PB3",  1, 3,  {"gpio_in", "gpio_out", "i2s0", NULL, NULL, NULL, NULL, NULL}},
	{"PB4",  1, 4,  {"gpio_in", "gpio_out", "i2s0", "uart3", NULL, NULL, NULL, NULL}},
	{"PB5",  1, 5,  {"gpio_in", "gpio_out", "i2s0", "uart3", "i2c3", NULL, NULL, NULL}},
	{"PB6",  1, 6,  {"gpio_in", "gpio_out", "i2s0", "uart3", "i2c3", NULL, NULL, NULL}},
	{"PB7",  1, 7,  {"gpio_in", "gpio_out", "i2s0", NULL, NULL, NULL, NULL, NULL}},

	{"PC0",  2, 0,  {"gpio_in", "gpio_out", "nand0", "spi0", NULL, NULL, NULL, NULL}},
	{"PC1",  2, 1,  {"gpio_in", "gpio_out", "nand0", "spi0", NULL, NULL, NULL, NULL}},
	{"PC2",  2, 2,  {"gpio_in", "gpio_out", "nand0", "spi0", NULL, NULL, NULL, NULL}},
	{"PC3",  2, 3,  {"gpio_in", "gpio_out", "nand0", NULL, NULL, NULL, NULL, NULL}},
	{"PC4",  2, 4,  {"gpio_in", "gpio_out", "nand0", NULL, NULL, NULL, NULL, NULL}},
	{"PC5",  2, 5,  {"gpio_in", "gpio_out", "nand0", NULL, NULL, NULL, NULL, NULL}},
	{"PC6",  2, 6,  {"gpio_in", "gpio_out", "nand0", "mmc2", "mmc3", NULL, NULL, NULL}},
	{"PC7",  2, 7,  {"gpio_in", "gpio_out", "nand0", "mmc2", "mmc3", NULL, NULL, NULL}},
	{"PC8",  2, 8,  {"gpio_in", "gpio_out", "nand0", "mmc2", "mmc3", NULL, NULL, NULL}},
	{"PC9",  2, 9,  {"gpio_in", "gpio_out", "nand0", "mmc2", "mmc3", NULL, NULL, NULL}},
	{"PC10", 2, 10, {"gpio_in", "gpio_out", "nand0", "mmc2", "mmc3", NULL, NULL, NULL}},
	{"PC11", 2, 11, {"gpio_in", "gpio_out", "nand0", "mmc2", "mmc3", NULL, NULL, NULL}},
	{"PC12", 2, 12, {"gpio_in", "gpio_out", "nand0", "mmc2", "mmc3", NULL, NULL, NULL}},
	{"PC13", 2, 13, {"gpio_in", "gpio_out", "nand0", "mmc2", "mmc3", NULL, NULL, NULL}},
	{"PC14", 2, 14, {"gpio_in", "gpio_out", "nand0", "mmc2", "mmc3", NULL, NULL, NULL}},
	{"PC15", 2, 15, {"gpio_in", "gpio_out", "nand0", "mmc2", "mmc3", NULL, NULL, NULL}},
	{"PC16", 2, 16, {"gpio_in", "gpio_out", "nand0", "nand1", NULL, NULL, NULL, NULL}},
	{"PC17", 2, 17, {"gpio_in", "gpio_out", "nand0", "nand1", NULL, NULL, NULL, NULL}},
	{"PC18", 2, 18, {"gpio_in", "gpio_out", "nand0", "nand1", NULL, NULL, NULL, NULL}},
	{"PC19", 2, 19, {"gpio_in", "gpio_out", "nand0", "nand1", NULL, NULL, NULL, NULL}},
	{"PC20", 2, 20, {"gpio_in", "gpio_out", "nand0", "nand1", NULL, NULL, NULL, NULL}},
	{"PC21", 2, 21, {"gpio_in", "gpio_out", "nand0", "nand1", NULL, NULL, NULL, NULL}},
	{"PC22", 2, 22, {"gpio_in", "gpio_out", "nand0", "nand1", NULL, NULL, NULL, NULL}},
	{"PC23", 2, 23, {"gpio_in", "gpio_out", "nand0", "nand1", NULL, NULL, NULL, NULL}},
	{"PC24", 2, 24, {"gpio_in", "gpio_out", "nand0", "mmc2", "mmc3", NULL, NULL, NULL}},
	{"PC25", 2, 24, {"gpio_in", "gpio_out", "nand0", NULL, NULL, NULL, NULL, NULL}},
	{"PC26", 2, 24, {"gpio_in", "gpio_out", "nand0", NULL, NULL, NULL, NULL, NULL}},
	{"PC27", 2, 24, {"gpio_in", "gpio_out", NULL, "spi0",NULL, NULL, NULL, NULL}},

	{"PD0",  3, 0,  {"gpio_in", "gpio_out", "lcd0", "lvds0", NULL, NULL, NULL, NULL}},
	{"PD1",  3, 1,  {"gpio_in", "gpio_out", "lcd0", "lvds0", NULL, NULL, NULL, NULL}},
	{"PD2",  3, 2,  {"gpio_in", "gpio_out", "lcd0", "lvds0", NULL, NULL, NULL, NULL}},
	{"PD3",  3, 3,  {"gpio_in", "gpio_out", "lcd0", "lvds0", NULL, NULL, NULL, NULL}},
	{"PD4",  3, 4,  {"gpio_in", "gpio_out", "lcd0", "lvds0", NULL, NULL, NULL, NULL}},
	{"PD5",  3, 5,  {"gpio_in", "gpio_out", "lcd0", "lvds0", NULL, NULL, NULL, NULL}},
	{"PD6",  3, 6,  {"gpio_in", "gpio_out", "lcd0", "lvds0", NULL, NULL, NULL, NULL}},
	{"PD7",  3, 7,  {"gpio_in", "gpio_out", "lcd0", "lvds0", NULL, NULL, NULL, NULL}},
	{"PD8",  3, 8,  {"gpio_in", "gpio_out", "lcd0", "lvds0", NULL, NULL, NULL, NULL}},
	{"PD9",  3, 9,  {"gpio_in", "gpio_out", "lcd0", "lvds0", NULL, NULL, NULL, NULL}},
	{"PD10", 3, 10, {"gpio_in", "gpio_out", "lcd0", "lvds1", NULL, NULL, NULL, NULL}},
	{"PD11", 3, 11, {"gpio_in", "gpio_out", "lcd0", "lvds1", NULL, NULL, NULL, NULL}},
	{"PD12", 3, 12, {"gpio_in", "gpio_out", "lcd0", "lvds1", NULL, NULL, NULL, NULL}},
	{"PD13", 3, 13, {"gpio_in", "gpio_out", "lcd0", "lvds1", NULL, NULL, NULL, NULL}},
	{"PD14", 3, 14, {"gpio_in", "gpio_out", "lcd0", "lvds1", NULL, NULL, NULL, NULL}},
	{"PD15", 3, 15, {"gpio_in", "gpio_out", "lcd0", "lvds1", NULL, NULL, NULL, NULL}},
	{"PD16", 3, 16, {"gpio_in", "gpio_out", "lcd0", "lvds1", NULL, NULL, NULL, NULL}},
	{"PD17", 3, 17, {"gpio_in", "gpio_out", "lcd0", "lvds1", NULL, NULL, NULL, NULL}},
	{"PD18", 3, 18, {"gpio_in", "gpio_out", "lcd0", "lvds1", NULL, NULL, NULL, NULL}},
	{"PD19", 3, 19, {"gpio_in", "gpio_out", "lcd0", "lvds1", NULL, NULL, NULL, NULL}},
	{"PD20", 3, 20, {"gpio_in", "gpio_out", "lcd0", NULL, NULL, NULL, NULL, NULL}},
	{"PD21", 3, 21, {"gpio_in", "gpio_out", "lcd0", NULL, NULL, NULL, NULL, NULL}},
	{"PD22", 3, 22, {"gpio_in", "gpio_out", "lcd0", NULL, NULL, NULL, NULL, NULL}},
	{"PD23", 3, 23, {"gpio_in", "gpio_out", "lcd0", NULL, NULL, NULL, NULL, NULL}},
	{"PD24", 3, 24, {"gpio_in", "gpio_out", "lcd0", NULL, NULL, NULL, NULL, NULL}},
	{"PD25", 3, 25, {"gpio_in", "gpio_out", "lcd0", NULL, NULL, NULL, NULL, NULL}},
	{"PD26", 3, 26, {"gpio_in", "gpio_out", "lcd0", NULL, NULL, NULL, NULL, NULL}},
	{"PD27", 3, 27, {"gpio_in", "gpio_out", "lcd0", NULL, NULL, NULL, NULL, NULL}},

	{"PE0",  4, 0,  {"gpio_in", "gpio_out", "csi", "ts", NULL, NULL, NULL, NULL}},
	{"PE1",  4, 1,  {"gpio_in", "gpio_out", "csi", "ts", NULL, NULL, NULL, NULL}},
	{"PE2",  4, 2,  {"gpio_in", "gpio_out", "csi", "ts", NULL, NULL, NULL, NULL}},
	{"PE3",  4, 3,  {"gpio_in", "gpio_out", "csi", "ts", NULL, NULL, NULL, NULL}},
	{"PE4",  4, 4,  {"gpio_in", "gpio_out", "csi", "uart5", NULL, NULL, NULL, NULL}},
	{"PE5",  4, 5,  {"gpio_in", "gpio_out", "csi", "uart5", NULL, NULL, NULL, NULL}},
	{"PE6",  4, 6,  {"gpio_in", "gpio_out", "csi", "uart5", NULL, NULL, NULL, NULL}},
	{"PE7",  4, 7,  {"gpio_in", "gpio_out", "csi", "uart5", NULL, NULL, NULL, NULL}},
	{"PE8",  4, 8,  {"gpio_in", "gpio_out", "csi", "ts", NULL, NULL, NULL, NULL}},
	{"PE9",  4, 9,  {"gpio_in", "gpio_out", "csi", "ts", NULL, NULL, NULL, NULL}},
	{"PE10", 4, 10, {"gpio_in", "gpio_out", "csi", "ts", NULL, NULL, NULL, NULL}},
	{"PE11", 4, 11, {"gpio_in", "gpio_out", "csi", "ts", NULL, NULL, NULL, NULL}},
	{"PE12", 4, 12, {"gpio_in", "gpio_out", "csi", "ts", NULL, NULL, NULL, NULL}},
	{"PE13", 4, 13, {"gpio_in", "gpio_out", "csi", "ts", NULL, NULL, NULL, NULL}},
	{"PE14", 4, 14, {"gpio_in", "gpio_out", "csi", "ts", NULL, NULL, NULL, NULL}},
	{"PE15", 4, 15, {"gpio_in", "gpio_out", "csi", "ts", NULL, NULL, NULL, NULL}},
	{"PE16", 4, 16, {"gpio_in", "gpio_out", "csi", NULL, NULL, NULL, NULL, NULL}},

	{"PF0",  5, 0,  {"gpio_in", "gpio_out", "mmc0", NULL, "jtag", NULL, NULL, NULL}},
	{"PF1",  5, 1,  {"gpio_in", "gpio_out", "mmc0", NULL, "jtag", NULL, NULL, NULL}},
	{"PF2",  5, 2,  {"gpio_in", "gpio_out", "mmc0", NULL, "uart0", NULL, NULL, NULL}},
	{"PF3",  5, 3,  {"gpio_in", "gpio_out", "mmc0", NULL, "jtag", NULL, NULL, NULL}},
	{"PF4",  5, 4,  {"gpio_in", "gpio_out", "mmc0", NULL, "uart0", NULL, NULL, NULL}},
	{"PF5",  5, 5,  {"gpio_in", "gpio_out", "mmc0", NULL, "jtag", NULL, NULL, NULL}},

	{"PG0",  6, 0,  {"gpio_in", "gpio_out", "mmc1", NULL, NULL, NULL, NULL, NULL}},
	{"PG1",  6, 1,  {"gpio_in", "gpio_out", "mmc1", NULL, NULL, NULL, NULL, NULL}},
	{"PG2",  6, 2,  {"gpio_in", "gpio_out", "mmc1", NULL, NULL, NULL, NULL, NULL}},
	{"PG3",  6, 3,  {"gpio_in", "gpio_out", "mmc1", NULL, NULL, NULL, NULL, NULL}},
	{"PG4",  6, 4,  {"gpio_in", "gpio_out", "mmc1", NULL, NULL, NULL, NULL, NULL}},
	{"PG5",  6, 5,  {"gpio_in", "gpio_out", "mmc1", NULL, NULL, NULL, NULL, NULL}},
	{"PG6",  6, 6,  {"gpio_in", "gpio_out", "uart2", NULL, NULL, NULL, NULL, NULL}},
	{"PG7",  6, 7,  {"gpio_in", "gpio_out", "uart2", NULL, NULL, NULL, NULL, NULL}},
	{"PG8",  6, 8,  {"gpio_in", "gpio_out", "uart2", NULL, NULL, NULL, NULL, NULL}},
	{"PG9",  6, 9,  {"gpio_in", "gpio_out", "uart2", NULL, NULL, NULL, NULL, NULL}},
	{"PG10", 6, 10, {"gpio_in", "gpio_out", "i2c3", "usb", NULL, NULL, NULL, NULL}},
	{"PG11", 6, 11, {"gpio_in", "gpio_out", "i2c3", "usb", NULL, NULL, NULL, NULL}},
	{"PG12", 6, 11, {"gpio_in", "gpio_out", "spi1", "i2s1", NULL, NULL, NULL, NULL}},
	{"PG13", 6, 11, {"gpio_in", "gpio_out", "spi1", "i2s1", NULL, NULL, NULL, NULL}},
	{"PG14", 6, 11, {"gpio_in", "gpio_out", "spi1", "i2s1", NULL, NULL, NULL, NULL}},
	{"PG15", 6, 11, {"gpio_in", "gpio_out", "spi1", "i2s1", NULL, NULL, NULL, NULL}},
	{"PG16", 6, 11, {"gpio_in", "gpio_out", "spi1", "i2s1", NULL, NULL, NULL, NULL}},
	{"PG17", 6, 11, {"gpio_in", "gpio_out", "uart4", NULL, NULL, NULL, NULL, NULL}},
	{"PG18", 6, 11, {"gpio_in", "gpio_out", "uart4", NULL, NULL, NULL, NULL, NULL}},

	{"PH0",  7, 0,  {"gpio_in", "gpio_out", "nand1", NULL, NULL, NULL, NULL, NULL}},
	{"PH1",  7, 1,  {"gpio_in", "gpio_out", "nand1", NULL, NULL, NULL, NULL, NULL}},
	{"PH2",  7, 2,  {"gpio_in", "gpio_out", "nand1", NULL, NULL, NULL, NULL, NULL}},
	{"PH3",  7, 3,  {"gpio_in", "gpio_out", "nand1", NULL, NULL, NULL, NULL, NULL}},
	{"PH4",  7, 4,  {"gpio_in", "gpio_out", "nand1", NULL, NULL, NULL, NULL, NULL}},
	{"PH5",  7, 5,  {"gpio_in", "gpio_out", "nand1", NULL, NULL, NULL, NULL, NULL}},
	{"PH6",  7, 6,  {"gpio_in", "gpio_out", "nand1", NULL, NULL, NULL, NULL, NULL}},
	{"PH7",  7, 7,  {"gpio_in", "gpio_out", "nand1", NULL, NULL, NULL, NULL, NULL}},
	{"PH8",  7, 8,  {"gpio_in", "gpio_out", "nand1", NULL, NULL, NULL, NULL, NULL}},
	{"PH9",  7, 9,  {"gpio_in", "gpio_out", "spi2", "jtag", "pwm1", NULL, NULL, NULL}},
	{"PH10", 7, 10, {"gpio_in", "gpio_out", "spi2", "jtag", "pwm1", NULL, NULL, NULL}},
	{"PH11", 7, 11, {"gpio_in", "gpio_out", "spi2", "jtag", "pwm2", NULL, NULL, NULL}},
	{"PH12", 7, 12, {"gpio_in", "gpio_out", "spi2", "jtag", "pwm2", NULL, NULL, NULL}},
	{"PH13", 7, 13, {"gpio_in", "gpio_out", "pwm0", NULL, NULL, NULL, NULL, NULL}},
	{"PH14", 7, 14, {"gpio_in", "gpio_out", "i2c0", NULL, NULL, NULL, NULL, NULL}},
	{"PH15", 7, 15, {"gpio_in", "gpio_out", "i2c0", NULL, NULL, NULL, NULL, NULL}},
	{"PH16", 7, 16, {"gpio_in", "gpio_out", "i2c1", NULL, NULL, NULL, NULL, NULL}},
	{"PH17", 7, 17, {"gpio_in", "gpio_out", "i2c1", NULL, NULL, NULL, NULL, NULL}},
	{"PH18", 7, 18, {"gpio_in", "gpio_out", "i2c2", NULL, NULL, NULL, NULL, NULL}},
	{"PH19", 7, 19, {"gpio_in", "gpio_out", "i2c2", NULL, NULL, NULL, NULL, NULL}},
	{"PH20", 7, 20, {"gpio_in", "gpio_out", "uart0", NULL, NULL, NULL, NULL, NULL}},
	{"PH21", 7, 21, {"gpio_in", "gpio_out", "uart0", NULL, NULL, NULL, NULL, NULL}},
	{"PH22", 7, 22, {"gpio_in", "gpio_out", NULL, NULL, NULL, NULL, NULL, NULL}},
	{"PH23", 7, 23, {"gpio_in", "gpio_out", NULL, NULL, NULL, NULL, NULL, NULL}},
	{"PH24", 7, 24, {"gpio_in", "gpio_out", NULL, NULL, NULL, NULL, NULL, NULL}},
	{"PH25", 7, 25, {"gpio_in", "gpio_out", NULL, NULL, NULL, NULL, NULL, NULL}},
	{"PH26", 7, 26, {"gpio_in", "gpio_out", NULL, NULL, NULL, NULL, NULL, NULL}},
	{"PH27", 7, 27, {"gpio_in", "gpio_out", NULL, NULL, NULL, NULL, NULL, NULL}},
	{"PH28", 7, 28, {"gpio_in", "gpio_out", NULL, NULL, NULL, NULL, NULL, NULL}},
	{"PH29", 7, 29, {"gpio_in", "gpio_out", "nand1", NULL, NULL, NULL, NULL, NULL}},
	{"PH30", 7, 30, {"gpio_in", "gpio_out", "nand1", NULL, NULL, NULL, NULL, NULL}},
};

const struct allwinner_padconf a31_padconf = {
	.npins = nitems(a31_pins),
	.pins = a31_pins,
};
