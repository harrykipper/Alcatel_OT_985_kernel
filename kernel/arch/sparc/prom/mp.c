

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>

#include <asm/openprom.h>
#include <asm/oplib.h>

extern void restore_current(void);

int
prom_startcpu(int cpunode, struct linux_prom_registers *ctable_reg, int ctx, char *pc)
{
	int ret;
	unsigned long flags;

	spin_lock_irqsave(&prom_lock, flags);
	switch(prom_vers) {
	case PROM_V0:
	case PROM_V2:
	default:
		ret = -1;
		break;
	case PROM_V3:
		ret = (*(romvec->v3_cpustart))(cpunode, (int) ctable_reg, ctx, pc);
		break;
	};
	restore_current();
	spin_unlock_irqrestore(&prom_lock, flags);

	return ret;
}

int
prom_stopcpu(int cpunode)
{
	int ret;
	unsigned long flags;

	spin_lock_irqsave(&prom_lock, flags);
	switch(prom_vers) {
	case PROM_V0:
	case PROM_V2:
	default:
		ret = -1;
		break;
	case PROM_V3:
		ret = (*(romvec->v3_cpustop))(cpunode);
		break;
	};
	restore_current();
	spin_unlock_irqrestore(&prom_lock, flags);

	return ret;
}

int
prom_idlecpu(int cpunode)
{
	int ret;
	unsigned long flags;

	spin_lock_irqsave(&prom_lock, flags);
	switch(prom_vers) {
	case PROM_V0:
	case PROM_V2:
	default:
		ret = -1;
		break;
	case PROM_V3:
		ret = (*(romvec->v3_cpuidle))(cpunode);
		break;
	};
	restore_current();
	spin_unlock_irqrestore(&prom_lock, flags);

	return ret;
}

int
prom_restartcpu(int cpunode)
{
	int ret;
	unsigned long flags;

	spin_lock_irqsave(&prom_lock, flags);
	switch(prom_vers) {
	case PROM_V0:
	case PROM_V2:
	default:
		ret = -1;
		break;
	case PROM_V3:
		ret = (*(romvec->v3_cpuresume))(cpunode);
		break;
	};
	restore_current();
	spin_unlock_irqrestore(&prom_lock, flags);

	return ret;
}
