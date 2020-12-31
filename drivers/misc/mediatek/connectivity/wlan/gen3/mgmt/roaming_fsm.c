



#include "precomp.h"

#if CFG_SUPPORT_ROAMING




#if DBG
/*lint -save -e64 Type mismatch */
static PUINT_8 apucDebugRoamingState[ROAMING_STATE_NUM] = {
	(PUINT_8) DISP_STRING("ROAMING_STATE_IDLE"),
	(PUINT_8) DISP_STRING("ROAMING_STATE_DECISION"),
	(PUINT_8) DISP_STRING("ROAMING_STATE_DISCOVERY"),
	(PUINT_8) DISP_STRING("ROAMING_STATE_ROAM")
};

/*lint -restore */
#endif /* DBG */




/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID roamingFsmInit(IN P_ADAPTER_T prAdapter)
{
	P_ROAMING_INFO_T prRoamingFsmInfo;
	P_CONNECTION_SETTINGS_T prConnSettings;

	DBGLOG(ROAMING, LOUD, "->roamingFsmInit(): Current Time = %ld\n", kalGetTimeTick());

	prRoamingFsmInfo = (P_ROAMING_INFO_T) &(prAdapter->rWifiVar.rRoamingInfo);
	prConnSettings = &(prAdapter->rWifiVar.rConnSettings);

	/* 4 <1> Initiate FSM */
	prRoamingFsmInfo->fgIsEnableRoaming = prConnSettings->fgIsEnableRoaming;
	prRoamingFsmInfo->eCurrentState = ROAMING_STATE_IDLE;
	prRoamingFsmInfo->rRoamingDiscoveryUpdateTime = 0;
}				/* end of roamingFsmInit() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID roamingFsmUninit(IN P_ADAPTER_T prAdapter)
{
	P_ROAMING_INFO_T prRoamingFsmInfo;

	DBGLOG(ROAMING, LOUD, "->roamingFsmUninit(): Current Time = %ld\n", kalGetTimeTick());

	prRoamingFsmInfo = (P_ROAMING_INFO_T) &(prAdapter->rWifiVar.rRoamingInfo);

	prRoamingFsmInfo->eCurrentState = ROAMING_STATE_IDLE;

}				/* end of roamingFsmUninit() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID roamingFsmSendCmd(IN P_ADAPTER_T prAdapter, IN P_CMD_ROAMING_TRANSIT_T prTransit)
{
	P_ROAMING_INFO_T prRoamingFsmInfo;
	WLAN_STATUS rStatus;

	DBGLOG(ROAMING, LOUD, "->roamingFsmSendCmd(): Current Time = %ld\n", kalGetTimeTick());

	prRoamingFsmInfo = (P_ROAMING_INFO_T) &(prAdapter->rWifiVar.rRoamingInfo);

	rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
				      CMD_ID_ROAMING_TRANSIT,	/* ucCID */
				      TRUE,	/* fgSetQuery */
				      FALSE,	/* fgNeedResp */
				      FALSE,	/* fgIsOid */
				      NULL,	/* pfCmdDoneHandler */
				      NULL,	/* pfCmdTimeoutHandler */
				      sizeof(CMD_ROAMING_TRANSIT_T),	/* u4SetQueryInfoLen */
				      (PUINT_8) prTransit,	/* pucInfoBuffer */
				      NULL,	/* pvSetQueryBuffer */
				      0	/* u4SetQueryBufferLen */
	    );

	/* ASSERT(rStatus == WLAN_STATUS_PENDING); */

}				/* end of roamingFsmSendCmd() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID roamingFsmScanResultsUpdate(IN P_ADAPTER_T prAdapter)
{
	P_ROAMING_INFO_T prRoamingFsmInfo;

	prRoamingFsmInfo = (P_ROAMING_INFO_T) &(prAdapter->rWifiVar.rRoamingInfo);

	/* Check Roaming Conditions */
	do {
		if (!(prRoamingFsmInfo->fgIsEnableRoaming))
			return;
	} while (0);

	DBGLOG(ROAMING, LOUD, "->roamingFsmScanResultsUpdate(): Current Time = %ld\n", kalGetTimeTick());

	GET_CURRENT_SYSTIME(&prRoamingFsmInfo->rRoamingDiscoveryUpdateTime);

}				/* end of roamingFsmScanResultsUpdate() */

