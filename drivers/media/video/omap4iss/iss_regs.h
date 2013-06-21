/*
 * TI OMAP4 ISS V4L2 Driver - Register defines
 *
 * Copyright (C) 2012 Texas Instruments.
 *
 * Author: Sergio Aguirre <saaguirre@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _OMAP4_ISS_REGS_H_
#define _OMAP4_ISS_REGS_H_

extern phys_addr_t omap_cam_get_mempool_base(void);
extern phys_addr_t omap_cam_get_mempool_size(void);

/* ISS */
#define ISS_HL_REVISION					0x0

#define ISS_HL_SYSCONFIG				0x10
#define ISS_HL_SYSCONFIG_IDLEMODE_SHIFT			2
#define ISS_HL_SYSCONFIG_IDLEMODE_FORCEIDLE		0x0
#define ISS_HL_SYSCONFIG_IDLEMODE_NOIDLE		0x1
#define ISS_HL_SYSCONFIG_IDLEMODE_SMARTIDLE		0x2
#define ISS_HL_SYSCONFIG_SOFTRESET			(1 << 0)

#define ISS_HL_IRQSTATUS_5				(0x24 + (0x10 * 5))
#define ISS_HL_IRQENABLE_5_SET				(0x28 + (0x10 * 5))
#define ISS_HL_IRQENABLE_5_CLR				(0x2C + (0x10 * 5))

#define ISS_HL_IRQ_BTE					(1 << 11)
#define ISS_HL_IRQ_CBUFF				(1 << 10)
#define ISS_HL_IRQ_CSIB					(1 << 5)
#define ISS_HL_IRQ_CSIA					(1 << 4)

#define ISS_CTRL					0x80

#define ISS_CLKCTRL					0x84
#define ISS_CLKCTRL_VPORT2_CLK				(1 << 30)
#define ISS_CLKCTRL_VPORT1_CLK				(1 << 29)
#define ISS_CLKCTRL_VPORT0_CLK				(1 << 28)
#define ISS_CLKCTRL_CCP2				(1 << 4)
#define ISS_CLKCTRL_CSI2_B				(1 << 3)
#define ISS_CLKCTRL_CSI2_A				(1 << 2)
#define ISS_CLKCTRL_ISP					(1 << 1)
#define ISS_CLKCTRL_SIMCOP				(1 << 0)

#define ISS_CLKSTAT					0x88
#define ISS_CLKSTAT_VPORT2_CLK				(1 << 30)
#define ISS_CLKSTAT_VPORT1_CLK				(1 << 29)
#define ISS_CLKSTAT_VPORT0_CLK				(1 << 28)
#define ISS_CLKSTAT_CCP2				(1 << 4)
#define ISS_CLKSTAT_CSI2_B				(1 << 3)
#define ISS_CLKSTAT_CSI2_A				(1 << 2)
#define ISS_CLKSTAT_ISP					(1 << 1)
#define ISS_CLKSTAT_SIMCOP				(1 << 0)

#define ISS_PM_STATUS					0x8C
#define ISS_PM_STATUS_CBUFF_PM_MASK			(3 << 12)
#define ISS_PM_STATUS_BTE_PM_MASK			(3 << 10)
#define ISS_PM_STATUS_SIMCOP_PM_MASK			(3 << 8)
#define ISS_PM_STATUS_ISP_PM_MASK			(3 << 6)
#define ISS_PM_STATUS_CCP2_PM_MASK			(3 << 4)
#define ISS_PM_STATUS_CSI2_B_PM_MASK			(3 << 2)
#define ISS_PM_STATUS_CSI2_A_PM_MASK			(3 << 0)

#define REGISTER0					0x0
#define REGISTER0_HSCLOCKCONFIG				(1 << 24)
#define REGISTER0_THS_TERM_MASK				(0xFF << 8)
#define REGISTER0_THS_TERM_SHIFT			8
#define REGISTER0_THS_SETTLE_MASK			(0xFF << 0)
#define REGISTER0_THS_SETTLE_SHIFT			0

