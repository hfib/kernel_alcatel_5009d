


#ifndef _GL_TYPEDEF_H
#define _GL_TYPEDEF_H


#include "config.h"

#if CFG_ENABLE_EARLY_SUSPEND
#include <linux/earlysuspend.h>
#endif



/* Define HZ of timer tick for function kalGetTimeTick() */
#define KAL_HZ                  (1000)

/* Miscellaneous Equates */
#ifndef FALSE
#define FALSE               ((BOOL) 0)
#define TRUE                ((BOOL) 1)
#endif /* FALSE */

#ifndef NULL
#if defined(__cplusplus)
#define NULL            0
#else
#define NULL            ((void *) 0)
#endif
#endif

#if CFG_ENABLE_EARLY_SUSPEND
typedef void (*early_suspend_callback) (struct early_suspend *h);
typedef void (*late_resume_callback) (struct early_suspend *h);
#endif

/* Type definition for void */
typedef void VOID, *PVOID, **PPVOID;

/* Type definition for Boolean */
typedef unsigned char BOOL, *PBOOL, BOOLEAN, *PBOOLEAN;

/* Type definition for signed integers */
typedef signed char CHAR, *PCHAR, **PPCHAR;
typedef signed char INT_8, *PINT_8, **PPINT_8;
typedef signed short INT_16, *PINT_16, **PPINT_16;
typedef signed int INT_32, *PINT_32, **PPINT_32;
typedef unsigned int UINT32, *PUINT32, **PPUINT32;
typedef signed long LONG, *PLONG, **PPLONG;
typedef signed long long INT_64, *PINT_64, **PPINT_64;

/* Type definition for unsigned integers */
typedef unsigned char UCHAR, *PUCHAR, **PPUCHAR;
typedef unsigned char UINT_8, *PUINT_8, **PPUINT_8, *P_UINT_8;
typedef unsigned short UINT_16, *PUINT_16, **PPUINT_16;
typedef unsigned int UINT_32, *PUINT_32, **PPUINT_32;
typedef unsigned long ULONG, *PULONG, *PPULONG;
typedef unsigned long long UINT_64, *PUINT_64, **PPUINT_64;

typedef unsigned int OS_SYSTIME, *POS_SYSTIME, **PPOS_SYSTIME;

typedef union _LARGE_INTEGER {
	struct {
		UINT_32 LowPart;
		INT_32 HighPart;
	} u;
	INT_64 QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
	struct {
		UINT_32 LowPart;
		UINT_32 HighPart;
	} u;
	UINT_64 QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

typedef INT_32(*probe_card) (PVOID pvData);
typedef VOID(*remove_card) (VOID);



#define IN			/* volatile */
#define OUT			/* volatile */

#define __KAL_INLINE__                  inline
#define __KAL_ATTRIB_PACKED__           __attribute__((__packed__))
#define __KAL_ATTRIB_ALIGN_4__          __aligned(4)

#ifndef BIT
#define BIT(n)                          ((UINT_32) 1UL << (n))
#endif /* BIT */

#ifndef BITS
#define BITS(m, n)                       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#endif /* BIT */

#ifndef OFFSET_OF
#define OFFSET_OF(_type, _field)    ((unsigned long)&(((_type *)0)->_field))
#endif /* OFFSET_OF */

#ifndef ENTRY_OF
#define ENTRY_OF(_addrOfField, _type, _field) \
	((_type *)((PINT_8)(_addrOfField) - (PINT_8)OFFSET_OF(_type, _field)))
#endif /* ENTRY_OF */

#ifndef ALIGN_4
#define ALIGN_4(_value)             (((_value) + 3) & ~3u)
#endif /* ALIGN_4 */

#ifndef IS_ALIGN_4
#define IS_ALIGN_4(_value)          (((_value) & 0x3) ? FALSE : TRUE)
#endif /* IS_ALIGN_4 */

#ifndef IS_NOT_ALIGN_4
#define IS_NOT_ALIGN_4(_value)      (((_value) & 0x3) ? TRUE : FALSE)
#endif /* IS_NOT_ALIGN_4 */

#ifndef BYTE_TO_DWORD
#define BYTE_TO_DWORD(_value)       ((_value + 3) >> 2)
#endif /* BYTE_TO_DWORD */

#ifndef DWORD_TO_BYTE
#define DWORD_TO_BYTE(_value)       ((_value) << 2)
#endif /* DWORD_TO_BYTE */

#if 1				/* Little-Endian */
#define NTOHS(_x)           ntohs(_x)

#define HTONS(_x)           htons(_x)

#define NTOHL(_x)           ntohl(_x)

#define HTONL(_x)           htonl(_x)

#else /* Big-Endian */

#define CONST_NTOHS(_x)

#define CONST_HTONS(_x)

#define NTOHS(_x)

#define HTONS(_x)

#endif



#endif /* _GL_TYPEDEF_H */
