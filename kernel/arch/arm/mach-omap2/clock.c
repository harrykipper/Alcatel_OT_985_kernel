
#undef DEBUG

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/bitops.h>

#include <plat/clock.h>
#include <plat/clockdomain.h>
#include <plat/cpu.h>
#include <plat/prcm.h>

#include "clock.h"
#include "prm.h"
#include "prm-regbits-24xx.h"
#include "cm.h"
#include "cm-regbits-24xx.h"
#include "cm-regbits-34xx.h"

u8 cpu_mask;


/* Private functions */

static void _omap2_module_wait_ready(struct clk *clk)
{
	void __iomem *companion_reg, *idlest_reg;
	u8 other_bit, idlest_bit, idlest_val;

	/* Not all modules have multiple clocks that their IDLEST depends on */
	if (clk->ops->find_companion) {
		clk->ops->find_companion(clk, &companion_reg, &other_bit);
		if (!(__raw_readl(companion_reg) & (1 << other_bit)))
			return;
	}

	clk->ops->find_idlest(clk, &idlest_reg, &idlest_bit, &idlest_val);

	omap2_cm_wait_idlest(idlest_reg, (1 << idlest_bit), idlest_val,
			     clk->name);
}

/* Public functions */

void omap2_init_clk_clkdm(struct clk *clk)
{
	struct clockdomain *clkdm;

	if (!clk->clkdm_name)
		return;

	clkdm = clkdm_lookup(clk->clkdm_name);
	if (clkdm) {
		pr_debug("clock: associated clk %s to clkdm %s\n",
			 clk->name, clk->clkdm_name);
		clk->clkdm = clkdm;
	} else {
		pr_debug("clock: could not associate clk %s to "
			 "clkdm %s\n", clk->name, clk->clkdm_name);
	}
}

void omap2_clk_dflt_find_companion(struct clk *clk, void __iomem **other_reg,
				   u8 *other_bit)
{
	u32 r;

	/*
	 * Convert CM_ICLKEN* <-> CM_FCLKEN*.  This conversion assumes
	 * it's just a matter of XORing the bits.
	 */
	r = ((__force u32)clk->enable_reg ^ (CM_FCLKEN ^ CM_ICLKEN));

	*other_reg = (__force void __iomem *)r;
	*other_bit = clk->enable_bit;
}

void omap2_clk_dflt_find_idlest(struct clk *clk, void __iomem **idlest_reg,
				u8 *idlest_bit, u8 *idlest_val)
{
	u32 r;

	r = (((__force u32)clk->enable_reg & ~0xf0) | 0x20);
	*idlest_reg = (__force void __iomem *)r;
	*idlest_bit = clk->enable_bit;

	/*
	 * 24xx uses 0 to indicate not ready, and 1 to indicate ready.
	 * 34xx reverses this, just to keep us on our toes
	 * AM35xx uses both, depending on the module.
	 */
	if (cpu_is_omap24xx())
		*idlest_val = OMAP24XX_CM_IDLEST_VAL;
	else if (cpu_is_omap34xx())
		*idlest_val = OMAP34XX_CM_IDLEST_VAL;
	else
		BUG();

}

int omap2_dflt_clk_enable(struct clk *clk)
{
	u32 v;

	if (unlikely(clk->enable_reg == NULL)) {
		pr_err("clock.c: Enable for %s without enable code\n",
		       clk->name);
		return 0; /* REVISIT: -EINVAL */
	}

	v = __raw_readl(clk->enable_reg);
	if (clk->flags & INVERT_ENABLE)
		v &= ~(1 << clk->enable_bit);
	else
		v |= (1 << clk->enable_bit);
	__raw_writel(v, clk->enable_reg);
	v = __raw_readl(clk->enable_reg); /* OCP barrier */

	if (clk->ops->find_idlest)
		_omap2_module_wait_ready(clk);

	return 0;
}

