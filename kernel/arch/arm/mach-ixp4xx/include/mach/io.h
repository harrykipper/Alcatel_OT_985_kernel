

#ifndef __ASM_ARM_ARCH_IO_H
#define __ASM_ARM_ARCH_IO_H

#include <linux/bitops.h>

#include <mach/hardware.h>

#define IO_SPACE_LIMIT 0x0000ffff

extern int (*ixp4xx_pci_read)(u32 addr, u32 cmd, u32* data);
extern int ixp4xx_pci_write(u32 addr, u32 cmd, u32 data);


#ifndef	CONFIG_IXP4XX_INDIRECT_PCI

#define __mem_pci(a)		(a)

#else


static inline int is_pci_memory(u32 addr)
{
	return (addr >= PCIBIOS_MIN_MEM) && (addr <= 0x4FFFFFFF);
}

static inline void __iomem * __indirect_ioremap(unsigned long addr, size_t size,
						unsigned int mtype)
{
	if (!is_pci_memory(addr))
		return __arm_ioremap(addr, size, mtype);

	return (void __iomem *)addr;
}

static inline void __indirect_iounmap(void __iomem *addr)
{
	if (!is_pci_memory((__force u32)addr))
		__iounmap(addr);
}

#define __arch_ioremap(a, s, f)		__indirect_ioremap(a, s, f)
#define __arch_iounmap(a)		__indirect_iounmap(a)

#define writeb(v, p)			__indirect_writeb(v, p)
#define writew(v, p)			__indirect_writew(v, p)
#define writel(v, p)			__indirect_writel(v, p)

#define writesb(p, v, l)		__indirect_writesb(p, v, l)
#define writesw(p, v, l)		__indirect_writesw(p, v, l)
#define writesl(p, v, l)		__indirect_writesl(p, v, l)

#define readb(p)			__indirect_readb(p)
#define readw(p)			__indirect_readw(p)
#define readl(p)			__indirect_readl(p)

#define readsb(p, v, l)			__indirect_readsb(p, v, l)
#define readsw(p, v, l)			__indirect_readsw(p, v, l)
#define readsl(p, v, l)			__indirect_readsl(p, v, l)

static inline void __indirect_writeb(u8 value, volatile void __iomem *p)
{
	u32 addr = (u32)p;
	u32 n, byte_enables, data;

	if (!is_pci_memory(addr)) {
		__raw_writeb(value, addr);
		return;
	}

	n = addr % 4;
	byte_enables = (0xf & ~BIT(n)) << IXP4XX_PCI_NP_CBE_BESL;
	data = value << (8*n);
	ixp4xx_pci_write(addr, byte_enables | NP_CMD_MEMWRITE, data);
}

static inline void __indirect_writesb(volatile void __iomem *bus_addr,
				      const u8 *vaddr, int count)
{
	while (count--)
		writeb(*vaddr++, bus_addr);
}

static inline void __indirect_writew(u16 value, volatile void __iomem *p)
{
	u32 addr = (u32)p;
	u32 n, byte_enables, data;

	if (!is_pci_memory(addr)) {
		__raw_writew(value, addr);
		return;
	}

	n = addr % 4;
	byte_enables = (0xf & ~(BIT(n) | BIT(n+1))) << IXP4XX_PCI_NP_CBE_BESL;
	data = value << (8*n);
	ixp4xx_pci_write(addr, byte_enables | NP_CMD_MEMWRITE, data);
}

static inline void __indirect_writesw(volatile void __iomem *bus_addr,
				      const u16 *vaddr, int count)
{
	while (count--)
		writew(*vaddr++, bus_addr);
}

static inline void __indirect_writel(u32 value, volatile void __iomem *p)
{
	u32 addr = (__force u32)p;

	if (!is_pci_memory(addr)) {
		__raw_writel(value, p);
		return;
	}

	ixp4xx_pci_write(addr, NP_CMD_MEMWRITE, value);
}

