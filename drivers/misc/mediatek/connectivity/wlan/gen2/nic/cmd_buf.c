

#include "precomp.h"




static BOOLEAN fgCmdDumpIsDone = FALSE;


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID cmdBufInitialize(IN P_ADAPTER_T prAdapter)
{
	P_CMD_INFO_T prCmdInfo;
	UINT_32 i;

	ASSERT(prAdapter);

	QUEUE_INITIALIZE(&prAdapter->rFreeCmdList);

	for (i = 0; i < CFG_TX_MAX_CMD_PKT_NUM; i++) {
		prCmdInfo = &prAdapter->arHifCmdDesc[i];
		QUEUE_INSERT_TAIL(&prAdapter->rFreeCmdList, &prCmdInfo->rQueEntry);
	}
	fgCmdDumpIsDone = FALSE;
}				/* end of cmdBufInitialize() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID cmdBufDumpCmdQueue(P_QUE_T prQueue, CHAR *queName)
{
	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T)QUEUE_GET_HEAD(prQueue);

	DBGLOG(NIC, INFO, "Dump CMD info for %s, Elem number:%u\n", queName, prQueue->u4NumElem);
	while (prCmdInfo) {
		P_CMD_INFO_T prCmdInfo1, prCmdInfo2, prCmdInfo3;

		prCmdInfo1 = (P_CMD_INFO_T)QUEUE_GET_NEXT_ENTRY((P_QUE_ENTRY_T)prCmdInfo);
		if (!prCmdInfo1) {
			DBGLOG(NIC, INFO, "CID:%d SEQ:%d\n", prCmdInfo->ucCID, prCmdInfo->ucCmdSeqNum);
			break;
		}
		prCmdInfo2 = (P_CMD_INFO_T)QUEUE_GET_NEXT_ENTRY((P_QUE_ENTRY_T)prCmdInfo1);
		if (!prCmdInfo2) {
			DBGLOG(NIC, INFO, "CID:%d, SEQ:%d; CID:%d, SEQ:%d\n", prCmdInfo->ucCID,
				prCmdInfo->ucCmdSeqNum, prCmdInfo1->ucCID, prCmdInfo1->ucCmdSeqNum);
			break;
		}
		prCmdInfo3 = (P_CMD_INFO_T)QUEUE_GET_NEXT_ENTRY((P_QUE_ENTRY_T)prCmdInfo2);
		if (!prCmdInfo3) {
			DBGLOG(NIC, INFO, "CID:%d, SEQ:%d; CID:%d, SEQ:%d; CID:%d, SEQ:%d\n", prCmdInfo->ucCID,
				prCmdInfo->ucCmdSeqNum, prCmdInfo1->ucCID, prCmdInfo1->ucCmdSeqNum,
				prCmdInfo2->ucCID, prCmdInfo2->ucCmdSeqNum);
			break;
		}
		DBGLOG(NIC, INFO, "CID:%d, SEQ:%d; CID:%d, SEQ:%d; CID:%d, SEQ:%d; CID:%d, SEQ:%d\n",
				prCmdInfo->ucCID, prCmdInfo->ucCmdSeqNum, prCmdInfo1->ucCID,
				prCmdInfo1->ucCmdSeqNum, prCmdInfo2->ucCID, prCmdInfo2->ucCmdSeqNum,
				prCmdInfo3->ucCID, prCmdInfo3->ucCmdSeqNum);
		prCmdInfo = (P_CMD_INFO_T)QUEUE_GET_NEXT_ENTRY((P_QUE_ENTRY_T)prCmdInfo3);
	}
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
P_CMD_INFO_T cmdBufAllocateCmdInfo(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Length)
{
	P_CMD_INFO_T prCmdInfo;

	KAL_SPIN_LOCK_DECLARATION();

	DEBUGFUNC("cmdBufAllocateCmdInfo");

	ASSERT(prAdapter);

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);
	QUEUE_REMOVE_HEAD(&prAdapter->rFreeCmdList, prCmdInfo, P_CMD_INFO_T);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);

	if (prCmdInfo) {
		/* Setup initial value in CMD_INFO_T */
		/* Start address of allocated memory */
		prCmdInfo->pucInfoBuffer = cnmMemAlloc(prAdapter, RAM_TYPE_BUF, u4Length);

		if (prCmdInfo->pucInfoBuffer == NULL) {
			KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);
			QUEUE_INSERT_TAIL(&prAdapter->rFreeCmdList, &prCmdInfo->rQueEntry);
			KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);

			prCmdInfo = NULL;

			DBGLOG(NIC, ERROR, "Allocate prCmdInfo->pucInfoBuffer fail!\n");
		} else {
			prCmdInfo->u2InfoBufLen = 0;
			prCmdInfo->fgIsOid = FALSE;
			prCmdInfo->fgDriverDomainMCR = FALSE;
			prCmdInfo->u4InqueTime = 0;
		}
		fgCmdDumpIsDone = FALSE;
	} else if (!fgCmdDumpIsDone) {
		P_GLUE_INFO_T prGlueInfo = prAdapter->prGlueInfo;
		P_QUE_T prCmdQue = &prGlueInfo->rCmdQueue;
		P_QUE_T prPendingCmdQue = &prAdapter->rPendingCmdQueue;
		P_TX_TCQ_STATUS_T prTc = &prAdapter->rTxCtrl.rTc;

		fgCmdDumpIsDone = TRUE;
		cmdBufDumpCmdQueue(prCmdQue, "waiting Tx CMD queue");
		cmdBufDumpCmdQueue(prPendingCmdQue, "waiting response CMD queue");
		DBGLOG(NIC, INFO, "Tc4 number:%d\n", prTc->aucFreeBufferCount[TC4_INDEX]);
	}

	return prCmdInfo;

}				/* end of cmdBufAllocateCmdInfo() */

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID cmdBufFreeCmdInfo(IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo)
{
	KAL_SPIN_LOCK_DECLARATION();

	DEBUGFUNC("cmdBufFreeCmdInfo");

	ASSERT(prAdapter);
	ASSERT(prCmdInfo);

	if (prCmdInfo) {
		if (prCmdInfo->pucInfoBuffer) {
			cnmMemFree(prAdapter, prCmdInfo->pucInfoBuffer);
			prCmdInfo->pucInfoBuffer = NULL;
		}

		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);
		QUEUE_INSERT_TAIL(&prAdapter->rFreeCmdList, &prCmdInfo->rQueEntry);
		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_RESOURCE);
	}

	return;

}				/* end of cmdBufFreeCmdPacket() */