void omap2_dflt_clk_disable(struct clk *clk)
{
	u32 v;

	if (!clk->enable_reg) {
		/*
		 * 'Independent' here refers to a clock which is not
		 * controlled by its parent.
		 */
		printk(KERN_ERR "clock: clk_disable called on independent "
		       "clock %s which has no enable_reg\n", clk->name);
		return;
	}

	v = __raw_readl(clk->enable_reg);
	if (clk->flags & INVERT_ENABLE)
		v |= (1 << clk->enable_bit);
	else
		v &= ~(1 << clk->enable_bit);
	__raw_writel(v, clk->enable_reg);
	/* No OCP barrier needed here since it is a disable operation */
}

const struct clkops clkops_omap2_dflt_wait = {
	.enable		= omap2_dflt_clk_enable,
	.disable	= omap2_dflt_clk_disable,
	.find_companion	= omap2_clk_dflt_find_companion,
	.find_idlest	= omap2_clk_dflt_find_idlest,
};

const struct clkops clkops_omap2_dflt = {
	.enable		= omap2_dflt_clk_enable,
	.disable	= omap2_dflt_clk_disable,
};

void omap2_clk_disable(struct clk *clk)
{
	if (clk->usecount == 0) {
		WARN(1, "clock: %s: omap2_clk_disable() called, but usecount "
		     "already 0?", clk->name);
		return;
	}

	pr_debug("clock: %s: decrementing usecount\n", clk->name);

	clk->usecount--;

	if (clk->usecount > 0)
		return;

	pr_debug("clock: %s: disabling in hardware\n", clk->name);

	clk->ops->disable(clk);

	if (clk->clkdm)
		omap2_clkdm_clk_disable(clk->clkdm, clk);

	if (clk->parent)
		omap2_clk_disable(clk->parent);
}

int omap2_clk_enable(struct clk *clk)
{
	int ret;

	pr_debug("clock: %s: incrementing usecount\n", clk->name);

	clk->usecount++;

	if (clk->usecount > 1)
		return 0;

	pr_debug("clock: %s: enabling in hardware\n", clk->name);

	if (clk->parent) {
		ret = omap2_clk_enable(clk->parent);
		if (ret) {
			WARN(1, "clock: %s: could not enable parent %s: %d\n",
			     clk->name, clk->parent->name, ret);
			goto oce_err1;
		}
	}

	if (clk->clkdm) {
		ret = omap2_clkdm_clk_enable(clk->clkdm, clk);
		if (ret) {
			WARN(1, "clock: %s: could not enable clockdomain %s: "
			     "%d\n", clk->name, clk->clkdm->name, ret);
			goto oce_err2;
		}
	}

	ret = clk->ops->enable(clk);
	if (ret) {
		WARN(1, "clock: %s: could not enable: %d\n", clk->name, ret);
		goto oce_err3;
	}

	return 0;

oce_err3:
	if (clk->clkdm)
		omap2_clkdm_clk_disable(clk->clkdm, clk);
oce_err2:
	if (clk->parent)
		omap2_clk_disable(clk->parent);
oce_err1:
	clk->usecount--;

	return ret;
}

/* Given a clock and a rate apply a clock specific rounding function */
long omap2_clk_round_rate(struct clk *clk, unsigned long rate)
{
	if (clk->round_rate)
		return clk->round_rate(clk, rate);

	return clk->rate;
}

/* Set the clock rate for a clock source */
int omap2_clk_set_rate(struct clk *clk, unsigned long rate)
{
	int ret = -EINVAL;

	pr_debug("clock: set_rate for clock %s to rate %ld\n", clk->name, rate);

	/* dpll_ck, core_ck, virt_prcm_set; plus all clksel clocks */
	if (clk->set_rate)
		ret = clk->set_rate(clk, rate);

	return ret;
}

int omap2_clk_set_parent(struct clk *clk, struct clk *new_parent)
{
	if (!clk->clksel)
		return -EINVAL;

	if (clk->parent == new_parent)
		return 0;

	return omap2_clksel_set_parent(clk, new_parent);
}

