
#ifndef __ASM_ARCH_IRQS_H
#define __ASM_ARCH_IRQS_H

/* NetSilicon 9360 */
#define IRQ_NS9XXX_WATCHDOG	0
#define IRQ_NS9XXX_AHBBUSERR	1
#define IRQ_NS9360_BBUSAGG	2
/* irq 3 is reserved for NS9360 */
#define IRQ_NS9XXX_ETHRX	4
#define IRQ_NS9XXX_ETHTX	5
#define IRQ_NS9XXX_ETHPHY	6
#define IRQ_NS9360_LCD		7
#define IRQ_NS9360_SERBRX	8
#define IRQ_NS9360_SERBTX	9
#define IRQ_NS9360_SERARX	10
#define IRQ_NS9360_SERATX	11
#define IRQ_NS9360_SERCRX	12
#define IRQ_NS9360_SERCTX	13
#define IRQ_NS9360_I2C		14
#define IRQ_NS9360_BBUSDMA	15
#define IRQ_NS9360_TIMER0	16
#define IRQ_NS9360_TIMER1	17
#define IRQ_NS9360_TIMER2	18
#define IRQ_NS9360_TIMER3	19
#define IRQ_NS9360_TIMER4	20
#define IRQ_NS9360_TIMER5	21
#define IRQ_NS9360_TIMER6	22
#define IRQ_NS9360_TIMER7	23
#define IRQ_NS9360_RTC		24
#define IRQ_NS9360_USBHOST	25
#define IRQ_NS9360_USBDEVICE	26
#define IRQ_NS9360_IEEE1284	27
#define IRQ_NS9XXX_EXT0		28
#define IRQ_NS9XXX_EXT1		29
#define IRQ_NS9XXX_EXT2		30
#define IRQ_NS9XXX_EXT3		31

#define BBUS_IRQ(irq)	(32 + irq)

#define IRQ_BBUS_DMA		BBUS_IRQ(0)
#define IRQ_BBUS_SERBRX		BBUS_IRQ(2)
#define IRQ_BBUS_SERBTX		BBUS_IRQ(3)
#define IRQ_BBUS_SERARX		BBUS_IRQ(4)
#define IRQ_BBUS_SERATX		BBUS_IRQ(5)
#define IRQ_BBUS_SERCRX		BBUS_IRQ(6)
#define IRQ_BBUS_SERCTX		BBUS_IRQ(7)
#define IRQ_BBUS_SERDRX		BBUS_IRQ(8)
#define IRQ_BBUS_SERDTX		BBUS_IRQ(9)
#define IRQ_BBUS_I2C		BBUS_IRQ(10)
#define IRQ_BBUS_1284		BBUS_IRQ(11)
#define IRQ_BBUS_UTIL		BBUS_IRQ(12)
#define IRQ_BBUS_RTC		BBUS_IRQ(13)
#define IRQ_BBUS_USBHST		BBUS_IRQ(14)
#define IRQ_BBUS_USBDEV		BBUS_IRQ(15)
#define IRQ_BBUS_AHBDMA1	BBUS_IRQ(24)
#define IRQ_BBUS_AHBDMA2	BBUS_IRQ(25)

#define FPGA_IRQ(irq)	(64 + irq)

#define IRQ_FPGA_UARTA		FPGA_IRQ(0)
#define IRQ_FPGA_UARTB		FPGA_IRQ(1)
#define IRQ_FPGA_UARTC		FPGA_IRQ(2)
#define IRQ_FPGA_UARTD		FPGA_IRQ(3)
#define IRQ_FPGA_TOUCH		FPGA_IRQ(4)
#define IRQ_FPGA_CF		FPGA_IRQ(5)
#define IRQ_FPGA_CAN0		FPGA_IRQ(6)
#define IRQ_FPGA_CAN1		FPGA_IRQ(7)

#define NR_IRQS	72

#endif /* __ASM_ARCH_IRQS_H */
