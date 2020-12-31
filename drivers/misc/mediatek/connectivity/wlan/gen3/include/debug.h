


#ifndef _DEBUG_H
#define _DEBUG_H

#ifndef BUILD_QA_DBG
#define BUILD_QA_DBG 0
#endif

#define DBG_DISABLE_ALL_LOG             0

#include "gl_typedef.h"

extern UINT_8 aucDebugModule[];

#define DBG_CLASS_ERROR         BIT(0)
#define DBG_CLASS_WARN          BIT(1)
#define DBG_CLASS_STATE         BIT(2)
#define DBG_CLASS_EVENT         BIT(3)
#define DBG_CLASS_TRACE         BIT(4)
#define DBG_CLASS_INFO          BIT(5)
#define DBG_CLASS_LOUD          BIT(6)
#define DBG_CLASS_TEMP          BIT(7)
#define DBG_CLASS_MASK          BITS(0, 7)

#if defined(LINUX)
#define DBG_PRINTF_64BIT_DEC    "lld"
#else /* Windows */
#define DBG_PRINTF_64BIT_DEC    "I64d"
#endif

#define DBG_ALL_MODULE_IDX      0xFFFFFFFF

/* Define debug module index */
typedef enum _ENUM_DBG_MODULE_T {
	DBG_INIT_IDX = 0,	/* 0x00 */ /* For driver initial */
	DBG_HAL_IDX,		/* 0x01 */ /* For HAL(HW) Layer */
	DBG_INTR_IDX,		/* 0x02 */ /* For Interrupt */
	DBG_REQ_IDX,		/* 0x03 */
	DBG_TX_IDX,		/* 0x04 */
	DBG_RX_IDX,		/* 0x05 */
	DBG_RFTEST_IDX,		/* 0x06 */ /* For RF test mode */
	DBG_EMU_IDX,		/* 0x07 */ /* Developer specific */

	DBG_SW1_IDX,		/* 0x08 */ /* Developer specific */
	DBG_SW2_IDX,		/* 0x09 */ /* Developer specific */
	DBG_SW3_IDX,		/* 0x0A */ /* Developer specific */
	DBG_SW4_IDX,		/* 0x0B */ /* Developer specific */

	DBG_HEM_IDX,		/* 0x0C */ /* HEM */
	DBG_AIS_IDX,		/* 0x0D */ /* AIS */
	DBG_RLM_IDX,		/* 0x0E */ /* RLM */
	DBG_MEM_IDX,		/* 0x0F */ /* RLM */
	DBG_CNM_IDX,		/* 0x10 */ /* CNM */
	DBG_RSN_IDX,		/* 0x11 */ /* RSN */
	DBG_BSS_IDX,		/* 0x12 */ /* BSS */
	DBG_SCN_IDX,		/* 0x13 */ /* SCN */
	DBG_SAA_IDX,		/* 0x14 */ /* SAA */
	DBG_AAA_IDX,		/* 0x15 */ /* AAA */
	DBG_P2P_IDX,		/* 0x16 */ /* P2P */
	DBG_QM_IDX,		/* 0x17 */ /* QUE_MGT */
	DBG_SEC_IDX,		/* 0x18 */ /* SEC */
	DBG_BOW_IDX,		/* 0x19 */ /* BOW */
	DBG_WAPI_IDX,		/* 0x1A */ /* WAPI */
	DBG_ROAMING_IDX,	/* 0x1B */ /* ROAMING */
	DBG_TDLS_IDX,		/* 0x1C */ /* TDLS */ /* CFG_SUPPORT_TDLS */
	DBG_OID_IDX,
	DBG_HS20_IDX,           /* 0x1E */ /* HotSpot 2.0 */
	DBG_NIC_IDX,
	DBG_MODULE_NUM		/* Notice the XLOG check */
} ENUM_DBG_MODULE_T;



/* Debug print format string for the OS system time */
#define OS_SYSTIME_DBG_FORMAT               "0x%08x"

/* Debug print argument for the OS system time */
#define OS_SYSTIME_DBG_ARGUMENT(systime)    (systime)

/* Debug print format string for the MAC Address */
#define MACSTR          "%pM"
/* "%02x:%02x:%02x:%02x:%02x:%02x" */

/* Debug print argument for the MAC Address */
#define MAC2STR(a)	a
/* ((PUINT_8)a)[0], ((PUINT_8)a)[1], ((PUINT_8)a)[2], ((PUINT_8)a)[3], ((PUINT_8)a)[4], ((PUINT_8)a)[5] */

/* Debug print format string for the IPv4 Address */
#define IPV4STR         "%pI4"
/* "%u.%u.%u.%u" */

/* Debug print argument for the IPv4 Address */
#define IPV4TOSTR(a)    a
/* ((PUINT_8)a)[0], ((PUINT_8)a)[1], ((PUINT_8)a)[2], ((PUINT_8)a)[3] */

/* Debug print format string for the MAC Address */
#define IPV6STR         "%pI6"
/* "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x" */

/* Debug print argument for the MAC Address */
#define IPV6TOSTR(a)    a

/* The pre-defined format to dump the value of a varaible with its name shown. */
#define DUMPVAR(variable, format)           (#variable " = " format "\n", variable)

/* The pre-defined format to dump the MAC type value with its name shown. */
#define DUMPMACADDR(addr)                   (#addr " = " MACSTR "\n", MAC2STR(addr))

