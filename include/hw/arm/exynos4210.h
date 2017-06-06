/*
 *  Samsung exynos4210 SoC emulation
 *
 *  Copyright (c) 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *    Maksim Kozlov <m.kozlov@samsung.com>
 *    Evgeny Voevodin <e.voevodin@samsung.com>
 *    Igor Mitsyanko <i.mitsyanko@samsung.com>
 *
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef EXYNOS4210_H
#define EXYNOS4210_H

#include "qemu-common.h"
#include "exec/memory.h"
#include "target/arm/cpu-qom.h"
#include "hw/cpu/a9mpcore.h"

#define TYPE_EXYNOS4210 "exynos4210"
#define EXYNOS4210(obj) \
    OBJECT_CHECK(Exynos4210State, (obj), TYPE_EXYNOS4210)

#define EXYNOS4210_NCPUS                    2

#define EXYNOS4210_DRAM0_BASE_ADDR          0x40000000
#define EXYNOS4210_DRAM1_BASE_ADDR          0xa0000000
#define EXYNOS4210_DRAM_MAX_SIZE            0x60000000  /* 1.5 GB */

#define EXYNOS4210_IROM_BASE_ADDR           0x00000000
#define EXYNOS4210_IROM_SIZE                0x00010000  /* 64 KB */
#define EXYNOS4210_IROM_MIRROR_BASE_ADDR    0x02000000
#define EXYNOS4210_IROM_MIRROR_SIZE         0x00010000  /* 64 KB */

#define EXYNOS4210_IRAM_BASE_ADDR           0x02020000
#define EXYNOS4210_IRAM_SIZE                0x00020000  /* 128 KB */

/* Secondary CPU startup code is in IROM memory */
#define EXYNOS4210_SMP_BOOT_ADDR            EXYNOS4210_IROM_BASE_ADDR
#define EXYNOS4210_SMP_BOOT_SIZE            0x1000
#define EXYNOS4210_BASE_BOOT_ADDR           EXYNOS4210_DRAM0_BASE_ADDR
/* Secondary CPU polling address to get loader start from */
#define EXYNOS4210_SECOND_CPU_BOOTREG       0x10020814

#define EXYNOS4210_SMP_PRIVATE_BASE_ADDR    0x10500000
#define EXYNOS4210_L2X0_BASE_ADDR           0x10502000

/*
 * exynos4210 IRQ subsystem stub definitions.
 */
#define EXYNOS4210_IRQ_GATE_NINPUTS 2 /* Internal and External GIC */

#define EXYNOS4210_MAX_INT_COMBINER_OUT_IRQ  64
#define EXYNOS4210_MAX_EXT_COMBINER_OUT_IRQ  16
#define EXYNOS4210_MAX_INT_COMBINER_IN_IRQ   \
    (EXYNOS4210_MAX_INT_COMBINER_OUT_IRQ * 8)
#define EXYNOS4210_MAX_EXT_COMBINER_IN_IRQ   \
    (EXYNOS4210_MAX_EXT_COMBINER_OUT_IRQ * 8)

#define EXYNOS4210_COMBINER_GET_IRQ_NUM(grp, bit)  ((grp)*8 + (bit))
#define EXYNOS4210_COMBINER_GET_GRP_NUM(irq)       ((irq) / 8)
#define EXYNOS4210_COMBINER_GET_BIT_NUM(irq) \
    ((irq) - 8 * EXYNOS4210_COMBINER_GET_GRP_NUM(irq))

/* IRQs number for external and internal GIC */
#define EXYNOS4210_EXT_GIC_NIRQ     (160-32)
#define EXYNOS4210_INT_GIC_NIRQ     64

#define EXYNOS4210_I2C_NUMBER               9

typedef struct Exynos4210Irq {
    qemu_irq int_combiner_irq[EXYNOS4210_MAX_INT_COMBINER_IN_IRQ];
    qemu_irq ext_combiner_irq[EXYNOS4210_MAX_EXT_COMBINER_IN_IRQ];
    qemu_irq int_gic_irq[EXYNOS4210_INT_GIC_NIRQ];
    qemu_irq ext_gic_irq[EXYNOS4210_EXT_GIC_NIRQ];
    qemu_irq board_irqs[EXYNOS4210_MAX_INT_COMBINER_IN_IRQ];
} Exynos4210Irq;

#define TYPE_EXYNOS4210_GIC "exynos4210.gic"
#define EXYNOS4210_GIC(obj) \
    OBJECT_CHECK(Exynos4210GicState, (obj), TYPE_EXYNOS4210_GIC)

