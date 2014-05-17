#ifndef _LINUX_BYTEORDER_SWABB_H
#define _LINUX_BYTEORDER_SWABB_H



#define ___swahw32(x) \
({ \
	__u32 __x = (x); \
	((__u32)( \
		(((__u32)(__x) & (__u32)0x0000ffffUL) << 16) | \
		(((__u32)(__x) & (__u32)0xffff0000UL) >> 16) )); \
})
#define ___swahb32(x) \
({ \
	__u32 __x = (x); \
	((__u32)( \
		(((__u32)(__x) & (__u32)0x00ff00ffUL) << 8) | \
		(((__u32)(__x) & (__u32)0xff00ff00UL) >> 8) )); \
})

#define ___constant_swahw32(x) \
	((__u32)( \
		(((__u32)(x) & (__u32)0x0000ffffUL) << 16) | \
		(((__u32)(x) & (__u32)0xffff0000UL) >> 16) ))
#define ___constant_swahb32(x) \
	((__u32)( \
		(((__u32)(x) & (__u32)0x00ff00ffUL) << 8) | \
		(((__u32)(x) & (__u32)0xff00ff00UL) >> 8) ))

#ifndef __arch__swahw32
#  define __arch__swahw32(x) ___swahw32(x)
#endif
#ifndef __arch__swahb32
#  define __arch__swahb32(x) ___swahb32(x)
#endif

#ifndef __arch__swahw32p
#  define __arch__swahw32p(x) __swahw32(*(x))
#endif
#ifndef __arch__swahb32p
#  define __arch__swahb32p(x) __swahb32(*(x))
#endif

#ifndef __arch__swahw32s
#  define __arch__swahw32s(x) do { *(x) = __swahw32p((x)); } while (0)
#endif
#ifndef __arch__swahb32s
#  define __arch__swahb32s(x) do { *(x) = __swahb32p((x)); } while (0)
#endif


#if defined(__GNUC__) && defined(__OPTIMIZE__)
#  define __swahw32(x) \
(__builtin_constant_p((__u32)(x)) ? \
 ___swahw32((x)) : \
 __fswahw32((x)))
#  define __swahb32(x) \
(__builtin_constant_p((__u32)(x)) ? \
 ___swahb32((x)) : \
 __fswahb32((x)))
#else
#  define __swahw32(x) __fswahw32(x)
#  define __swahb32(x) __fswahb32(x)
#endif /* OPTIMIZE */


static inline __u32 __fswahw32(__u32 x)
{
	return __arch__swahw32(x);
}

static inline __u32 __swahw32p(__u32 *x)
{
	return __arch__swahw32p(x);
}

static inline void __swahw32s(__u32 *addr)
{
	__arch__swahw32s(addr);
}

static inline __u32 __fswahb32(__u32 x)
{
	return __arch__swahb32(x);
}

static inline __u32 __swahb32p(__u32 *x)
{
	return __arch__swahb32p(x);
}

static inline void __swahb32s(__u32 *addr)
{
	__arch__swahb32s(addr);
}

#ifdef __BYTEORDER_HAS_U64__
#endif /* __BYTEORDER_HAS_U64__ */

#if defined(__KERNEL__)
#define swahw32 __swahw32
#define swahb32 __swahb32
#define swahw32p __swahw32p
#define swahb32p __swahb32p
#define swahw32s __swahw32s
#define swahb32s __swahb32s
#endif

#endif /* _LINUX_BYTEORDER_SWABB_H */
