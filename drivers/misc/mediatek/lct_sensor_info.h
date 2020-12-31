#ifndef __LCT_SENSOR_VERSION_INFO_H__
#define __LCT_SENSOR_VERSION_INFO_H__


#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>


#define SUPPORT_SENSOR_VERSION  // show sensor infor 
#define SUPPORT_GSENSOR_VERSION  // support gsensor version
//#define SUPPORT_MSENSOR_VERSION  // support msensor version
//#define SUPPORT_GYROSCOPE_VERSION // support gyroscope version
//#define SUPPORT_ALSPS_VERSION // support alsps version

#define ACCELEROMETER_NO 0
#define MAGNETOMETER_NO  1
#define GYROSCOPE_NO     2
#define ALSPS_NO         3

int init_sensor_version_info(u16 sensor , char * version_info_str, char * name);




#endif // end __LCT_SENSOR_VERSION_INFO_H__ 
