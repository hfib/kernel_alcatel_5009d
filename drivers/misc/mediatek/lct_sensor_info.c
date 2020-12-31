/*
* Copyright (C) 2016 LCT sensor version.
*
* add for show sensor info
*/

#include "lct_sensor_info.h"
static char sensor_version_str[4][80];
#ifdef SUPPORT_SENSOR_VERSION
static ssize_t sensor_info_read(struct file *file , char __user *buf ,size_t size, loff_t *ppos)
{
	int cnt ;
	char *sensor_info = NULL ;
	sensor_info = kzalloc(320,GFP_KERNEL);
	if(sensor_info == NULL){
		pr_err("kzalloc error\n");
		return -1;
	}
	cnt = sprintf(sensor_info ,"%s%s%s%s",sensor_version_str[ACCELEROMETER_NO],sensor_version_str[MAGNETOMETER_NO],sensor_version_str[GYROSCOPE_NO],sensor_version_str[ALSPS_NO]);
	
	printk("%s ,sensor_info:%s",__func__ , sensor_info);
	cnt = simple_read_from_buffer(buf , size , ppos, sensor_info,cnt);
	kfree(sensor_info) ;

	return cnt;
}

static const struct file_operations sensor_proc_sensor_info_fops={
	.read  = sensor_info_read,
};


static int sensor_create_proc_entry(void)
{
	struct proc_dir_entry * proc_enty_sensor ;
	int ret = -1 ;
	proc_enty_sensor = proc_create_data("sensor_info",0444,NULL,&sensor_proc_sensor_info_fops,NULL);
	if(IS_ERR_OR_NULL(proc_enty_sensor)){
		ret =-1 ;
		pr_err("add /proc/sensor_info error \n");
	}else{
		ret = 0 ;
	}
	return ret ;
}
#ifdef SUPPORT_GSENSOR_VERSION 
static int get_gsensor_info(char * version_info_str , char * name)
{
	memset(sensor_version_str[ACCELEROMETER_NO] , 0 ,sizeof(sensor_version_str[ACCELEROMETER_NO]));
	if(strlen(version_info_str)&&strlen(name)){
		sprintf(sensor_version_str[ACCELEROMETER_NO] , "%s %s\n",name , version_info_str);
	}else{
		sprintf(sensor_version_str[ACCELEROMETER_NO] , "gsensor nop\n");
	}

	return 0 ;
}
#endif
#ifdef SUPPORT_MSENSOR_VERSION
static int get_msensor_info(char * version_info_str , char * name)
{
	memset(sensor_version_str[MAGNETOMETER_NO] , 0 ,sizeof(sensor_version_str[MAGNETOMETER_NO])) ;
	if(strlen(version_info_str)&&strlen(name)){
		sprintf(sensor_version_str[MAGNETOMETER_NO] , "%s %s\n",name , version_info_str);
	}else{
		sprintf(sensor_version_str[MAGNETOMETER_NO] , "msensor nop\n");	
	}
	return 0;
}
#endif
#ifdef SUPPORT_GYROSCOPE_VERSION 
static int get_gyroscope_info(char * version_info_str , char * name)
{
	// keep for use
	memset(sensor_version_str[GYROSCOPE_NO] , 0 ,sizeof(sensor_version_str[GYROSCOPE_NO])) ;
	if(strlen(version_info_str)&&strlen(name)){
		sprintf(sensor_version_str[GYROSCOPE_NO] , "%s %s\n",name , version_info_str);
	}else{
		sprintf(sensor_version_str[GYROSCOPE_NO] , "gyro nop\n");	
	}

	return 0;
}
#endif
#ifdef SUPPORT_ALSPS_VERSION
static int get_alsps_info(char * version_info_str , char * name)
{
	// keep for use
	memset(sensor_version_str[ALSPS_NO] , 0 ,sizeof(sensor_version_str[ALSPS_NO])) ;
	if(strlen(version_info_str)&&strlen(name)){
		sprintf(sensor_version_str[ALSPS_NO] , "%s %s\n",name , version_info_str);
	}else{
		sprintf(sensor_version_str[ALSPS_NO] , "alsps nop\n");	
	}

	return 0;
}
#endif
/*
 *sensor :0 accelerometer  1 magnetometer  2 gyroscope  3 alsps
 *
 * */
int init_sensor_version_info(u16 sensor , char * version_info_str, char * name)
{
	int i ; 
	for(i = 0 ; i < 4 ; i ++){
		memset(sensor_version_str[i] , 0 , sizeof(sensor_version_str[i]));
	}
	sprintf(sensor_version_str[ACCELEROMETER_NO],"gsensor nop\n");
	sprintf(sensor_version_str[MAGNETOMETER_NO],"msensor nop\n");
	sprintf(sensor_version_str[GYROSCOPE_NO],"gyro nop\n");
	sprintf(sensor_version_str[ALSPS_NO],"alsps nop\n");

	switch(sensor){
		case ACCELEROMETER_NO : 
			#ifdef SUPPORT_GSENSOR_VERSION 
			get_gsensor_info(version_info_str,name);
			#endif
			break ;
		case MAGNETOMETER_NO :
			#ifdef SUPPORT_MSENSOR_VERSION 
			get_msensor_info(version_info_str,name);	 
			#endif
			break ;
		case GYROSCOPE_NO :
			#ifdef SUPPORT_GYROSCOPE_VERSION
			get_gyroscope_info(version_info_str,name);
			#endif	
			break ;
		case ALSPS_NO :
			#ifdef SUPPORT_ALSPS_VERSION 
			get_alsps_info(version_info_str,name);
			#endif
			break ;
		default :	break ;
	}
	sensor_create_proc_entry();
	return 0 ;
}
#endif  // end SUPPORT_SENSOR_VERSION

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("sensor device driver");
