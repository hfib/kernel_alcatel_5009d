

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/string.h>
/*#include <mach/memory.h>*/
#include <linux/io.h>
#include <linux/proc_fs.h>

/* only available for trustonic */
#include "mobicore_driver_api.h"

#include "secwidevine.h"

#define SECWIDEVINE_NAME     "secwidevine"
#define DEFAULT_HANDLES_NUM (64)
#define MAX_OPEN_SESSIONS   10

/* Debug message event */
#define DBG_EVT_NONE        (0)       /* No event */
#define DBG_EVT_CMD         (1 << 0)  /* SEC CMD related event */
#define DBG_EVT_FUNC        (1 << 1)  /* SEC function event */
#define DBG_EVT_INFO        (1 << 2)  /* SEC information event */
#define DBG_EVT_WRN         (1 << 30) /* Warning event */
#define DBG_EVT_ERR         (1 << 31) /* Error event */
#define DBG_EVT_ALL         (0xffffffff)

#define DBG_EVT_MASK        (DBG_EVT_ALL)

#define MSG(evt, fmt, args...) \
do { \
	if ((DBG_EVT_##evt) & DBG_EVT_MASK) { \
		pr_debug("[secwidevine][%s] "fmt, SECWIDEVINE_NAME, ##args); \
	} \
} while (0)

#define MSG_FUNC() MSG(FUNC, "%s\n", __func__)


#define TCI_PAYLOAD_LEN 1024
struct tci_t {
	u8 message[TCI_PAYLOAD_LEN];   /**< TCI message */
};

struct secwidevine_param {
	struct tci_t tci_data;
	u32 refcount;   /* INOUT */
	struct mc_session_handle session_handle; /* OUT */
};

static DEFINE_MUTEX(secwidevine_lock);

/* This version just load secure driver */
#define DR_secwidevine_UUID {0x37, 0x5d, 0xc3, 0xaa, 0x77, 0x68, 0x11, \
	0xe3, 0x8c, 0x99, 0x2c, 0x27, 0xd7, 0x44, 0xd6, 0x6e}
/* static const struct mc_uuid_t secwidevine_uuid = {TL_secwidevine_UUID}; */
static const struct mc_uuid_t secwidevinedr_uuid = {DR_secwidevine_UUID};

static struct mc_session_handle secwidevinedr_session = {0};
static u32 secwidevine_session_ref;
static u32 secwidevine_devid = MC_DEVICE_ID_DEFAULT;
/*static struct tci_t *secwidevine_tci;*/
static struct tci_t *secwidevinedr_tci;


/* Open driver in open */
static int secwidevine_session_open(void)
{
	enum mc_result mc_ret = MC_DRV_OK;

	mutex_lock(&secwidevine_lock);

	do {
		/* sessions reach max numbers ? */
		if (secwidevine_session_ref > MAX_OPEN_SESSIONS) {
			MSG(WRN, "secwidevine_session > 0x%x\n", MAX_OPEN_SESSIONS);
			break;
		}

	if (secwidevine_session_ref > 0) {
		secwidevine_session_ref++;
		break;
	}

	/* open device */
	mc_ret = mc_open_device(secwidevine_devid);
	if (MC_DRV_OK != mc_ret) {
		MSG(ERR, "mc_open_device failed: %d\n", mc_ret);
		break;
	}

	/* allocating WSM for DCI */
	/*//open trustlet
	mc_ret = mc_malloc_wsm(secwidevine_devid, 0, sizeof(struct tci_t),
	    (uint8_t **)&secwidevine_tci, 0);
	if (MC_DRV_OK != mc_ret) {
	    mc_close_device(secwidevine_devid);
	    MSG(ERR, "mc_malloc_wsm failed: %d\n", mc_ret);
	    break;
	}

	// open session
	secwidevine_session.device_id = secwidevine_devid;
	mc_ret = mc_open_session(&secwidevine_session, &secwidevine_uuid,
	    (uint8_t *)secwidevine_tci, sizeof(struct tci_t));

	if (MC_DRV_OK != mc_ret)
	{
	    mc_free_wsm(secwidevine_devid, (uint8_t *)secwidevine_tci);
	    secwidevine_tci = NULL;
	    mc_close_device(secwidevine_devid);
	    MSG(ERR, "mc_open_session failed: %d\n", mc_ret);
	    break;
	}*/
	/* open driver */
	mc_ret = mc_malloc_wsm(secwidevine_devid, 0, sizeof(struct tci_t), (uint8_t **) &secwidevinedr_tci,
		0);
	if (MC_DRV_OK != mc_ret) {
		/*mc_free_wsm(secwidevine_devid, (uint8_t *) secwidevine_tci);
		secwidevine_tci = NULL;*/
		mc_close_device(secwidevine_devid);
		MSG(ERR, "2.mc_malloc_wsm failed: %d\n", mc_ret);
		break;
	}

	/* open session */
	secwidevinedr_session.device_id = secwidevine_devid;
	mc_ret = mc_open_session(&secwidevinedr_session, &secwidevinedr_uuid,
		(uint8_t *) secwidevinedr_tci, sizeof(struct tci_t));

	if (MC_DRV_OK != mc_ret) {
		/* mc_free_wsm(secwidevine_devid, (uint8_t *) secwidevine_tci);
		 * secwidevine_tci = NULL;*/
		mc_free_wsm(secwidevine_devid, (uint8_t *) secwidevinedr_tci);
		secwidevinedr_tci = NULL;
		mc_close_device(secwidevine_devid);
		MSG(ERR, "2.mc_open_session failed: %d\n", mc_ret);
		break;
	}
	secwidevine_session_ref = 1;

	} while (0);

	MSG(INFO, "secwidevine_session_open: ret=%d, ref=%d\n", mc_ret, secwidevine_session_ref);
	MSG(INFO, "driver sessionId = %d, deviceId = %d\n",
		secwidevinedr_session.session_id, secwidevinedr_session.device_id);

	mutex_unlock(&secwidevine_lock);

	if (MC_DRV_OK != mc_ret) {
		MSG(ERR, "secwidevine_session_open fail");
		return -ENXIO;
	}

	return 0;
}

/* Close trustlet and driver */
static int secwidevine_session_close(void)
{
	enum mc_result mc_ret = MC_DRV_OK;

	mutex_lock(&secwidevine_lock);

	do {
		/* session is already closed ? */
		if (secwidevine_session_ref == 0) {
			MSG(WRN, "secwidevine_session already closed\n");
			break;
		}

		if (secwidevine_session_ref > 1) {
			secwidevine_session_ref--;
			break;
		}

    /*  close session
	mc_ret = mc_close_session(&secwidevine_session);
	if (MC_DRV_OK != mc_ret)
	{
	    MSG(ERR, "mc_close_session failed: %d\n", mc_ret);
	    break;
	}

	 free WSM for DCI
	mc_ret = mc_free_wsm(secwidevine_devid, (uint8_t*) secwidevine_tci);
	if (MC_DRV_OK != mc_ret)
	{
	    MSG(ERR, "mc_free_wsm failed: %d\n", mc_ret);
	    break;
	}
	secwidevine_tci = NULL;*/

	/* close session */
	mc_ret = mc_close_session(&secwidevinedr_session);
	if (MC_DRV_OK != mc_ret) {
		MSG(ERR, "2.mc_close_session failed: %d\n", mc_ret);
		break;
	}

	/* free WSM for DCI */
	mc_ret = mc_free_wsm(secwidevine_devid, (uint8_t *) secwidevinedr_tci);
	if (MC_DRV_OK != mc_ret) {
		MSG(ERR, "2.mc_free_wsm failed: %d\n", mc_ret);
		break;
	}
	secwidevinedr_tci = NULL;

	secwidevine_session_ref = 0;

	/* close device */
	mc_ret = mc_close_device(secwidevine_devid);
	if (MC_DRV_OK != mc_ret)
		MSG(ERR, "mc_close_device failed: %d\n", mc_ret);

	} while (0);

	MSG(INFO, "secwidevine_session_close: ret=%d, ref=%d\n", mc_ret, secwidevine_session_ref);

	mutex_unlock(&secwidevine_lock);

	if (MC_DRV_OK != mc_ret)
		return -ENXIO;

	return 0;

}

static int secwidevine_open(struct inode *inode, struct file *file)
{
	/* open session */
	if (secwidevine_session_open() < 0) {
		MSG(ERR, "secwidevine_open fail - secwidevine_session_open fail");
		return -ENXIO;
	}
	return 0;
}

static int secwidevine_release(struct inode *inode, struct file *file)
{
	int ret = 0;

	ret = secwidevine_session_close();
	return ret;
}

static const struct file_operations secwidevine_fops = {
	.owner   = THIS_MODULE,
	.open    = secwidevine_open,
	.release = secwidevine_release,
	.unlocked_ioctl = NULL,
	.write   = NULL,
	.read    = NULL,
};

static int __init secwidevine_init(void)
{
#if 0
	struct proc_dir_entry *secwidevine_proc;

	secwidevine_proc = create_proc_entry("secwidevine0",
		(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH), NULL);

	if (IS_ERR(secwidevine_proc))
		goto error;

	secwidevine_proc->proc_fops = &secwidevine_fops;
#else
	proc_create("secwidevine0", (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH), NULL, &secwidevine_fops);
#endif

	return 0;

#if 0
error:
	return -1;
#endif
}

late_initcall(secwidevine_init);