/* OMAP3/4 non-CORE DPLL clkops */

#if defined(CONFIG_ARCH_OMAP3) || defined(CONFIG_ARCH_OMAP4)

const struct clkops clkops_omap3_noncore_dpll_ops = {
	.enable		= omap3_noncore_dpll_enable,
	.disable	= omap3_noncore_dpll_disable,
};

#endif



#ifdef CONFIG_OMAP_RESET_CLOCKS
void omap2_clk_disable_unused(struct clk *clk)
{
	u32 regval32, v;

	v = (clk->flags & INVERT_ENABLE) ? (1 << clk->enable_bit) : 0;

	regval32 = __raw_readl(clk->enable_reg);
	if ((regval32 & (1 << clk->enable_bit)) == v)
		return;

	printk(KERN_DEBUG "Disabling unused clock \"%s\"\n", clk->name);
	if (cpu_is_omap34xx()) {
		omap2_clk_enable(clk);
		omap2_clk_disable(clk);
	} else {
		clk->ops->disable(clk);
	}
	if (clk->clkdm != NULL)
		pwrdm_clkdm_state_switch(clk->clkdm);
}
#endif

int __init omap2_clk_switch_mpurate_at_boot(const char *mpurate_ck_name)
{
	struct clk *mpurate_ck;
	int r;

	if (!mpurate)
		return -EINVAL;

	mpurate_ck = clk_get(NULL, mpurate_ck_name);
	if (WARN(IS_ERR(mpurate_ck), "Failed to get %s.\n", mpurate_ck_name))
		return -ENOENT;

	r = clk_set_rate(mpurate_ck, mpurate);
	if (IS_ERR_VALUE(r)) {
		WARN(1, "clock: %s: unable to set MPU rate to %d: %d\n",
		     mpurate_ck->name, mpurate, r);
		return -EINVAL;
	}

	calibrate_delay();
	recalculate_root_clocks();

	clk_put(mpurate_ck);

	return 0;
}

void __init omap2_clk_print_new_rates(const char *hfclkin_ck_name,
				      const char *core_ck_name,
				      const char *mpu_ck_name)
{
	struct clk *hfclkin_ck, *core_ck, *mpu_ck;
	unsigned long hfclkin_rate;

	mpu_ck = clk_get(NULL, mpu_ck_name);
	if (WARN(IS_ERR(mpu_ck), "clock: failed to get %s.\n", mpu_ck_name))
		return;

	core_ck = clk_get(NULL, core_ck_name);
	if (WARN(IS_ERR(core_ck), "clock: failed to get %s.\n", core_ck_name))
		return;

	hfclkin_ck = clk_get(NULL, hfclkin_ck_name);
	if (WARN(IS_ERR(hfclkin_ck), "Failed to get %s.\n", hfclkin_ck_name))
		return;

	hfclkin_rate = clk_get_rate(hfclkin_ck);

	pr_info("Switched to new clocking rate (Crystal/Core/MPU): "
		"%ld.%01ld/%ld/%ld MHz\n",
		(hfclkin_rate / 1000000),
		((hfclkin_rate / 100000) % 10),
		(clk_get_rate(core_ck) / 1000000),
		(clk_get_rate(mpu_ck) / 1000000));
}

/* Common data */

struct clk_functions omap2_clk_functions = {
	.clk_enable		= omap2_clk_enable,
	.clk_disable		= omap2_clk_disable,
	.clk_round_rate		= omap2_clk_round_rate,
	.clk_set_rate		= omap2_clk_set_rate,
	.clk_set_parent		= omap2_clk_set_parent,
	.clk_disable_unused	= omap2_clk_disable_unused,
#ifdef CONFIG_CPU_FREQ
	/* These will be removed when the OPP code is integrated */
	.clk_init_cpufreq_table	= omap2_clk_init_cpufreq_table,
	.clk_exit_cpufreq_table	= omap2_clk_exit_cpufreq_table,
#endif
};

