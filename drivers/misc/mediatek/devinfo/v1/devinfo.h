
#ifndef DEVFINO_H
#define DEVFINO_H

 /*****************************************************************************
 * MODULE DEFINITION
 *****************************************************************************/
#define MODULE_NAME	 "[devinfo]"
#define DEV_NAME		"devmap"
#define MAJOR_DEV_NUM	196
 /*****************************************************************************
 * IOCTL DEFINITION
 *****************************************************************************/
#define DEV_IOC_MAGIC	   'd'
#define READ_DEV_DATA	   _IOR(DEV_IOC_MAGIC,  1, unsigned int)
#define DEV_IOC_MAXNR	   (10)

struct devinfo_lk_atag_tag_header {
	u32 size;
	u32 tag;
};

struct devinfo_lk_atag_tag_devinfo_data {
	u32 devinfo_data_size;
};

#define devinfo_lk_atag_tag_size(type)  ((sizeof(struct devinfo_lk_atag_tag_header) + sizeof(struct type)) >> 2)

#ifdef CONFIG_OF
/*device information data*/
struct devinfo_tag {
	u32 size;
	u32 tag;
	u32 data[0];	/* device information */
};
#endif

#endif /* end of DEVFINO_H */