#define REGISTER1					0x4
#define REGISTER1_RESET_DONE_CTRLCLK			(1 << 29)
#define REGISTER1_CLOCK_MISS_DETECTOR_STATUS		(1 << 25)
#define REGISTER1_TCLK_TERM_MASK			(0x3F << 18)
#define REGISTER1_TCLK_TERM_SHIFT			18
#define REGISTER1_DPHY_HS_SYNC_PATTERN_SHIFT		10
#define REGISTER1_CTRLCLK_DIV_FACTOR_MASK		(0x3 << 8)
#define REGISTER1_CTRLCLK_DIV_FACTOR_SHIFT		8
#define REGISTER1_TCLK_SETTLE_MASK			(0xFF << 0)
#define REGISTER1_TCLK_SETTLE_SHIFT			0

#define REGISTER2					0x8

#define CSI2_SYSCONFIG					0x10
#define CSI2_SYSCONFIG_MSTANDBY_MODE_MASK		(3 << 12)
#define CSI2_SYSCONFIG_MSTANDBY_MODE_FORCE		(0 << 12)
#define CSI2_SYSCONFIG_MSTANDBY_MODE_NO			(1 << 12)
#define CSI2_SYSCONFIG_MSTANDBY_MODE_SMART		(2 << 12)
#define CSI2_SYSCONFIG_SOFT_RESET			(1 << 1)
#define CSI2_SYSCONFIG_AUTO_IDLE			(1 << 0)

#define CSI2_SYSSTATUS					0x14
#define CSI2_SYSSTATUS_RESET_DONE			(1 << 0)

#define CSI2_IRQSTATUS					0x18
#define CSI2_IRQENABLE					0x1C

/* Shared bits across CSI2_IRQENABLE and IRQSTATUS */

#define CSI2_IRQ_OCP_ERR				(1 << 14)
#define CSI2_IRQ_SHORT_PACKET				(1 << 13)
#define CSI2_IRQ_ECC_CORRECTION				(1 << 12)
#define CSI2_IRQ_ECC_NO_CORRECTION			(1 << 11)
#define CSI2_IRQ_COMPLEXIO_ERR				(1 << 9)
#define CSI2_IRQ_FIFO_OVF				(1 << 8)
#define CSI2_IRQ_CONTEXT0				(1 << 0)

#define CSI2_CTRL					0x40
#define CSI2_CTRL_MFLAG_LEVH_MASK			(7 << 20)
#define CSI2_CTRL_MFLAG_LEVH_SHIFT			20
#define CSI2_CTRL_MFLAG_LEVL_MASK			(7 << 17)
#define CSI2_CTRL_MFLAG_LEVL_SHIFT			17
#define CSI2_CTRL_BURST_SIZE_EXPAND			(1 << 16)
#define CSI2_CTRL_VP_CLK_EN				(1 << 15)
#define CSI2_CTRL_NON_POSTED_WRITE			(1 << 13)
#define CSI2_CTRL_VP_ONLY_EN				(1 << 11)
#define CSI2_CTRL_VP_OUT_CTRL_MASK			(3 << 8)
#define CSI2_CTRL_VP_OUT_CTRL_SHIFT			8
#define CSI2_CTRL_DBG_EN				(1 << 7)
#define CSI2_CTRL_BURST_SIZE_MASK			(3 << 5)
#define CSI2_CTRL_ENDIANNESS				(1 << 4)
#define CSI2_CTRL_FRAME					(1 << 3)
#define CSI2_CTRL_ECC_EN				(1 << 2)
#define CSI2_CTRL_IF_EN					(1 << 0)

#define CSI2_DBG_H					0x44