static inline void __indirect_writesl(volatile void __iomem *bus_addr,
				      const u32 *vaddr, int count)
{
	while (count--)
		writel(*vaddr++, bus_addr);
}

static inline unsigned char __indirect_readb(const volatile void __iomem *p)
{
	u32 addr = (u32)p;
	u32 n, byte_enables, data;

	if (!is_pci_memory(addr))
		return __raw_readb(addr);

	n = addr % 4;
	byte_enables = (0xf & ~BIT(n)) << IXP4XX_PCI_NP_CBE_BESL;
	if (ixp4xx_pci_read(addr, byte_enables | NP_CMD_MEMREAD, &data))
		return 0xff;

	return data >> (8*n);
}

static inline void __indirect_readsb(const volatile void __iomem *bus_addr,
				     u8 *vaddr, u32 count)
{
	while (count--)
		*vaddr++ = readb(bus_addr);
}

static inline unsigned short __indirect_readw(const volatile void __iomem *p)
{
	u32 addr = (u32)p;
	u32 n, byte_enables, data;

	if (!is_pci_memory(addr))
		return __raw_readw(addr);

	n = addr % 4;
	byte_enables = (0xf & ~(BIT(n) | BIT(n+1))) << IXP4XX_PCI_NP_CBE_BESL;
	if (ixp4xx_pci_read(addr, byte_enables | NP_CMD_MEMREAD, &data))
		return 0xffff;

	return data>>(8*n);
}

static inline void __indirect_readsw(const volatile void __iomem *bus_addr,
				     u16 *vaddr, u32 count)
{
	while (count--)
		*vaddr++ = readw(bus_addr);
}

static inline unsigned long __indirect_readl(const volatile void __iomem *p)
{
	u32 addr = (__force u32)p;
	u32 data;

	if (!is_pci_memory(addr))
		return __raw_readl(p);

	if (ixp4xx_pci_read(addr, NP_CMD_MEMREAD, &data))
		return 0xffffffff;

	return data;
}

static inline void __indirect_readsl(const volatile void __iomem *bus_addr,
				     u32 *vaddr, u32 count)
{
	while (count--)
		*vaddr++ = readl(bus_addr);
}


#define memset_io(c,v,l)		_memset_io((c),(v),(l))
#define memcpy_fromio(a,c,l)		_memcpy_fromio((a),(c),(l))
#define memcpy_toio(c,a,l)		_memcpy_toio((c),(a),(l))

#endif /* CONFIG_IXP4XX_INDIRECT_PCI */

#ifndef CONFIG_PCI

#define	__io(v)		__typesafe_io(v)

#else


static inline void outb(u8 value, u32 addr)
{
	u32 n, byte_enables, data;
	n = addr % 4;
	byte_enables = (0xf & ~BIT(n)) << IXP4XX_PCI_NP_CBE_BESL;
	data = value << (8*n);
	ixp4xx_pci_write(addr, byte_enables | NP_CMD_IOWRITE, data);
}

static inline void outsb(u32 io_addr, const u8 *vaddr, u32 count)
{
	while (count--)
		outb(*vaddr++, io_addr);
}

static inline void outw(u16 value, u32 addr)
{
	u32 n, byte_enables, data;
	n = addr % 4;
	byte_enables = (0xf & ~(BIT(n) | BIT(n+1))) << IXP4XX_PCI_NP_CBE_BESL;
	data = value << (8*n);
	ixp4xx_pci_write(addr, byte_enables | NP_CMD_IOWRITE, data);
}

static inline void outsw(u32 io_addr, const u16 *vaddr, u32 count)
{
	while (count--)
		outw(cpu_to_le16(*vaddr++), io_addr);
}

static inline void outl(u32 value, u32 addr)
{
	ixp4xx_pci_write(addr, NP_CMD_IOWRITE, value);
}

static inline void outsl(u32 io_addr, const u32 *vaddr, u32 count)
{
	while (count--)
		outl(cpu_to_le32(*vaddr++), io_addr);
}

