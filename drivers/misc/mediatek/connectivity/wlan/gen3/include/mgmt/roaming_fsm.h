


#ifndef _ROAMING_FSM_H
#define _ROAMING_FSM_H



/* Roaming Discovery interval, SCAN result need to be updated */
#define ROAMING_DISCOVERY_TIMEOUT_SEC               5	/* Seconds. */

/* #define ROAMING_NO_SWING_RCPI_STEP                  5 //rcpi */
typedef enum _ENUM_ROAMING_FAIL_REASON_T {
	ROAMING_FAIL_REASON_CONNLIMIT = 0,
	ROAMING_FAIL_REASON_NOCANDIDATE,
	ROAMING_FAIL_REASON_NUM
} ENUM_ROAMING_FAIL_REASON_T;

/* events of roaming between driver and firmware */
typedef enum _ENUM_ROAMING_EVENT_T {
	ROAMING_EVENT_START = 0,
	ROAMING_EVENT_DISCOVERY,
	ROAMING_EVENT_ROAM,
	ROAMING_EVENT_FAIL,
	ROAMING_EVENT_ABORT,
	ROAMING_EVENT_NUM
} ENUM_ROAMING_EVENT_T;

typedef struct _CMD_ROAMING_TRANSIT_T {
	UINT_16 u2Event;
	UINT_16 u2Data;
} CMD_ROAMING_TRANSIT_T, *P_CMD_ROAMING_TRANSIT_T;

typedef struct _CMD_ROAMING_CTRL_T {
	UINT_8 fgEnable;
	UINT_8 ucRcpiAdjustStep;
	UINT_16 u2RcpiLowThr;
	UINT_8 ucRoamingRetryLimit;
	UINT_8 ucRoamingStableTimeout;
} CMD_ROAMING_CTRL_T, *P_CMD_ROAMING_CTRL_T;

 /**/ typedef enum _ENUM_ROAMING_STATE_T {
	ROAMING_STATE_IDLE = 0,
	ROAMING_STATE_DECISION,
	ROAMING_STATE_DISCOVERY,
	ROAMING_STATE_ROAM,
	ROAMING_STATE_NUM
} ENUM_ROAMING_STATE_T;

typedef struct _ROAMING_INFO_T {
	BOOLEAN fgIsEnableRoaming;

	ENUM_ROAMING_STATE_T eCurrentState;

	OS_SYSTIME rRoamingDiscoveryUpdateTime;
} ROAMING_INFO_T, *P_ROAMING_INFO_T;




#if CFG_SUPPORT_ROAMING
#define IS_ROAMING_ACTIVE(prAdapter) \
	(prAdapter->rWifiVar.rRoamingInfo.eCurrentState == ROAMING_STATE_ROAM)
#else
#define IS_ROAMING_ACTIVE(prAdapter) FALSE
#endif /* CFG_SUPPORT_ROAMING */

VOID roamingFsmInit(IN P_ADAPTER_T prAdapter);

VOID roamingFsmUninit(IN P_ADAPTER_T prAdapter);

VOID roamingFsmSendCmd(IN P_ADAPTER_T prAdapter, IN P_CMD_ROAMING_TRANSIT_T prTransit);

VOID roamingFsmScanResultsUpdate(IN P_ADAPTER_T prAdapter);

VOID roamingFsmSteps(IN P_ADAPTER_T prAdapter, IN ENUM_ROAMING_STATE_T eNextState);

VOID roamingFsmRunEventStart(IN P_ADAPTER_T prAdapter);

VOID roamingFsmRunEventDiscovery(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Param);

VOID roamingFsmRunEventRoam(IN P_ADAPTER_T prAdapter);

VOID roamingFsmRunEventFail(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Reason);

VOID roamingFsmRunEventAbort(IN P_ADAPTER_T prAdapter);

WLAN_STATUS roamingFsmProcessEvent(IN P_ADAPTER_T prAdapter, IN P_CMD_ROAMING_TRANSIT_T prTransit);


#endif /* _ROAMING_FSM_H */
