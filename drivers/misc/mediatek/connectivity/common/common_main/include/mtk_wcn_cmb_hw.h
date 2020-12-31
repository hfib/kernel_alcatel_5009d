


#ifndef _MTK_WCN_CMB_HW_H_
#define _MTK_WCN_CMB_HW_H_





#include <osal_typedef.h>





typedef struct _PWR_SEQ_TIME_ {
	UINT32 rtcStableTime;
	UINT32 ldoStableTime;
	UINT32 rstStableTime;
	UINT32 offStableTime;
	UINT32 onStableTime;
} PWR_SEQ_TIME, *P_PWR_SEQ_TIME;














extern INT32 mtk_wcn_cmb_hw_pwr_off(VOID);
extern INT32 mtk_wcn_cmb_hw_pwr_on(VOID);
extern INT32 mtk_wcn_cmb_hw_rst(VOID);
extern INT32 mtk_wcn_cmb_hw_init(P_PWR_SEQ_TIME pPwrSeqTime);
extern INT32 mtk_wcn_cmb_hw_deinit(VOID);
extern INT32 mtk_wcn_cmb_hw_state_show(VOID);


#endif				/* _MTK_WCN_CMB_HW_H_ */
