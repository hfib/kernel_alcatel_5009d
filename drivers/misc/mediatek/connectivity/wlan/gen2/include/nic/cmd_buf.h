
#ifndef _CMD_BUF_H
#define _CMD_BUF_H





typedef enum _COMMAND_TYPE {
	COMMAND_TYPE_GENERAL_IOCTL,
	COMMAND_TYPE_NETWORK_IOCTL,
	COMMAND_TYPE_SECURITY_FRAME,
	COMMAND_TYPE_MANAGEMENT_FRAME,
	COMMAND_TYPE_NUM
} COMMAND_TYPE, *P_COMMAND_TYPE;

typedef VOID(*PFN_CMD_DONE_HANDLER) (IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo, IN PUINT_8 pucEventBuf);

typedef VOID(*PFN_CMD_TIMEOUT_HANDLER) (IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo);

struct _CMD_INFO_T {
	QUE_ENTRY_T rQueEntry;

	COMMAND_TYPE eCmdType;

	UINT_16 u2InfoBufLen;	/* This is actual CMD buffer length */
	PUINT_8 pucInfoBuffer;	/* May pointer to structure in prAdapter */
	P_NATIVE_PACKET prPacket;	/* only valid when it's a security frame */

	ENUM_NETWORK_TYPE_INDEX_T eNetworkType;
	UINT_8 ucStaRecIndex;	/* only valid when it's a security frame */

	PFN_CMD_DONE_HANDLER pfCmdDoneHandler;
	PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler;

	BOOLEAN fgIsOid;	/* Used to check if we need indicate */

	UINT_8 ucCID;
	BOOLEAN fgSetQuery;
	BOOLEAN fgNeedResp;
	BOOLEAN fgDriverDomainMCR;	/* Access Driver Domain MCR, for CMD_ID_ACCESS_REG only */
	UINT_8 ucCmdSeqNum;
	UINT_32 u4SetInfoLen;	/* Indicate how many byte we read for Set OID */

	/* information indicating by OID/ioctl */
	PVOID pvInformationBuffer;
	UINT_32 u4InformationBufferLength;

	/* private data */
	UINT_32 u4PrivateData;
	UINT_32 u4InqueTime;
};




VOID cmdBufInitialize(IN P_ADAPTER_T prAdapter);

P_CMD_INFO_T cmdBufAllocateCmdInfo(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Length);

VOID cmdBufFreeCmdInfo(IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo);

/*----------------------------------------------------------------------------*/
/* Routines for CMDs                                                          */
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanSendSetQueryCmd(IN P_ADAPTER_T prAdapter,
		    UINT_8 ucCID,
		    BOOLEAN fgSetQuery,
		    BOOLEAN fgNeedResp,
		    BOOLEAN fgIsOid,
		    PFN_CMD_DONE_HANDLER pfCmdDoneHandler,
		    PFN_CMD_TIMEOUT_HANDLER pfCmdTimeoutHandler,
		    UINT_32 u4SetQueryInfoLen,
		    PUINT_8 pucInfoBuffer, OUT PVOID pvSetQueryBuffer, IN UINT_32 u4SetQueryBufferLen);
VOID cmdBufDumpCmdQueue(P_QUE_T prQueue, CHAR *queName);
#endif /* _CMD_BUF_H */
