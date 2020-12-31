
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/gpio.h>
#include "kd_flashlight.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include <mt-plat/mt_gpio.h>
#include "kd_camera_typedef.h"
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/wakelock.h>
#include <mt-plat/mt_pwm.h>

/* availible parameter */
/* ANDROID_LOG_ASSERT */
/* ANDROID_LOG_ERROR */
/* ANDROID_LOG_WARNING */
/* ANDROID_LOG_INFO */
/* ANDROID_LOG_DEBUG */
/* ANDROID_LOG_VERBOSE */

#define TAG_NAME "[leds_strobe.c]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    pr_debug(TAG_NAME "%s: " fmt, __func__ , ##arg)

/*#define DEBUG_LEDS_STROBE*/
#ifdef DEBUG_LEDS_STROBE
#define PK_DBG PK_DBG_FUNC
#else
#define PK_DBG(a, ...)
#endif


static DEFINE_SPINLOCK(g_strobeSMPLock);	/* cotta-- SMP proection */

static u32 strobe_Res;
static u32 strobe_Timeus;
static BOOL g_strobe_On;

static int g_timeOutTimeMs;

static DEFINE_MUTEX(g_strobeSem);

#define STROBE_DEVICE_ID 0xC6

static struct work_struct workTimeOut;

static void work_timeOutFunc(struct work_struct *data);

/* AW3640 FLASHLIGHT DRIVER START */
#ifdef CONFIG_FLASHLIGHT_AW3640

#define AW3640_DEVNAME "aw3640_flashlight"
static int aw3640_enable_pin;
static struct pwm_spec_config aw3640_pwm_setting;
static struct wake_lock aw3640_wake_lock;

static void aw3640_enable(void)
{
	PK_DBG("%s:%d!\n", __func__, __LINE__);

	wake_lock(&aw3640_wake_lock);

	pwm_set_spec_config(&aw3640_pwm_setting);
}

static void aw3640_disable(void)
{
	PK_DBG("%s:%d!\n", __func__, __LINE__);

	wake_unlock(&aw3640_wake_lock);

	mt_pwm_disable(PWM1,0);
}

static void aw3640_dim_duty(kal_uint32 duty)
{
	PK_DBG("%s:%d duty=%d!\n", __func__, __LINE__, duty);

	if(duty == 0)
		aw3640_pwm_setting.PWM_MODE_OLD_REGS.THRESH = 8; //modify by liaoesen for reduce flashlight current
	else if (duty == 1)
		aw3640_pwm_setting.PWM_MODE_OLD_REGS.THRESH = 8;
	else if (duty > 16)
		aw3640_pwm_setting.PWM_MODE_OLD_REGS.THRESH = 16;
	else
		aw3640_pwm_setting.PWM_MODE_OLD_REGS.THRESH = duty;
}

static const struct of_device_id aw3640_of_match[] = {
	{.compatible = "mediatek,flashlight_aw3640"},
};