#define CSI2_COMPLEXIO_CFG				0x50
#define CSI2_COMPLEXIO_CFG_RESET_CTRL			(1 << 30)
#define CSI2_COMPLEXIO_CFG_RESET_DONE			(1 << 29)
#define CSI2_COMPLEXIO_CFG_PWD_CMD_MASK			(3 << 27)
#define CSI2_COMPLEXIO_CFG_PWD_CMD_OFF			(0 << 27)
#define CSI2_COMPLEXIO_CFG_PWD_CMD_ON			(1 << 27)
#define CSI2_COMPLEXIO_CFG_PWD_CMD_ULP			(2 << 27)
#define CSI2_COMPLEXIO_CFG_PWD_STATUS_MASK		(3 << 25)
#define CSI2_COMPLEXIO_CFG_PWD_STATUS_OFF		(0 << 25)
#define CSI2_COMPLEXIO_CFG_PWD_STATUS_ON		(1 << 25)
#define CSI2_COMPLEXIO_CFG_PWD_STATUS_ULP		(2 << 25)
#define CSI2_COMPLEXIO_CFG_PWR_AUTO			(1 << 24)
#define CSI2_COMPLEXIO_CFG_DATA_POL(i)			(1 << (((i) * 4) + 3))
#define CSI2_COMPLEXIO_CFG_DATA_POSITION_MASK(i)	(7 << ((i) * 4))
#define CSI2_COMPLEXIO_CFG_DATA_POSITION_SHIFT(i)	((i) * 4)
#define CSI2_COMPLEXIO_CFG_CLOCK_POL			(1 << 3)
#define CSI2_COMPLEXIO_CFG_CLOCK_POSITION_MASK		(7 << 0)
#define CSI2_COMPLEXIO_CFG_CLOCK_POSITION_SHIFT		0

#define CSI2_COMPLEXIO_IRQSTATUS			0x54

#define CSI2_SHORT_PACKET				0x5C

#define CSI2_COMPLEXIO_IRQENABLE			0x60

/* Shared bits across CSI2_COMPLEXIO_IRQENABLE and IRQSTATUS */
#define CSI2_COMPLEXIO_IRQ_STATEALLULPMEXIT		(1 << 26)
#define CSI2_COMPLEXIO_IRQ_STATEALLULPMENTER		(1 << 25)
#define CSI2_COMPLEXIO_IRQ_STATEULPM5			(1 << 24)
#define CSI2_COMPLEXIO_IRQ_STATEULPM4			(1 << 23)
#define CSI2_COMPLEXIO_IRQ_STATEULPM3			(1 << 22)
#define CSI2_COMPLEXIO_IRQ_STATEULPM2			(1 << 21)
#define CSI2_COMPLEXIO_IRQ_STATEULPM1			(1 << 20)
#define CSI2_COMPLEXIO_IRQ_ERRCONTROL5			(1 << 19)
#define CSI2_COMPLEXIO_IRQ_ERRCONTROL4			(1 << 18)
#define CSI2_COMPLEXIO_IRQ_ERRCONTROL3			(1 << 17)
#define CSI2_COMPLEXIO_IRQ_ERRCONTROL2			(1 << 16)
#define CSI2_COMPLEXIO_IRQ_ERRCONTROL1			(1 << 15)
#define CSI2_COMPLEXIO_IRQ_ERRESC5			(1 << 14)
#define CSI2_COMPLEXIO_IRQ_ERRESC4			(1 << 13)
#define CSI2_COMPLEXIO_IRQ_ERRESC3			(1 << 12)
#define CSI2_COMPLEXIO_IRQ_ERRESC2			(1 << 11)
#define CSI2_COMPLEXIO_IRQ_ERRESC1			(1 << 10)
#define CSI2_COMPLEXIO_IRQ_ERRSOTSYNCHS5		(1 << 9)
#define CSI2_COMPLEXIO_IRQ_ERRSOTSYNCHS4		(1 << 8)
#define CSI2_COMPLEXIO_IRQ_ERRSOTSYNCHS3		(1 << 7)
#define CSI2_COMPLEXIO_IRQ_ERRSOTSYNCHS2		(1 << 6)
#define CSI2_COMPLEXIO_IRQ_ERRSOTSYNCHS1		(1 << 5)
#define CSI2_COMPLEXIO_IRQ_ERRSOTHS5			(1 << 4)
#define CSI2_COMPLEXIO_IRQ_ERRSOTHS4			(1 << 3)
#define CSI2_COMPLEXIO_IRQ_ERRSOTHS3			(1 << 2)
#define CSI2_COMPLEXIO_IRQ_ERRSOTHS2			(1 << 1)
#define CSI2_COMPLEXIO_IRQ_ERRSOTHS1			(1 << 0)

#define CSI2_DBG_P					0x68

