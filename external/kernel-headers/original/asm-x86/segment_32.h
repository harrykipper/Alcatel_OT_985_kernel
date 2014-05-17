#ifndef _ASM_SEGMENT_H
#define _ASM_SEGMENT_H

#define GDT_ENTRY_TLS_ENTRIES	3
#define GDT_ENTRY_TLS_MIN	6
#define GDT_ENTRY_TLS_MAX 	(GDT_ENTRY_TLS_MIN + GDT_ENTRY_TLS_ENTRIES - 1)

#define TLS_SIZE (GDT_ENTRY_TLS_ENTRIES * 8)

#define GDT_ENTRY_DEFAULT_USER_CS	14
#define __USER_CS (GDT_ENTRY_DEFAULT_USER_CS * 8 + 3)

#define GDT_ENTRY_DEFAULT_USER_DS	15
#define __USER_DS (GDT_ENTRY_DEFAULT_USER_DS * 8 + 3)

#define GDT_ENTRY_KERNEL_BASE	12

#define GDT_ENTRY_KERNEL_CS		(GDT_ENTRY_KERNEL_BASE + 0)
#define __KERNEL_CS (GDT_ENTRY_KERNEL_CS * 8)

#define GDT_ENTRY_KERNEL_DS		(GDT_ENTRY_KERNEL_BASE + 1)
#define __KERNEL_DS (GDT_ENTRY_KERNEL_DS * 8)

#define GDT_ENTRY_TSS			(GDT_ENTRY_KERNEL_BASE + 4)
#define GDT_ENTRY_LDT			(GDT_ENTRY_KERNEL_BASE + 5)

#define GDT_ENTRY_PNPBIOS_BASE		(GDT_ENTRY_KERNEL_BASE + 6)
#define GDT_ENTRY_APMBIOS_BASE		(GDT_ENTRY_KERNEL_BASE + 11)

#define GDT_ENTRY_ESPFIX_SS		(GDT_ENTRY_KERNEL_BASE + 14)
#define __ESPFIX_SS (GDT_ENTRY_ESPFIX_SS * 8)

#define GDT_ENTRY_PERCPU			(GDT_ENTRY_KERNEL_BASE + 15)
#ifdef CONFIG_SMP
#define __KERNEL_PERCPU (GDT_ENTRY_PERCPU * 8)
#else
#define __KERNEL_PERCPU 0
#endif

#define GDT_ENTRY_DOUBLEFAULT_TSS	31

#define GDT_ENTRIES 32
#define GDT_SIZE (GDT_ENTRIES * 8)

/* Simple and small GDT entries for booting only */

#define GDT_ENTRY_BOOT_CS		2
#define __BOOT_CS	(GDT_ENTRY_BOOT_CS * 8)

#define GDT_ENTRY_BOOT_DS		(GDT_ENTRY_BOOT_CS + 1)
#define __BOOT_DS	(GDT_ENTRY_BOOT_DS * 8)

/* The PnP BIOS entries in the GDT */
#define GDT_ENTRY_PNPBIOS_CS32		(GDT_ENTRY_PNPBIOS_BASE + 0)
#define GDT_ENTRY_PNPBIOS_CS16		(GDT_ENTRY_PNPBIOS_BASE + 1)
#define GDT_ENTRY_PNPBIOS_DS		(GDT_ENTRY_PNPBIOS_BASE + 2)
#define GDT_ENTRY_PNPBIOS_TS1		(GDT_ENTRY_PNPBIOS_BASE + 3)
#define GDT_ENTRY_PNPBIOS_TS2		(GDT_ENTRY_PNPBIOS_BASE + 4)

/* The PnP BIOS selectors */
#define PNP_CS32   (GDT_ENTRY_PNPBIOS_CS32 * 8)	/* segment for calling fn */
#define PNP_CS16   (GDT_ENTRY_PNPBIOS_CS16 * 8)	/* code segment for BIOS */
#define PNP_DS     (GDT_ENTRY_PNPBIOS_DS * 8)	/* data segment for BIOS */
#define PNP_TS1    (GDT_ENTRY_PNPBIOS_TS1 * 8)	/* transfer data segment */
#define PNP_TS2    (GDT_ENTRY_PNPBIOS_TS2 * 8)	/* another data segment */

#define IDT_ENTRIES 256

/* Bottom two bits of selector give the ring privilege level */
#define SEGMENT_RPL_MASK	0x3
/* Bit 2 is table indicator (LDT/GDT) */
#define SEGMENT_TI_MASK		0x4

/* User mode is privilege level 3 */
#define USER_RPL		0x3
/* LDT segment has TI set, GDT has it cleared */
#define SEGMENT_LDT		0x4
#define SEGMENT_GDT		0x0

#ifndef CONFIG_PARAVIRT
#define get_kernel_rpl()  0
#endif

/* Matches only __KERNEL_CS, ignoring PnP / USER / APM segments */
#define SEGMENT_IS_KERNEL_CODE(x) (((x) & 0xfc) == GDT_ENTRY_KERNEL_CS * 8)

/* Matches __KERNEL_CS and __USER_CS (they must be 2 entries apart) */
#define SEGMENT_IS_FLAT_CODE(x)  (((x) & 0xec) == GDT_ENTRY_KERNEL_CS * 8)

/* Matches PNP_CS32 and PNP_CS16 (they must be consecutive) */
#define SEGMENT_IS_PNP_CODE(x)   (((x) & 0xf4) == GDT_ENTRY_PNPBIOS_BASE * 8)

#endif
