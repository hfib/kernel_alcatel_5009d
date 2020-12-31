
#ifndef _QOSMAP_H
#define _QOSMAP_H








#define DSCP_SUPPORT 1
VOID handleQosMapConf(IN P_ADAPTER_T prAdapter, IN P_SW_RFB_T prSwRfb);

int qosHandleQosMapConfigure(IN P_ADAPTER_T prAdapter, IN P_SW_RFB_T prSwRfb);

struct _QOS_MAP_SET *qosParseQosMapSet(IN P_ADAPTER_T prAdapter, IN PUINT_8 qosMapSet);

UINT_8 getUpFromDscp(IN P_GLUE_INFO_T prGlueInfo, IN int type, IN int dscp);

void QosMapSetRelease(IN P_STA_RECORD_T prStaRec);

#endif /* _QOSMAP_H */