static inline u8 inb(u32 addr)
{
	u32 n, byte_enables, data;
	n = addr % 4;
	byte_enables = (0xf & ~BIT(n)) << IXP4XX_PCI_NP_CBE_BESL;
	if (ixp4xx_pci_read(addr, byte_enables | NP_CMD_IOREAD, &data))
		return 0xff;

	return data >> (8*n);
}

static inline void insb(u32 io_addr, u8 *vaddr, u32 count)
{
	while (count--)
		*vaddr++ = inb(io_addr);
}

static inline u16 inw(u32 addr)
{
	u32 n, byte_enables, data;
	n = addr % 4;
	byte_enables = (0xf & ~(BIT(n) | BIT(n+1))) << IXP4XX_PCI_NP_CBE_BESL;
	if (ixp4xx_pci_read(addr, byte_enables | NP_CMD_IOREAD, &data))
		return 0xffff;

	return data>>(8*n);
}

static inline void insw(u32 io_addr, u16 *vaddr, u32 count)
{
	while (count--)
		*vaddr++ = le16_to_cpu(inw(io_addr));
}

static inline u32 inl(u32 addr)
{
	u32 data;
	if (ixp4xx_pci_read(addr, NP_CMD_IOREAD, &data))
		return 0xffffffff;

	return data;
}

static inline void insl(u32 io_addr, u32 *vaddr, u32 count)
{
	while (count--)
		*vaddr++ = le32_to_cpu(inl(io_addr));
}

#define PIO_OFFSET      0x10000UL
#define PIO_MASK        0x0ffffUL

#define	__is_io_address(p)	(((unsigned long)p >= PIO_OFFSET) && \
					((unsigned long)p <= (PIO_MASK + PIO_OFFSET)))

#define	ioread8(p)			ioread8(p)
static inline unsigned int ioread8(const void __iomem *addr)
{
	unsigned long port = (unsigned long __force)addr;
	if (__is_io_address(port))
		return (unsigned int)inb(port & PIO_MASK);
	else
#ifndef CONFIG_IXP4XX_INDIRECT_PCI
		return (unsigned int)__raw_readb(port);
#else
		return (unsigned int)__indirect_readb(addr);
#endif
}

#define	ioread8_rep(p, v, c)		ioread8_rep(p, v, c)
static inline void ioread8_rep(const void __iomem *addr, void *vaddr, u32 count)
{
	unsigned long port = (unsigned long __force)addr;
	if (__is_io_address(port))
		insb(port & PIO_MASK, vaddr, count);
	else
#ifndef	CONFIG_IXP4XX_INDIRECT_PCI
		__raw_readsb(addr, vaddr, count);
#else
		__indirect_readsb(addr, vaddr, count);
#endif
}

#define	ioread16(p)			ioread16(p)
static inline unsigned int ioread16(const void __iomem *addr)
{
	unsigned long port = (unsigned long __force)addr;
	if (__is_io_address(port))
		return	(unsigned int)inw(port & PIO_MASK);
	else
#ifndef CONFIG_IXP4XX_INDIRECT_PCI
		return le16_to_cpu(__raw_readw((u32)port));
#else
		return (unsigned int)__indirect_readw(addr);
#endif
}

#define	ioread16_rep(p, v, c)		ioread16_rep(p, v, c)
static inline void ioread16_rep(const void __iomem *addr, void *vaddr,
				u32 count)
{
	unsigned long port = (unsigned long __force)addr;
	if (__is_io_address(port))
		insw(port & PIO_MASK, vaddr, count);
	else
#ifndef	CONFIG_IXP4XX_INDIRECT_PCI
		__raw_readsw(addr, vaddr, count);
#else
		__indirect_readsw(addr, vaddr, count);
#endif
}

#define	ioread32(p)			ioread32(p)
static inline unsigned int ioread32(const void __iomem *addr)
{
	unsigned long port = (unsigned long __force)addr;
	if (__is_io_address(port))
		return	(unsigned int)inl(port & PIO_MASK);
	else {
#ifndef CONFIG_IXP4XX_INDIRECT_PCI
		return le32_to_cpu((__force __le32)__raw_readl(addr));
#else
		return (unsigned int)__indirect_readl(addr);
#endif
	}
}