static BOOLEAN roamingFsmIsNeedScan(IN P_ADAPTER_T prAdapter)
{
	P_SCAN_INFO_T prScanInfo;
	P_LINK_T prRoamBSSDescList;
	P_ROAM_BSS_DESC_T prRoamBssDesc;
	P_BSS_INFO_T prAisBssInfo;
	P_BSS_DESC_T prBssDesc;
	CMD_SW_DBG_CTRL_T rCmdSwCtrl;
	BOOLEAN fgIsNeedScan = FALSE;
	BOOLEAN fgIsRoamingSSID = FALSE;

	prAisBssInfo = prAdapter->prAisBssInfo;
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prRoamBSSDescList = &prScanInfo->rRoamBSSDescList;
	/* Count same BSS Desc from current SCAN result list. */
	LINK_FOR_EACH_ENTRY(prRoamBssDesc, prRoamBSSDescList, rLinkEntry, ROAM_BSS_DESC_T) {
		if (EQUAL_SSID(prRoamBssDesc->aucSSID,
				       prRoamBssDesc->ucSSIDLen,
				       prAisBssInfo->aucSSID, prAisBssInfo->ucSSIDLen)) {
			fgIsRoamingSSID = TRUE;
			fgIsNeedScan = TRUE;
			DBGLOG(INIT, INFO, "roamingFsmSteps: IsRoamingSSID:%d\n", fgIsRoamingSSID);
			break;
		}
	}

	if (!fgIsRoamingSSID) {
		prBssDesc = scanSearchBssDescByBssid(prAdapter, prAisBssInfo->aucBSSID);
		if (prBssDesc) {

			rCmdSwCtrl.u4Id = 0xa0280000;
			rCmdSwCtrl.u4Data = 0x1;
			wlanSendSetQueryCmd(prAdapter,
					    CMD_ID_SW_DBG_CTRL,
					    TRUE,
					    FALSE,
					    FALSE, NULL, NULL, sizeof(CMD_SW_DBG_CTRL_T),
					    (PUINT_8)&rCmdSwCtrl, NULL, 0);

			DBGLOG(INIT, INFO, "roamingFsmSteps: RCPI:%d RoamSkipTimes:%d\n",
								prBssDesc->ucRCPI, prAisBssInfo->ucRoamSkipTimes);
			if (prBssDesc->ucRCPI > 90) {
				prAisBssInfo->ucRoamSkipTimes = 3;
				prAisBssInfo->fgGoodRcpiArea = TRUE;
				prAisBssInfo->fgPoorRcpiArea = FALSE;
			} else {
				if (prAisBssInfo->fgGoodRcpiArea) {
					prAisBssInfo->ucRoamSkipTimes--;
				} else if (prBssDesc->ucRCPI > 67) {
					if (!prAisBssInfo->fgPoorRcpiArea) {
						prAisBssInfo->ucRoamSkipTimes = 2;
						prAisBssInfo->fgPoorRcpiArea = TRUE;
						prAisBssInfo->fgGoodRcpiArea = FALSE;
					} else {
						prAisBssInfo->ucRoamSkipTimes--;
					}
				} else {
					prAisBssInfo->fgPoorRcpiArea = FALSE;
					prAisBssInfo->fgGoodRcpiArea = FALSE;
					prAisBssInfo->ucRoamSkipTimes--;
				}
			}

			if (prAisBssInfo->ucRoamSkipTimes == 0) {
				prAisBssInfo->ucRoamSkipTimes = 3;
				prAisBssInfo->fgPoorRcpiArea = FALSE;
				prAisBssInfo->fgGoodRcpiArea = FALSE;
				DBGLOG(INIT, INFO, "roamingFsmSteps: Need Scan\n");
				fgIsNeedScan = TRUE;
			}
		}
	}

	return fgIsNeedScan;
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID roamingFsmSteps(IN P_ADAPTER_T prAdapter, IN ENUM_ROAMING_STATE_T eNextState)
{
	P_ROAMING_INFO_T prRoamingFsmInfo;
	ENUM_ROAMING_STATE_T ePreviousState;
	BOOLEAN fgIsTransition = (BOOLEAN) FALSE;

	prRoamingFsmInfo = (P_ROAMING_INFO_T) &(prAdapter->rWifiVar.rRoamingInfo);

	do {

		/* Do entering Next State */
#if DBG
		DBGLOG(ROAMING, STATE, "TRANSITION: [%s] -> [%s]\n",
					apucDebugRoamingState[prRoamingFsmInfo->eCurrentState],
					apucDebugRoamingState[eNextState]);
#else
		DBGLOG(ROAMING, STATE, "[%d] TRANSITION: [%d] -> [%d]\n",
					DBG_ROAMING_IDX, prRoamingFsmInfo->eCurrentState, eNextState);
#endif
		/* NOTE(Kevin): This is the only place to change the eCurrentState(except initial) */
		ePreviousState = prRoamingFsmInfo->eCurrentState;
		prRoamingFsmInfo->eCurrentState = eNextState;

		fgIsTransition = (BOOLEAN) FALSE;

		/* Do tasks of the State that we just entered */
		switch (prRoamingFsmInfo->eCurrentState) {
			/* NOTE(Kevin): we don't have to rearrange the sequence of following
			 * switch case. Instead I would like to use a common lookup table of array
			 * of function pointer to speed up state search.
			 */
		case ROAMING_STATE_IDLE:
		case ROAMING_STATE_DECISION:
			break;

		case ROAMING_STATE_DISCOVERY:
			{
				OS_SYSTIME rCurrentTime;
				BOOLEAN fgIsNeedScan = FALSE;

				fgIsNeedScan = roamingFsmIsNeedScan(prAdapter);

				GET_CURRENT_SYSTIME(&rCurrentTime);
				if (CHECK_FOR_TIMEOUT
				    (rCurrentTime, prRoamingFsmInfo->rRoamingDiscoveryUpdateTime,
				     SEC_TO_SYSTIME(ROAMING_DISCOVERY_TIMEOUT_SEC)) && fgIsNeedScan) {
					DBGLOG(ROAMING, LOUD, "roamingFsmSteps: DiscoveryUpdateTime Timeout");
					aisFsmRunEventRoamingDiscovery(prAdapter, TRUE);
				} else {
					DBGLOG(ROAMING, LOUD, "roamingFsmSteps: DiscoveryUpdateTime Updated");
					aisFsmRunEventRoamingDiscovery(prAdapter, FALSE);
				}
			}
			break;

		case ROAMING_STATE_ROAM:
			break;

		default:
			ASSERT(0);	/* Make sure we have handle all STATEs */
		}
	} while (fgIsTransition);

}				/* end of roamingFsmSteps() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID roamingFsmRunEventStart(IN P_ADAPTER_T prAdapter)
{
	P_ROAMING_INFO_T prRoamingFsmInfo;
	ENUM_ROAMING_STATE_T eNextState;
	P_BSS_INFO_T prAisBssInfo;
	CMD_ROAMING_TRANSIT_T rTransit;

	prRoamingFsmInfo = (P_ROAMING_INFO_T) &(prAdapter->rWifiVar.rRoamingInfo);

	/* Check Roaming Conditions */
	do {
		if (!(prRoamingFsmInfo->fgIsEnableRoaming))
			return;
	} while (0);

	prAisBssInfo = prAdapter->prAisBssInfo;
	if (prAisBssInfo->eCurrentOPMode != OP_MODE_INFRASTRUCTURE)
		return;

	DBGLOG(ROAMING, EVENT, "EVENT-ROAMING START: Current Time = %ld\n", kalGetTimeTick());

	/* IDLE, ROAM -> DECISION */
	/* Errors as DECISION, DISCOVERY -> DECISION */
	if (!(prRoamingFsmInfo->eCurrentState == ROAMING_STATE_IDLE
	      || prRoamingFsmInfo->eCurrentState == ROAMING_STATE_ROAM))
		return;

	eNextState = ROAMING_STATE_DECISION;
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		rTransit.u2Event = ROAMING_EVENT_START;
		rTransit.u2Data = prAisBssInfo->ucBssIndex;
		roamingFsmSendCmd(prAdapter, (P_CMD_ROAMING_TRANSIT_T) & rTransit);

		/* Step to next state */
		roamingFsmSteps(prAdapter, eNextState);
	}
}				/* end of roamingFsmRunEventStart() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID roamingFsmRunEventDiscovery(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Param)
{
	P_ROAMING_INFO_T prRoamingFsmInfo;
	ENUM_ROAMING_STATE_T eNextState;

	prRoamingFsmInfo = (P_ROAMING_INFO_T) &(prAdapter->rWifiVar.rRoamingInfo);

	/* Check Roaming Conditions */
	do {
		if (!(prRoamingFsmInfo->fgIsEnableRoaming))
			return;
	} while (0);

	DBGLOG(ROAMING, EVENT, "EVENT-ROAMING DISCOVERY: Current Time = %ld\n", kalGetTimeTick());

	/* DECISION -> DISCOVERY */
	/* Errors as IDLE, DISCOVERY, ROAM -> DISCOVERY */
	if (prRoamingFsmInfo->eCurrentState != ROAMING_STATE_DECISION)
		return;

	eNextState = ROAMING_STATE_DISCOVERY;
	/* DECISION -> DISCOVERY */
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		P_BSS_INFO_T prAisBssInfo;
		P_BSS_DESC_T prBssDesc;

		/* sync. rcpi with firmware */
		prAisBssInfo = prAdapter->prAisBssInfo;
		prBssDesc = scanSearchBssDescByBssid(prAdapter, prAisBssInfo->aucBSSID);
		if (prBssDesc)
			prBssDesc->ucRCPI = (UINT_8) (u4Param & 0xff);

		roamingFsmSteps(prAdapter, eNextState);
	}

}				/* end of roamingFsmRunEventDiscovery() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID roamingFsmRunEventRoam(IN P_ADAPTER_T prAdapter)
{
	P_ROAMING_INFO_T prRoamingFsmInfo;
	ENUM_ROAMING_STATE_T eNextState;
	CMD_ROAMING_TRANSIT_T rTransit;

	prRoamingFsmInfo = (P_ROAMING_INFO_T) &(prAdapter->rWifiVar.rRoamingInfo);

	/* Check Roaming Conditions */
	do {
		if (!(prRoamingFsmInfo->fgIsEnableRoaming))
			return;
	} while (0);

	DBGLOG(ROAMING, EVENT, "EVENT-ROAMING ROAM: Current Time = %ld\n", kalGetTimeTick());

	/* IDLE, ROAM -> DECISION */
	/* Errors as IDLE, DECISION, ROAM -> ROAM */
	if (prRoamingFsmInfo->eCurrentState != ROAMING_STATE_DISCOVERY)
		return;

	eNextState = ROAMING_STATE_ROAM;
	/* DISCOVERY -> ROAM */
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		rTransit.u2Event = ROAMING_EVENT_ROAM;
		roamingFsmSendCmd(prAdapter, (P_CMD_ROAMING_TRANSIT_T) & rTransit);

		/* Step to next state */
		roamingFsmSteps(prAdapter, eNextState);
	}

}				/* end of roamingFsmRunEventRoam() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID roamingFsmRunEventFail(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Param)
{
	P_ROAMING_INFO_T prRoamingFsmInfo;
	ENUM_ROAMING_STATE_T eNextState;
	CMD_ROAMING_TRANSIT_T rTransit;

	prRoamingFsmInfo = (P_ROAMING_INFO_T) &(prAdapter->rWifiVar.rRoamingInfo);

	/* Check Roaming Conditions */
	do {
		if (!(prRoamingFsmInfo->fgIsEnableRoaming))
			return;
	} while (0);

	DBGLOG(ROAMING, EVENT, "EVENT-ROAMING FAIL: reason %x Current Time = %ld\n", u4Param, kalGetTimeTick());

	/* IDLE, ROAM -> DECISION */
	/* Errors as IDLE, DECISION, DISCOVERY -> DECISION */
	if (prRoamingFsmInfo->eCurrentState != ROAMING_STATE_ROAM)
		return;

	eNextState = ROAMING_STATE_DECISION;
	/* ROAM -> DECISION */
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		rTransit.u2Event = ROAMING_EVENT_FAIL;
		rTransit.u2Data = (UINT_16) (u4Param & 0xffff);
		roamingFsmSendCmd(prAdapter, (P_CMD_ROAMING_TRANSIT_T) & rTransit);

		/* Step to next state */
		roamingFsmSteps(prAdapter, eNextState);
	}

}				/* end of roamingFsmRunEventFail() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID roamingFsmRunEventAbort(IN P_ADAPTER_T prAdapter)
{
	P_ROAMING_INFO_T prRoamingFsmInfo;
	ENUM_ROAMING_STATE_T eNextState;
	CMD_ROAMING_TRANSIT_T rTransit;

	prRoamingFsmInfo = (P_ROAMING_INFO_T) &(prAdapter->rWifiVar.rRoamingInfo);

	do {
		if (!(prRoamingFsmInfo->fgIsEnableRoaming))
			return;
	} while (0);

	DBGLOG(ROAMING, TRACE, "EVENT-ROAMING ABORT: Current Time = %ld\n", kalGetTimeTick());

	eNextState = ROAMING_STATE_IDLE;
	/* IDLE, DECISION, DISCOVERY, ROAM -> IDLE */
	if (eNextState != prRoamingFsmInfo->eCurrentState) {
		rTransit.u2Event = ROAMING_EVENT_ABORT;
		roamingFsmSendCmd(prAdapter, (P_CMD_ROAMING_TRANSIT_T) & rTransit);

		/* Step to next state */
		roamingFsmSteps(prAdapter, eNextState);
	}

}				/* end of roamingFsmRunEventAbort() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS roamingFsmProcessEvent(IN P_ADAPTER_T prAdapter, IN P_CMD_ROAMING_TRANSIT_T prTransit)
{
	DBGLOG(ROAMING, LOUD, "ROAMING Process Events: Current Time = %ld\n", kalGetTimeTick());

	if (ROAMING_EVENT_DISCOVERY == prTransit->u2Event)
		roamingFsmRunEventDiscovery(prAdapter, prTransit->u2Data);

	return WLAN_STATUS_SUCCESS;
}

#endif
