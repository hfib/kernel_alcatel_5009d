




#include "precomp.h"


#define P2P_INF_NAME "p2p%d"
#define AP_INF_NAME  "ap%d"

#define RUNNING_P2P_MODE  0
#define RUNNING_AP_MODE   1



static PUCHAR ifname = P2P_INF_NAME;
static UINT_16 mode = RUNNING_P2P_MODE;




VOID p2pSetSuspendMode(P_GLUE_INFO_T prGlueInfo, BOOLEAN fgEnable)
{
	struct net_device *prDev = NULL;

	if (!prGlueInfo)
		return;

	if (!prGlueInfo->prAdapter->fgIsP2PRegistered) {
		DBGLOG(P2P, INFO, "%s: P2P is not enabled, SKIP!\n", __func__);
		return;
	}

	prDev = prGlueInfo->prP2PInfo->prDevHandler;
	if (!prDev) {
		DBGLOG(P2P, WARN, "%s: P2P dev is not available, SKIP!\n", __func__);
		return;
	}

	kalSetNetAddressFromInterface(prGlueInfo, prDev, fgEnable);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN p2pLaunch(P_GLUE_INFO_T prGlueInfo)
{
	if (prGlueInfo->prAdapter->fgIsP2PRegistered == TRUE) {
		DBGLOG(P2P, INFO, "p2p is already registered\n");
		return FALSE;
	}

	if (!glRegisterP2P(prGlueInfo, ifname, (BOOLEAN) mode)) {
		DBGLOG(P2P, ERROR, "Launch failed\n");
		return FALSE;
	}

	prGlueInfo->prAdapter->fgIsP2PRegistered = TRUE;
	DBGLOG(P2P, INFO, "Launch success, fgIsP2PRegistered TRUE\n");
	return TRUE;
}

VOID p2pSetMode(IN BOOLEAN fgIsAPMode)
{
	if (fgIsAPMode) {
		mode = RUNNING_AP_MODE;
		ifname = AP_INF_NAME;
	} else {
		mode = RUNNING_P2P_MODE;
		ifname = P2P_INF_NAME;
	}

}				/* p2pSetMode */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN p2pRemove(P_GLUE_INFO_T prGlueInfo)
{
	if (prGlueInfo->prAdapter->fgIsP2PRegistered == FALSE) {
		DBGLOG(P2P, INFO, "p2p is not registered\n");
		return FALSE;
	}

	DBGLOG(P2P, INFO, "fgIsP2PRegistered FALSE\n");
	prGlueInfo->prAdapter->fgIsP2PRegistered = FALSE;
	glUnregisterP2P(prGlueInfo);
	return TRUE;
}
