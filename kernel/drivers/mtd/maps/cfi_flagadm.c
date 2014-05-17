

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>



#define FLASH_PHYS_ADDR 0x40000000
#define FLASH_SIZE 0x400000

#define FLASH_PARTITION0_ADDR 0x00000000
#define FLASH_PARTITION0_SIZE 0x00020000

#define FLASH_PARTITION1_ADDR 0x00020000
#define FLASH_PARTITION1_SIZE 0x000A0000

#define FLASH_PARTITION2_ADDR 0x000C0000
#define FLASH_PARTITION2_SIZE 0x00180000

#define FLASH_PARTITION3_ADDR 0x00240000
#define FLASH_PARTITION3_SIZE 0x001C0000


struct map_info flagadm_map = {
		.name =		"FlagaDM flash device",
		.size =		FLASH_SIZE,
		.bankwidth =	2,
};

struct mtd_partition flagadm_parts[] = {
	{
		.name =		"Bootloader",
		.offset	=	FLASH_PARTITION0_ADDR,
		.size =		FLASH_PARTITION0_SIZE
	},
	{
		.name =		"Kernel image",
		.offset =	FLASH_PARTITION1_ADDR,
		.size =		FLASH_PARTITION1_SIZE
	},
	{
		.name =		"Initial ramdisk image",
		.offset =	FLASH_PARTITION2_ADDR,
		.size =		FLASH_PARTITION2_SIZE
	},
	{
		.name =		"Persistent storage",
		.offset =	FLASH_PARTITION3_ADDR,
		.size =		FLASH_PARTITION3_SIZE
	}
};

#define PARTITION_COUNT ARRAY_SIZE(flagadm_parts)

static struct mtd_info *mymtd;

static int __init init_flagadm(void)
{
	printk(KERN_NOTICE "FlagaDM flash device: %x at %x\n",
			FLASH_SIZE, FLASH_PHYS_ADDR);

	flagadm_map.phys = FLASH_PHYS_ADDR;
	flagadm_map.virt = ioremap(FLASH_PHYS_ADDR,
					FLASH_SIZE);

	if (!flagadm_map.virt) {
		printk("Failed to ioremap\n");
		return -EIO;
	}

	simple_map_init(&flagadm_map);

	mymtd = do_map_probe("cfi_probe", &flagadm_map);
	if (mymtd) {
		mymtd->owner = THIS_MODULE;
		add_mtd_partitions(mymtd, flagadm_parts, PARTITION_COUNT);
		printk(KERN_NOTICE "FlagaDM flash device initialized\n");
		return 0;
	}

	iounmap((void *)flagadm_map.virt);
	return -ENXIO;
}

static void __exit cleanup_flagadm(void)
{
	if (mymtd) {
		del_mtd_partitions(mymtd);
		map_destroy(mymtd);
	}
	if (flagadm_map.virt) {
		iounmap((void *)flagadm_map.virt);
		flagadm_map.virt = 0;
	}
}

module_init(init_flagadm);
module_exit(cleanup_flagadm);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kári Davíðsson <kd@flaga.is>");
MODULE_DESCRIPTION("MTD map driver for Flaga digital module");