#define LOG_FUNC                kalPrint

/* If __FUNCTION__ is already defined by compiler, we just use it. */
#define DEBUGFUNC(_Func)

#if DBG_DISABLE_ALL_LOG
#define DBGLOG(_Module, _Class, _Fmt)
#define DBGLOG_MEM8(_Module, _Class, _StartAddr, _Length)
#define DBGLOG_MEM32(_Module, _Class, _StartAddr, _Length)
#else
#define DBGLOG(_Module, _Class, _Fmt, ...) \
	do { \
		if ((aucDebugModule[DBG_##_Module##_IDX] & DBG_CLASS_##_Class) == 0) \
			break; \
		LOG_FUNC("%s:(" #_Module " " #_Class ") " _Fmt, __func__, ##__VA_ARGS__); \
	} while (0)

#define DBGLOG_MEM8(_Module, _Class, _StartAddr, _Length) \
	{ \
		if (aucDebugModule[DBG_##_Module##_IDX] & DBG_CLASS_##_Class) { \
			LOG_FUNC("%s:(" #_Module " " #_Class ")\n", __func__); \
			dumpMemory8((PUINT_8) (_StartAddr), (UINT_32) (_Length)); \
		} \
	}

#define DBGLOG_MEM32(_Module, _Class, _StartAddr, _Length) \
	{ \
		if (aucDebugModule[DBG_##_Module##_IDX] & DBG_CLASS_##_Class) { \
			LOG_FUNC("%s:(" #_Module " " #_Class ")\n", __func__); \
			dumpMemory32((PUINT_32) (_StartAddr), (UINT_32) (_Length)); \
		} \
	}
#endif

#define DISP_STRING(_str)       _str

#undef ASSERT
#undef ASSERT_REPORT

#if (BUILD_QA_DBG || DBG)
#ifdef _lint
#define ASSERT_NOMEM()
#define ASSERT(_exp) \
	{ \
		if (!(_exp)) { \
			do {} while (1); \
		} \
	}

#define ASSERT_REPORT(_exp, _fmt) \
	{ \
		LOG_FUNC("Assertion failed: %s:%d (%s)\n", __FILE__, __LINE__, #_exp); \
		LOG_FUNC _fmt; \
		if (!(_exp)) { \
			do {} while (1); \
		} \
	}
#elif defined(WINDOWS_CE)
#define ASSERT_NOMEM()
#define UNICODE_TEXT(_msg)  TEXT(_msg)
#define ASSERT(_exp) \
	{ \
		if (!(_exp) && !fgIsBusAccessFailed) { \
			TCHAR rUbuf[256]; \
			kalBreakPoint(); \
			_stprintf(rUbuf, TEXT("Assertion failed: %s:%d %s\n"), \
				  UNICODE_TEXT(__FILE__), __LINE__, UNICODE_TEXT(#_exp)); \
			MessageBox(NULL, rUbuf, TEXT("ASSERT!"), MB_OK); \
		} \
	}

#define ASSERT_REPORT(_exp, _fmt) \
	{ \
		if (!(_exp) && !fgIsBusAccessFailed) { \
			TCHAR rUbuf[256]; \
			kalBreakPoint(); \
			_stprintf(rUbuf, TEXT("Assertion failed: %s:%d %s\n"), \
				  UNICODE_TEXT(__FILE__), __LINE__, UNICODE_TEXT(#_exp)); \
			MessageBox(NULL, rUbuf, TEXT("ASSERT!"), MB_OK); \
		} \
	}
#else
#define ASSERT_NOMEM() \
	{ \
		LOG_FUNC("alloate memory failed at %s:%d\n", __FILE__, __LINE__); \
		kalSendAeeWarning("Wlan_Gen3 No Mem", "Memory Alloate Failed %s:%d",\
				  __FILE__, __LINE__); \
	}

#define ASSERT(_exp) \
	{ \
		if (!(_exp) && !fgIsBusAccessFailed) { \
			LOG_FUNC("Assertion failed: %s:%d (%s)\n", __FILE__, __LINE__, #_exp); \
			kalBreakPoint(); \
		} \
	}

#define ASSERT_REPORT(_exp, _fmt) \
	{ \
		if (!(_exp) && !fgIsBusAccessFailed) { \
			LOG_FUNC("Assertion failed: %s:%d (%s)\n", __FILE__, __LINE__, #_exp); \
			LOG_FUNC _fmt; \
			kalBreakPoint(); \
		} \
	}
#endif /* WINDOWS_CE */
#else
#define ASSERT_NOMEM()
#define ASSERT(_exp)
#define ASSERT_REPORT(_exp, _fmt)
#endif /* BUILD_QA_DBG */

/* The following macro is used for debugging packed structures. */
#ifndef DATA_STRUCT_INSPECTING_ASSERT
#define DATA_STRUCT_INSPECTING_ASSERT(expr) \
{ \
	switch (0) {case 0: case (expr): default:; } \
}
#endif


VOID dumpMemory8(IN PUINT_8 pucStartAddr, IN UINT_32 u4Length);

VOID dumpMemory32(IN PUINT_32 pu4StartAddr, IN UINT_32 u4Length);

VOID wlanPrintFwLog(PUINT_8 pucLogContent, UINT_16 u2MsgSize, UINT_8 ucMsgType);

#endif /* _DEBUG_H */
