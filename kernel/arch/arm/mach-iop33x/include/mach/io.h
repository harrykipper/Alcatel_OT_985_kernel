

#ifndef __IO_H
#define __IO_H

#include <asm/hardware/iop3xx.h>

extern void __iomem *__iop3xx_ioremap(unsigned long cookie, size_t size,
	unsigned int mtype);
extern void __iop3xx_iounmap(void __iomem *addr);

#define IO_SPACE_LIMIT		0xffffffff
#define __io(p)		((void __iomem *)IOP3XX_PCI_IO_PHYS_TO_VIRT(p))
#define __mem_pci(a)		(a)

#define __arch_ioremap(a, s, f) __iop3xx_ioremap(a, s, f)
#define __arch_iounmap(a)	 __iop3xx_iounmap(a)

#endif
