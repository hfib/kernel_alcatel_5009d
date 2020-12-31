
#ifndef _WAPI_H
#define _WAPI_H

#if CFG_SUPPORT_WAPI







#define WAPI_CIPHER_SUITE_WPI           0x01721400	/* WPI_SMS4 */
#define WAPI_AKM_SUITE_802_1X           0x01721400	/* WAI */
#define WAPI_AKM_SUITE_PSK              0x02721400	/* WAI_PSK */

#define ELEM_ID_WAPI                    68	/* WAPI IE */

#define WAPI_IE(fp)                     ((P_WAPI_INFO_ELEM_T) fp)


VOID wapiGenerateWAPIIE(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo);

BOOLEAN wapiParseWapiIE(IN P_WAPI_INFO_ELEM_T prInfoElem, OUT P_WAPI_INFO_T prWapiInfo);

BOOLEAN wapiPerformPolicySelection(IN P_ADAPTER_T prAdapter, IN P_BSS_DESC_T prBss);

/* BOOLEAN */
/* wapiUpdateTxKeyIdx ( */
/* IN  P_STA_RECORD_T     prStaRec, */
/* IN  UINT_8             ucWlanIdx */
/* ); */

#endif
#endif /* _WAPI_H */
