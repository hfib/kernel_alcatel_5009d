


#ifndef _HIF_H
#define _HIF_H

#if MTK_WCN_HIF_SDIO
#include "hif_sdio.h"
#endif



#define SDIO_X86_WORKAROUND_WRITE_MCR   0x00C4


typedef struct _GL_HIF_INFO_T {
#if MTK_WCN_HIF_SDIO
	MTK_WCN_HIF_SDIO_CLTCTX cltCtx;

	const MTK_WCN_HIF_SDIO_FUNCINFO *prFuncInfo;
#else
	struct sdio_func *func;
#endif
	BOOLEAN fgIntReadClear;
	BOOLEAN fgMbxReadClear;
} GL_HIF_INFO_T, *P_GL_HIF_INFO_T;

#if CFG_DBG_GPIO_PINS

extern void mtk_wcn_stp_debug_gpio_assert(UINT_32 dwIndex, UINT_32 dwMethod);

/* Platform DEPENDENT configurations: MT6516 GPIO pin name */
#define GPIO_6516(x) (GPIO##x)
#define GPIO_PLATFORM(x) GPIO_6516(x)

/* Platform independent configurations */
#define DBG_TIE_DIR 0x40000000UL
#define DBG_LOW 0x0UL
#define DBG_HIGH 0x1UL
#define DBG_TIE_LOW (DBG_TIE_DIR | DBG_LOW)
#define DBG_TIE_HIGH (DBG_TIE_DIR | DBG_HIGH)

#define DBG_TOGGLE(x) (0x80000000UL | x)
#define DBG_TOGGLE_NUM(x) (x & 0xFFF)
#define GPIO_INVALID 0xFFFF

typedef enum {
	IDX_ERR = 0,
	IDX_TX_THREAD,
	IDX_TX_REQ,
	IDX_TX_PORT_WRITE,
	IDX_STP_MTX_BT,
	IDX_STP_MTX_FM,		/* 5 */
	IDX_STP_MTX_GPS,
	IDX_STP_MTX_WIFI,
	IDX_STP_MTX_WMT,
	IDX_LOOP_CNT,
	IDX_NO_BUF,		/*10 */
	IDX_BT_TX,
	IDX_BT_RX,
	IDX_GPS_RX,
	IDX_GPS_2,
	IDX_GPS_3,
	IDX_GPS_TX,
	IDX_STP_MAX
} DBG_PIN;

#define DBG_MTX_OFFSET (IDX_STP_MTX_BT)	/* index of mutex0 */
#endif




WLAN_STATUS glRegisterBus(probe_card pfProbe, remove_card pfRemove);

VOID glUnregisterBus(remove_card pfRemove);

VOID glSetHifInfo(P_GLUE_INFO_T prGlueInfo, ULONG ulCookie);

VOID glClearHifInfo(P_GLUE_INFO_T prGlueInfo);

BOOL glBusInit(PVOID pvData);

VOID glBusRelease(PVOID pData);

INT_32 glBusSetIrq(PVOID pvData, PVOID pfnIsr, PVOID pvCookie);

VOID glBusFreeIrq(PVOID pvData, PVOID pvCookie);

VOID glSetPowerState(IN P_GLUE_INFO_T prGlueInfo, IN UINT_32 ePowerMode);

#endif /* _HIF_H */