#define CSI2_TIMING					0x6C
#define CSI2_TIMING_FORCE_RX_MODE_IO1			(1 << 15)
#define CSI2_TIMING_STOP_STATE_X16_IO1			(1 << 14)
#define CSI2_TIMING_STOP_STATE_X4_IO1			(1 << 13)
#define CSI2_TIMING_STOP_STATE_COUNTER_IO1_MASK		(0x1FFF << 0)
#define CSI2_TIMING_STOP_STATE_COUNTER_IO1_SHIFT	0

#define CSI2_CTX_CTRL1(i)				(0x70 + (0x20 * i))
#define CSI2_CTX_CTRL1_GENERIC				(1 << 30)
#define CSI2_CTX_CTRL1_TRANSCODE			(0xF << 24)
#define CSI2_CTX_CTRL1_FEC_NUMBER_MASK			(0xFF << 16)
#define CSI2_CTX_CTRL1_COUNT_MASK			(0xFF << 8)
#define CSI2_CTX_CTRL1_COUNT_SHIFT			8
#define CSI2_CTX_CTRL1_EOF_EN				(1 << 7)
#define CSI2_CTX_CTRL1_EOL_EN				(1 << 6)
#define CSI2_CTX_CTRL1_CS_EN				(1 << 5)
#define CSI2_CTX_CTRL1_COUNT_UNLOCK			(1 << 4)
#define CSI2_CTX_CTRL1_PING_PONG			(1 << 3)
#define CSI2_CTX_CTRL1_CTX_EN				(1 << 0)

#define CSI2_CTX_CTRL2(i)				(0x74 + (0x20 * i))
#define CSI2_CTX_CTRL2_USER_DEF_MAP_SHIFT		13
#define CSI2_CTX_CTRL2_USER_DEF_MAP_MASK		\
		(0x3 << CSI2_CTX_CTRL2_USER_DEF_MAP_SHIFT)
#define CSI2_CTX_CTRL2_VIRTUAL_ID_MASK			(3 << 11)
#define CSI2_CTX_CTRL2_VIRTUAL_ID_SHIFT			11
#define CSI2_CTX_CTRL2_DPCM_PRED			(1 << 10)
#define CSI2_CTX_CTRL2_FORMAT_MASK			(0x3FF << 0)
#define CSI2_CTX_CTRL2_FORMAT_SHIFT			0

#define CSI2_CTX_DAT_OFST(i)				(0x78 + (0x20 * i))
#define CSI2_CTX_DAT_OFST_MASK				(0xFFF << 5)

#define CSI2_CTX_PING_ADDR(i)				(0x7C + (0x20 * i))
#define CSI2_CTX_PING_ADDR_MASK				0xFFFFFFE0

#define CSI2_CTX_PONG_ADDR(i)				(0x80 + (0x20 * i))
#define CSI2_CTX_PONG_ADDR_MASK				CSI2_CTX_PING_ADDR_MASK

#define CSI2_CTX_IRQENABLE(i)				(0x84 + (0x20 * i))
#define CSI2_CTX_IRQSTATUS(i)				(0x88 + (0x20 * i))

#define CSI2_CTX_CTRL3(i)				(0x8C + (0x20 * i))
#define CSI2_CTX_CTRL3_ALPHA_SHIFT			5
#define CSI2_CTX_CTRL3_ALPHA_MASK			\
		(0x3fff << CSI2_CTX_CTRL3_ALPHA_SHIFT)

/* Shared bits across CSI2_CTX_IRQENABLE and IRQSTATUS */
#define CSI2_CTX_IRQ_ECC_CORRECTION			(1 << 8)
#define CSI2_CTX_IRQ_LINE_NUMBER			(1 << 7)
#define CSI2_CTX_IRQ_FRAME_NUMBER			(1 << 6)
#define CSI2_CTX_IRQ_CS					(1 << 5)
#define CSI2_CTX_IRQ_LE					(1 << 3)
#define CSI2_CTX_IRQ_LS					(1 << 2)
#define CSI2_CTX_IRQ_FE					(1 << 1)
#define CSI2_CTX_IRQ_FS					(1 << 0)

#endif /* _OMAP4_CAMERA_REGS_H_ */
