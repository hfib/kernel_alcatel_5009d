


#ifndef _PWR_MGT_H
#define _PWR_MGT_H


#define PM_UAPSD_AC0                        (BIT(0))
#define PM_UAPSD_AC1                        (BIT(1))
#define PM_UAPSD_AC2                        (BIT(2))
#define PM_UAPSD_AC3                        (BIT(3))

#define PM_UAPSD_ALL                        (PM_UAPSD_AC0 | PM_UAPSD_AC1 | PM_UAPSD_AC2 | PM_UAPSD_AC3)
#define PM_UAPSD_NONE                       0

typedef struct _PM_PROFILE_SETUP_INFO_T {
	/* Profile setup */
	UINT_8 ucBmpDeliveryAC;	/* 0: AC_BE, 1: AC_BK, 2: AC_VI, 3: AC_VO */
	UINT_8 ucBmpTriggerAC;	/* 0: AC_BE, 1: AC_BK, 2: AC_VI, 3: AC_VO */

	UINT_8 ucUapsdSp;	/* Number of triggered packets in UAPSD */

} PM_PROFILE_SETUP_INFO_T, *P_PM_PROFILE_SETUP_INFO_T;




#if !CFG_ENABLE_FULL_PM
#define ACQUIRE_POWER_CONTROL_FROM_PM(_prAdapter)
#define RECLAIM_POWER_CONTROL_TO_PM(_prAdapter, _fgEnableGINT_in_IST)
#else
#define ACQUIRE_POWER_CONTROL_FROM_PM(_prAdapter) \
	{ \
		if (_prAdapter->fgIsFwOwn) { \
			nicpmSetDriverOwn(_prAdapter); \
		} \
		/* Increase Block to Enter Low Power Semaphore count */ \
		GLUE_INC_REF_CNT(_prAdapter->u4PwrCtrlBlockCnt); \
	}

#define RECLAIM_POWER_CONTROL_TO_PM(_prAdapter, _fgEnableGINT_in_IST) \
	{ \
		ASSERT(_prAdapter->u4PwrCtrlBlockCnt != 0); \
		/* Decrease Block to Enter Low Power Semaphore count */ \
		GLUE_DEC_REF_CNT(_prAdapter->u4PwrCtrlBlockCnt); \
		if (_prAdapter->fgWiFiInSleepyState && (_prAdapter->u4PwrCtrlBlockCnt == 0)) { \
			nicpmSetFWOwn(_prAdapter, _fgEnableGINT_in_IST); \
		} \
	}
#endif



#endif /* _PWR_MGT_H */
