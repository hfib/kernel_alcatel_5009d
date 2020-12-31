
#ifndef _P2P_NIC_CMD_EVENT_H
#define _P2P_NIC_CMD_EVENT_H









typedef struct _EVENT_P2P_DEV_DISCOVER_RESULT_T {
/* UINT_8                          aucCommunicateAddr[MAC_ADDR_LEN];  // Deprecated. */
	UINT_8 aucDeviceAddr[MAC_ADDR_LEN];	/* Device Address. */
	UINT_8 aucInterfaceAddr[MAC_ADDR_LEN];	/* Device Address. */
	UINT_8 ucDeviceCapabilityBitmap;
	UINT_8 ucGroupCapabilityBitmap;
	UINT_16 u2ConfigMethod;	/* Configure Method. */
	P2P_DEVICE_TYPE_T rPriDevType;
	UINT_8 ucSecDevTypeNum;
	P2P_DEVICE_TYPE_T arSecDevType[2];
	UINT_16 u2NameLength;
	UINT_8 aucName[32];
	PUINT_8 pucIeBuf;
	UINT_16 u2IELength;
	UINT_8 aucBSSID[MAC_ADDR_LEN];
	/* TODO: Service Information or PasswordID valid? */
} EVENT_P2P_DEV_DISCOVER_RESULT_T, *P_EVENT_P2P_DEV_DISCOVER_RESULT_T;

#endif