static int aw3640_probe(struct platform_device *dev)
{
	int ret = 0;
	struct device_node *aw3640_flashlight_device_node = NULL;

	PK_DBG("%s:%d!\n", __func__, __LINE__);

	aw3640_flashlight_device_node = of_find_matching_node(aw3640_flashlight_device_node, aw3640_of_match);

	of_property_read_u32(aw3640_flashlight_device_node, "fl-en-gpio", &aw3640_enable_pin);

	aw3640_pwm_setting.pwm_no = PWM1;
	aw3640_pwm_setting.pmic_pad = 0;
#ifdef PWM_FIFO_1
	aw3640_pwm_setting.mode = PWM_MODE_FIFO;
	aw3640_pwm_setting.clk_div=CLK_DIV1;
	aw3640_pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;

	aw3640_pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
	aw3640_pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
	aw3640_pwm_setting.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE = 31;
	aw3640_pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = 4;
	aw3640_pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = 4;
	aw3640_pwm_setting.PWM_MODE_FIFO_REGS.GDURATION =
		(aw3640_pwm_setting.PWM_MODE_FIFO_REGS.HDURATION + 1) * 32 - 1;
	aw3640_pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 1;
	aw3640_pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 =50;
	aw3640_pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;
#else
	aw3640_pwm_setting.mode = PWM_MODE_OLD;
	aw3640_pwm_setting.clk_div=CLK_DIV1;
	aw3640_pwm_setting.clk_src = PWM_CLK_OLD_MODE_BLOCK;
	aw3640_pwm_setting.PWM_MODE_OLD_REGS.IDLE_VALUE = 0;
	aw3640_pwm_setting.PWM_MODE_OLD_REGS.GUARD_VALUE = 0;
	aw3640_pwm_setting.PWM_MODE_OLD_REGS.GDURATION = 0;
	aw3640_pwm_setting.PWM_MODE_OLD_REGS.WAVE_NUM = 0;
	aw3640_pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 16;
	aw3640_pwm_setting.PWM_MODE_OLD_REGS.THRESH = 8;
#endif

	wake_lock_init(&aw3640_wake_lock, WAKE_LOCK_SUSPEND, "aw3640 wakelock");

	return ret;
}

static int aw3640_remove(struct platform_device *dev)
{
	PK_DBG("%s:%d!\n", __func__, __LINE__);

	return 0;
}

static struct platform_driver aw3640_platform_driver = {
	.probe = aw3640_probe,
	.remove = aw3640_remove,
	.driver = {
		.name = AW3640_DEVNAME,
		.owner = THIS_MODULE,
		.of_match_table = aw3640_of_match,
	},
};

static struct platform_device aw3640_platform_device = {
	.name = AW3640_DEVNAME,
};

static int __init aw3640_init(void)
{
	int ret = 0;

	PK_DBG("%s:%d!\n", __func__, __LINE__);

	ret = platform_device_register(&aw3640_platform_device);
	if (ret) {
		printk(KERN_ERR "%s:%d: aw3640 register platform device failed!\n", __FUNCTION__, __LINE__);
		return ret;
	}

	ret = platform_driver_register(&aw3640_platform_driver);
	if (ret) {
		printk(KERN_ERR "%s:%d: aw3640 register platform driver failed!\n", __FUNCTION__, __LINE__);
		return ret;
	}

	return ret;
}

static void __exit aw3640_exit(void)
{
	PK_DBG("%s:%d!\n", __func__, __LINE__);
	platform_driver_unregister(&aw3640_platform_driver);
}

