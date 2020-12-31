






#if (CFG_SUPPORT_STATISTICS == 1)
#define STATS_RX_PKT_INFO_DISPLAY			StatsRxPktInfoDisplay
#define STATS_TX_PKT_INFO_DISPLAY			StatsTxPktInfoDisplay
#else
#define STATS_RX_PKT_INFO_DISPLAY(__Pkt__)
#define STATS_TX_PKT_INFO_DISPLAY(__Pkt__)
#endif /* CFG_SUPPORT_STATISTICS */





VOID StatsRxPktInfoDisplay(UINT_8 *pPkt);

VOID StatsTxPktInfoDisplay(UINT_8 *pPkt);

/* End of stats.h */
