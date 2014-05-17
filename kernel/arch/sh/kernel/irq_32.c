
#include <linux/irqflags.h>
#include <linux/module.h>

void notrace raw_local_irq_restore(unsigned long flags)
{
	unsigned long __dummy0, __dummy1;

	if (flags == RAW_IRQ_DISABLED) {
		__asm__ __volatile__ (
			"stc	sr, %0\n\t"
			"or	#0xf0, %0\n\t"
			"ldc	%0, sr\n\t"
			: "=&z" (__dummy0)
			: /* no inputs */
			: "memory"
		);
	} else {
		__asm__ __volatile__ (
			"stc	sr, %0\n\t"
			"and	%1, %0\n\t"
#ifdef CONFIG_CPU_HAS_SR_RB
			"stc	r6_bank, %1\n\t"
			"or	%1, %0\n\t"
#endif
			"ldc	%0, sr\n\t"
			: "=&r" (__dummy0), "=r" (__dummy1)
			: "1" (~RAW_IRQ_DISABLED)
			: "memory"
		);
	}
}
EXPORT_SYMBOL(raw_local_irq_restore);

unsigned long notrace __raw_local_save_flags(void)
{
	unsigned long flags;

	__asm__ __volatile__ (
		"stc	sr, %0\n\t"
		"and	#0xf0, %0\n\t"
		: "=&z" (flags)
		: /* no inputs */
		: "memory"
	);

	return flags;
}
EXPORT_SYMBOL(__raw_local_save_flags);
