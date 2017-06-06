#ifndef EXYNOS4210_MCT_H
#define EXYNOS4210_MCT_H

/* global timer */
typedef struct {
    qemu_irq  irq[MCT_GT_CMP_NUM];

    struct gregs {
        uint64_t cnt;
        uint32_t cnt_wstat;
        uint32_t tcon;
        uint32_t int_cstat;
        uint32_t int_enb;
        uint32_t wstat;
        uint64_t comp[MCT_GT_CMP_NUM];
        uint32_t comp_add_incr[MCT_GT_CMP_NUM];
    } reg;

    uint64_t count;            /* Value FRC was armed with */
    int32_t curr_comp;             /* Current comparator FRC is running to */

    ptimer_state *ptimer_frc;                   /* FRC timer */

} Exynos4210MCTGT;

/* local timer */
typedef struct {
    int         id;             /* timer id */
    qemu_irq    irq;            /* local timer irq */

    struct tick_timer {
        uint32_t cnt_run;           /* cnt timer is running */
        uint32_t int_run;           /* int timer is running */

        uint32_t last_icnto;
        uint32_t last_tcnto;
        uint32_t tcntb;             /* initial value for TCNTB */
        uint32_t icntb;             /* initial value for ICNTB */

        /* for step mode */
        uint64_t    distance;       /* distance to count to the next event */
        uint64_t    progress;       /* progress when counting by steps */
        uint64_t    count;          /* count to arm timer with */

        ptimer_state *ptimer_tick;  /* timer for tick counter */
    } tick_timer;

    /* use ptimer.c to represent count down timer */

    ptimer_state *ptimer_frc;   /* timer for free running counter */

    /* registers */
    struct lregs {
        uint32_t    cnt[L_REG_CNT_AMOUNT];
        uint32_t    tcon;
        uint32_t    int_cstat;
        uint32_t    int_enb;
        uint32_t    wstat;
    } reg;

} Exynos4210MCTLT;

#define TYPE_EXYNOS4210_MCT "exynos4210.mct"
#define EXYNOS4210_MCT(obj) \
    OBJECT_CHECK(Exynos4210MCTState, (obj), TYPE_EXYNOS4210_MCT)

typedef struct Exynos4210MCTState {
    SysBusDevice parent_obj;

    MemoryRegion iomem;

    /* Registers */
    uint32_t    reg_mct_cfg;

    Exynos4210MCTLT l_timer[2];
    Exynos4210MCTGT g_timer;

    uint32_t    freq;                   /* all timers tick frequency, TCLK */
} Exynos4210MCTState;


#endif /* EXYNOS4210_MCT_H */
