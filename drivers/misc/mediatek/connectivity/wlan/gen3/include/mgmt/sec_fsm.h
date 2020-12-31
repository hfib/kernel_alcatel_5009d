


#ifndef _SEC_FSM_H
#define _SEC_FSM_H




/* TKIP CounterMeasure interval for Rejoin to Network. */
#define COUNTER_MEASURE_TIMEOUT_INTERVAL_SEC        60

/* Timeout to wait the EAPoL Error Report frame Send out. */
#define EAPOL_REPORT_SEND_TIMEOUT_INTERVAL_SEC       1


typedef UINT_32 SEC_STATUS, *P_SEC_STATUS;

#if 0
/* WPA2 PMKID candicate structure */
typedef struct _PMKID_CANDICATE_T {
	UINT_8 aucBssid[MAC_ADDR_LEN];	/* MAC address */
	UINT_32 u4PreAuthFlags;
} PMKID_CANDICATE_T, *P_PMKID_CANDICATE_T;
#endif

typedef SEC_STATUS(*PFN_SEC_FSM_STATE_HANDLER) (VOID);



#define SEC_STATE_TRANSITION_FLAG   fgIsTransition
#define SEC_NEXT_STATE_VAR          eNextState

#define SEC_STATE_TRANSITION(prAdapter, prSta, eFromState, eToState) \
	{ secFsmTrans_ ## eFromState ## _to_ ## eToState(prAdapter, prSta); \
	  SEC_NEXT_STATE_VAR = SEC_STATE_ ## eToState; \
	  SEC_STATE_TRANSITION_FLAG = (BOOLEAN)TRUE; \
	}


/*--------------------------------------------------------------*/
/* Routines to handle the sec check                             */
/*--------------------------------------------------------------*/
/***** Routines in sec_fsm.c *****/
VOID secFsmInit(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

VOID secFsmEventInit(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

VOID secFsmEventStart(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

VOID secFsmEventAbort(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

BOOLEAN secFsmEventPTKInstalled(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

VOID secFsmEvent2ndEapolTx(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

VOID secFsmEvent4ndEapolTxDone(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

VOID
secFsmEventEapolTxDone(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prStaRec, IN ENUM_TX_RESULT_CODE_T rTxDoneStatus);

VOID secFsmEventEapolTxTimeout(IN P_ADAPTER_T prAdapter, IN ULONG ulParamPtr);

VOID
secFsmEventDeauthTxDone(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo, IN ENUM_TX_RESULT_CODE_T rTxDoneStatus);

VOID secFsmEventStartCounterMeasure(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

VOID secFsmEventEndOfCounterMeasure(IN P_ADAPTER_T prAdapter, IN ULONG ulParamPtr);

#endif /* _SEC_FSM_H */