typedef struct {
    SysBusDevice parent_obj;

    MemoryRegion cpu_container;
    MemoryRegion dist_container;
    MemoryRegion cpu_alias[EXYNOS4210_NCPUS];
    MemoryRegion dist_alias[EXYNOS4210_NCPUS];
    uint32_t num_cpu;
    DeviceState *gic;
} Exynos4210GicState;

/* IRQ OR Gate struct.
 *
 * This device models an OR gate. There are n_in input qdev gpio lines and one
 * output sysbus IRQ line. The output IRQ level is formed as OR between all
 * gpio inputs.
 */

#define TYPE_EXYNOS4210_IRQ_GATE "exynos4210.irq_gate"
#define EXYNOS4210_IRQ_GATE(obj) \
    OBJECT_CHECK(Exynos4210IRQGateState, (obj), TYPE_EXYNOS4210_IRQ_GATE)

typedef struct Exynos4210IRQGateState {
    SysBusDevice parent_obj;

    uint32_t n_in;      /* inputs amount */
    uint32_t *level;    /* input levels */
    qemu_irq out;       /* output IRQ */
} Exynos4210IRQGateState;

/*
 * State for each output signal of internal combiner
 */
typedef struct Exynos4210CombinerGroupState {
    uint8_t src_mask;            /* 1 - source enabled, 0 - disabled */
    uint8_t src_pending;        /* Pending source interrupts before masking */
} Exynos4210CombinerGroupState;

/* Internal Interrupt Combiner Groups number */
#define EXYNOS4210_COMBINER_IIC_NGRP            64
/* Internal Interrupt Combiner Interrupts number */
#define EXYNOS4210_COMBINER_IIC_NIRQ            (EXYNOS4210_COMBINER_IIC_NGRP * 8)
/* Size of memory mapped region */
#define EXYNOS4210_COMBINER_IIC_REGION_SIZE     0x108
#define EXYNOS4210_COMBINER_IIC_REGSET_SIZE     0x41

#define TYPE_EXYNOS4210_COMBINER "exynos4210.combiner"
#define EXYNOS4210_COMBINER(obj) \
    OBJECT_CHECK(Exynos4210CombinerState, (obj), TYPE_EXYNOS4210_COMBINER)

typedef struct Exynos4210CombinerState {
    SysBusDevice parent_obj;

    MemoryRegion iomem;

    struct Exynos4210CombinerGroupState group[EXYNOS4210_COMBINER_IIC_NGRP];
    uint32_t reg_set[EXYNOS4210_COMBINER_IIC_REGSET_SIZE];
    uint32_t icipsr[2];
    uint32_t external;          /* 1 means that this combiner is external */

    qemu_irq output_irq[EXYNOS4210_COMBINER_IIC_NGRP];
} Exynos4210CombinerState;


typedef struct Exynos4210State {
    DeviceState parent_obj;

    ARMCPU *cpu[EXYNOS4210_NCPUS];

    A9MPPrivState a9mpcore;
    Exynos4210GicState gic;
    Exynos4210IRQGateState irq_gate[EXYNOS4210_NCPUS];
    Exynos4210CombinerState combiner;
    Exynos4210Irq irqs;
    qemu_irq *irq_table;

    MemoryRegion chipid_mem;
    MemoryRegion iram_mem;
    MemoryRegion irom_mem;
    MemoryRegion irom_alias_mem;
    MemoryRegion boot_secondary;
    MemoryRegion bootreg_mem;
    I2CBus *i2c_if[EXYNOS4210_I2C_NUMBER];
} Exynos4210State;

void exynos4210_write_secondary(ARMCPU *cpu,
        const struct arm_boot_info *info);

/* Initialize exynos4210 IRQ subsystem stub */
qemu_irq *exynos4210_init_irq(Exynos4210Irq *env);

/* Initialize board IRQs.
 * These IRQs contain splitted Int/External Combiner and External Gic IRQs */
void exynos4210_init_board_irqs(Exynos4210Irq *s);

/* Get IRQ number from exynos4210 IRQ subsystem stub.
 * To identify IRQ source use internal combiner group and bit number
 *  grp - group number
 *  bit - bit number inside group */
uint32_t exynos4210_get_irq(uint32_t grp, uint32_t bit);

/*
 * Get Combiner input GPIO into irqs structure
 */
void exynos4210_combiner_get_gpioin(Exynos4210Irq *irqs, DeviceState *dev,
        int ext);

/*
 * exynos4210 UART
 */
DeviceState *exynos4210_uart_create(hwaddr addr,
                                    int fifo_size,
                                    int channel,
                                    Chardev *chr,
                                    qemu_irq irq);

#endif /* EXYNOS4210_H */
