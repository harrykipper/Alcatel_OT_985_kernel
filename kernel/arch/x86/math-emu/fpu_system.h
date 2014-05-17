

#ifndef _FPU_SYSTEM_H
#define _FPU_SYSTEM_H

/* system dependent definitions */

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>

#define LDT_DESCRIPTOR(s)	(((struct desc_struct *)current->mm->context.ldt)[(s) >> 3])
#define SEG_D_SIZE(x)		((x).b & (3 << 21))
#define SEG_G_BIT(x)		((x).b & (1 << 23))
#define SEG_GRANULARITY(x)	(((x).b & (1 << 23)) ? 4096 : 1)
#define SEG_286_MODE(x)		((x).b & ( 0xff000000 | 0xf0000 | (1 << 23)))
#define SEG_BASE_ADDR(s)	(((s).b & 0xff000000) \
				 | (((s).b & 0xff) << 16) | ((s).a >> 16))
#define SEG_LIMIT(s)		(((s).b & 0xff0000) | ((s).a & 0xffff))
#define SEG_EXECUTE_ONLY(s)	(((s).b & ((1 << 11) | (1 << 9))) == (1 << 11))
#define SEG_WRITE_PERM(s)	(((s).b & ((1 << 11) | (1 << 9))) == (1 << 9))
#define SEG_EXPAND_DOWN(s)	(((s).b & ((1 << 11) | (1 << 10))) \
				 == (1 << 10))

#define I387			(current->thread.fpu.state)
#define FPU_info		(I387->soft.info)

#define FPU_CS			(*(unsigned short *) &(FPU_info->regs->cs))
#define FPU_SS			(*(unsigned short *) &(FPU_info->regs->ss))
#define FPU_DS			(*(unsigned short *) &(FPU_info->regs->ds))
#define FPU_EAX			(FPU_info->regs->ax)
#define FPU_EFLAGS		(FPU_info->regs->flags)
#define FPU_EIP			(FPU_info->regs->ip)
#define FPU_ORIG_EIP		(FPU_info->___orig_eip)

#define FPU_lookahead           (I387->soft.lookahead)

#define no_ip_update		(*(u_char *)&(I387->soft.no_update))
#define FPU_rm			(*(u_char *)&(I387->soft.rm))

#define access_limit		(*(u_char *)&(I387->soft.alimit))

#define partial_status		(I387->soft.swd)
#define control_word		(I387->soft.cwd)
#define fpu_tag_word		(I387->soft.twd)
#define registers		(I387->soft.st_space)
#define top			(I387->soft.ftop)

#define instruction_address	(*(struct address *)&I387->soft.fip)
#define operand_address		(*(struct address *)&I387->soft.foo)

#define FPU_access_ok(x,y,z)	if ( !access_ok(x,y,z) ) \
				math_abort(FPU_info,SIGSEGV)
#define FPU_abort		math_abort(FPU_info, SIGSEGV)

#undef FPU_IGNORE_CODE_SEGV
#ifdef FPU_IGNORE_CODE_SEGV
#define	FPU_code_access_ok(z)
#else
#define	FPU_code_access_ok(z) FPU_access_ok(VERIFY_READ,(void __user *)FPU_EIP,z)
#endif

#define FPU_get_user(x,y)       get_user((x),(y))
#define FPU_put_user(x,y)       put_user((x),(y))

#endif
