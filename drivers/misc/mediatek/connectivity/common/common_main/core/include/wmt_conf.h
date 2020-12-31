


#ifndef _WMT_CONF_H_
#define _WMT_CONF_H_


#define CUST_CFG_WMT "WMT.cfg"
#define CUST_CFG_WMT_SOC "WMT_SOC.cfg"
#define CUST_CFG_WMT_PREFIX "/vendor/firmware/"

















INT32 wmt_conf_read_file(VOID);
P_WMT_GEN_CONF wmt_conf_get_cfg(VOID);
INT32 wmt_conf_set_cfg_file(const PINT8 name);


#endif				/* _WMT_CONF_H_ */