#define	ioread32_rep(p, v, c)		ioread32_rep(p, v, c)
static inline void ioread32_rep(const void __iomem *addr, void *vaddr,
				u32 count)
{
	unsigned long port = (unsigned long __force)addr;
	if (__is_io_address(port))
		insl(port & PIO_MASK, vaddr, count);
	else
#ifndef	CONFIG_IXP4XX_INDIRECT_PCI
		__raw_readsl(addr, vaddr, count);
#else
		__indirect_readsl(addr, vaddr, count);
#endif
}

#define	iowrite8(v, p)			iowrite8(v, p)
static inline void iowrite8(u8 value, void __iomem *addr)
{
	unsigned long port = (unsigned long __force)addr;
	if (__is_io_address(port))
		outb(value, port & PIO_MASK);
	else
#ifndef CONFIG_IXP4XX_INDIRECT_PCI
		__raw_writeb(value, port);
#else
		__indirect_writeb(value, addr);
#endif
}

#define	iowrite8_rep(p, v, c)		iowrite8_rep(p, v, c)
static inline void iowrite8_rep(void __iomem *addr, const void *vaddr,
				u32 count)
{
	unsigned long port = (unsigned long __force)addr;
	if (__is_io_address(port))
		outsb(port & PIO_MASK, vaddr, count);
	else
#ifndef CONFIG_IXP4XX_INDIRECT_PCI
		__raw_writesb(addr, vaddr, count);
#else
		__indirect_writesb(addr, vaddr, count);
#endif
}

#define	iowrite16(v, p)			iowrite16(v, p)
static inline void iowrite16(u16 value, void __iomem *addr)
{
	unsigned long port = (unsigned long __force)addr;
	if (__is_io_address(port))
		outw(value, port & PIO_MASK);
	else
#ifndef CONFIG_IXP4XX_INDIRECT_PCI
		__raw_writew(cpu_to_le16(value), addr);
#else
		__indirect_writew(value, addr);
#endif
}

#define	iowrite16_rep(p, v, c)		iowrite16_rep(p, v, c)
static inline void iowrite16_rep(void __iomem *addr, const void *vaddr,
				 u32 count)
{
	unsigned long port = (unsigned long __force)addr;
	if (__is_io_address(port))
		outsw(port & PIO_MASK, vaddr, count);
	else
#ifndef CONFIG_IXP4XX_INDIRECT_PCI
		__raw_writesw(addr, vaddr, count);
#else
		__indirect_writesw(addr, vaddr, count);
#endif
}

#define	iowrite32(v, p)			iowrite32(v, p)
static inline void iowrite32(u32 value, void __iomem *addr)
{
	unsigned long port = (unsigned long __force)addr;
	if (__is_io_address(port))
		outl(value, port & PIO_MASK);
	else
#ifndef CONFIG_IXP4XX_INDIRECT_PCI
		__raw_writel((u32 __force)cpu_to_le32(value), addr);
#else
		__indirect_writel(value, addr);
#endif
}

#define	iowrite32_rep(p, v, c)		iowrite32_rep(p, v, c)
static inline void iowrite32_rep(void __iomem *addr, const void *vaddr,
				 u32 count)
{
	unsigned long port = (unsigned long __force)addr;
	if (__is_io_address(port))
		outsl(port & PIO_MASK, vaddr, count);
	else
#ifndef CONFIG_IXP4XX_INDIRECT_PCI
		__raw_writesl(addr, vaddr, count);
#else
		__indirect_writesl(addr, vaddr, count);
#endif
}

#define	ioport_map(port, nr)		((void __iomem*)(port + PIO_OFFSET))
#define	ioport_unmap(addr)
#endif /* CONFIG_PCI */

#endif /* __ASM_ARM_ARCH_IO_H */