module_init(aw3640_init);
module_exit(aw3640_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("liaokesen");
MODULE_DESCRIPTION("aw3640 flashlight control driver");

#endif
/* AW3640 FLASHLIGHT DRIVER END */

int FL_Enable(void)
{
#ifdef CONFIG_FLASHLIGHT_AW3640
	aw3640_enable();
#endif

	return 0;
}

int FL_Disable(void)
{
#ifdef CONFIG_FLASHLIGHT_AW3640
	aw3640_disable();
#endif

	return 0;
}

int FL_dim_duty(kal_uint32 duty)
{
	PK_DBG(" FL_dim_duty line=%d\n", __LINE__);

#ifdef CONFIG_FLASHLIGHT_AW3640
	aw3640_dim_duty(duty);
#endif

	return 0;
}

int FL_Init(void)
{
	PK_DBG("%s:%d!\n", __func__, __LINE__);

	return 0;
}

int FL_Uninit(void)
{
	PK_DBG("%s:%d!\n", __func__, __LINE__);

	FL_Disable();
	return 0;
}


static void work_timeOutFunc(struct work_struct *data)
{
	FL_Disable();
	PK_DBG("ledTimeOut_callback\n");
}

enum hrtimer_restart ledTimeOutCallback(struct hrtimer *timer)
{
	schedule_work(&workTimeOut);
	return HRTIMER_NORESTART;
}

static struct hrtimer g_timeOutTimer;
void timerInit(void)
{
	static int init_flag;

	if (init_flag == 0) {
		init_flag = 1;
		INIT_WORK(&workTimeOut, work_timeOutFunc);
		g_timeOutTimeMs = 1000;
		hrtimer_init(&g_timeOutTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		g_timeOutTimer.function = ledTimeOutCallback;
	}
}

static int constant_flashlight_ioctl(unsigned int cmd, unsigned long arg)
{
	int i4RetValue = 0;
	int ior_shift;
	int iow_shift;
	int iowr_shift;

	ior_shift = cmd - (_IOR(FLASHLIGHT_MAGIC, 0, int));
	iow_shift = cmd - (_IOW(FLASHLIGHT_MAGIC, 0, int));
	iowr_shift = cmd - (_IOWR(FLASHLIGHT_MAGIC, 0, int));

	switch (cmd) {

	case FLASH_IOC_SET_TIME_OUT_TIME_MS:
		PK_DBG("FLASH_IOC_SET_TIME_OUT_TIME_MS: %d\n", (int)arg);
		g_timeOutTimeMs = arg;
		break;


	case FLASH_IOC_SET_DUTY:
		PK_DBG("FLASHLIGHT_DUTY: %d\n", (int)arg);
		FL_dim_duty(arg);
		break;


	case FLASH_IOC_SET_STEP:
		PK_DBG("FLASH_IOC_SET_STEP: %d\n", (int)arg);

		break;

	case FLASH_IOC_SET_ONOFF:
		PK_DBG("FLASHLIGHT_ONOFF: %d\n", (int)arg);
		if (arg == 1) {

			int s;
			int ms;

			if (g_timeOutTimeMs > 1000) {
				s = g_timeOutTimeMs / 1000;
				ms = g_timeOutTimeMs - s * 1000;
			} else {
				s = 0;
				ms = g_timeOutTimeMs;
			}

			if (g_timeOutTimeMs != 0) {
				ktime_t ktime;

				ktime = ktime_set(s, ms * 1000000);
				hrtimer_start(&g_timeOutTimer, ktime, HRTIMER_MODE_REL);
			}
			FL_Enable();
		} else {
			FL_Disable();
			hrtimer_cancel(&g_timeOutTimer);
		}
		break;
	default:
		PK_DBG(" No such command\n");
		i4RetValue = -EPERM;
		break;
	}
	return i4RetValue;
}

static int constant_flashlight_open(void *pArg)
{
	int i4RetValue = 0;

	PK_DBG("constant_flashlight_open line=%d\n", __LINE__);

	if (0 == strobe_Res) {
		FL_Init();
		timerInit();
	}
	PK_DBG("constant_flashlight_open line=%d\n", __LINE__);
	spin_lock_irq(&g_strobeSMPLock);


	if (strobe_Res) {
		PK_DBG(" busy!\n");
		i4RetValue = -EBUSY;
	} else {
		strobe_Res += 1;
	}


	spin_unlock_irq(&g_strobeSMPLock);
	PK_DBG("constant_flashlight_open line=%d\n", __LINE__);

	return i4RetValue;

}

static int constant_flashlight_release(void *pArg)
{
	PK_DBG(" constant_flashlight_release\n");

	if (strobe_Res) {
		spin_lock_irq(&g_strobeSMPLock);

		strobe_Res = 0;
		strobe_Timeus = 0;

		/* LED On Status */
		g_strobe_On = FALSE;

		spin_unlock_irq(&g_strobeSMPLock);

		FL_Uninit();
	}

	PK_DBG(" Done\n");

	return 0;

}

FLASHLIGHT_FUNCTION_STRUCT constantFlashlightFunc = {
	constant_flashlight_open,
	constant_flashlight_release,
	constant_flashlight_ioctl
};

MUINT32 constantFlashlightInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc)
{
	if (pfFunc != NULL)
		*pfFunc = &constantFlashlightFunc;
	return 0;
}

/* LED flash control for high current capture mode*/
ssize_t strobe_VDIrq(void)
{
	return 0;
}
EXPORT_SYMBOL(strobe_VDIrq);
