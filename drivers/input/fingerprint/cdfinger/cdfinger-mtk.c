#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/wakelock.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/semaphore.h>
#include <linux/poll.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/input.h>
#include <linux/signal.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/fb.h>
#include <linux/notifier.h>
#include <linux/regulator/consumer.h>
#include <linux/proc_fs.h>

#if defined(CONFIG_MTK_CLKMGR)
 /* mt_clkmgr.h will be removed after CCF porting is finished. */
#include <mach/mt_clkmgr.h>
#endif                          /* defined(CONFIG_MTK_CLKMGR) */

//#define PINBO_TEE_COMPATIBLE
#ifdef PINBO_TEE_COMPATIBLE
#include <tee_fp.h>
//#include "../../spi/mediatek/mt6735/mt_spi.h"
#include <mt_spi.h>
static struct mt_chip_conf chip_conf={
	.setuptime = 8,
	.holdtime = 8,
	.high_time = 13,
	.low_time = 13,
	.cs_idletime = 6,
	.cpol = 0,
	.cpha = 0,
	.rx_mlsb = 1,
	.tx_mlsb = 1,
	.tx_endian = 0,
	.rx_endian = 0,
	.com_mod = FIFO_TRANSFER,
	.pause = 1,
	.finish_intr = 1,
	.deassert = 0,
	.ulthigh = 0,
	.tckdly = 0,
};
#endif

//modify by zhoudan for less log
static u8 cdfinger_debug = 0x01;
static int isInKeyMode = 0; // key mode
static int screen_status = 1; // screen on
static int sign_sync = 0; // for poll
static int irq_success = 0; // for irq
typedef struct key_report{
	int key;
	int value;
}key_report_t;

#define CDFINGER_DBG(fmt, args...) \
	do{ \
		if(cdfinger_debug & 0x01) \
			printk( "[DBG][cdfinger]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
	}while(0)
#define CDFINGER_FUNCTION(fmt, args...) \
	do{ \
		if(cdfinger_debug & 0x02) \
			printk( "[DBG][cdfinger]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
	}while(0)
#define CDFINGER_REG(fmt, args...) \
	do{ \
		if(cdfinger_debug & 0x04) \
			printk( "[DBG][cdfinger]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
	}while(0)
#define CDFINGER_ERR(fmt, args...) \
    do{ \
		printk( "[DBG][cdfinger]:%5d: <%s>" fmt, __LINE__,__func__,##args ); \
    }while(0)

#define HAS_RESET_PIN

#define VERSION                         "cdfinger version 2.4"
#define DEVICE_NAME                     "fpsdev0"

#define CDFINGER_IOCTL_MAGIC_NO          0xFB
#define CDFINGER_INIT                    _IOW(CDFINGER_IOCTL_MAGIC_NO, 0, uint8_t)
#define CDFINGER_GETIMAGE                _IOW(CDFINGER_IOCTL_MAGIC_NO, 1, uint8_t)
#define CDFINGER_INITERRUPT_MODE	     _IOW(CDFINGER_IOCTL_MAGIC_NO, 2, uint8_t)
#define CDFINGER_INITERRUPT_KEYMODE      _IOW(CDFINGER_IOCTL_MAGIC_NO, 3, uint8_t)
#define CDFINGER_INITERRUPT_FINGERUPMODE _IOW(CDFINGER_IOCTL_MAGIC_NO, 4, uint8_t)
#define CDFINGER_RELEASE_WAKELOCK        _IO(CDFINGER_IOCTL_MAGIC_NO, 5)
#define CDFINGER_CHECK_INTERRUPT         _IO(CDFINGER_IOCTL_MAGIC_NO, 6)
#define CDFINGER_SET_SPI_SPEED           _IOW(CDFINGER_IOCTL_MAGIC_NO, 7, uint8_t)
#define CDFINGER_REPORT_KEY_LEGACY              _IOW(CDFINGER_IOCTL_MAGIC_NO, 10, uint8_t)
#define CDFINGER_REPORT_KEY              _IOW(CDFINGER_IOCTL_MAGIC_NO, 20, key_report_t)
#define CDFINGER_POWERDOWN               _IO(CDFINGER_IOCTL_MAGIC_NO, 11)
#define CDFINGER_ENABLE_IRQ               _IO(CDFINGER_IOCTL_MAGIC_NO, 12)
#define CDFINGER_DISABLE_IRQ               _IO(CDFINGER_IOCTL_MAGIC_NO, 13)
#define CDFINGER_HW_RESET               _IOW(CDFINGER_IOCTL_MAGIC_NO, 14, uint8_t)
#define CDFINGER_GET_STATUS               _IO(CDFINGER_IOCTL_MAGIC_NO, 15)
#define CDFINGER_SPI_CLK               _IOW(CDFINGER_IOCTL_MAGIC_NO, 16, uint8_t)
#define CDFINGER_KEY_REPORT		  _IOW(CDFINGER_IOCTL_MAGIC_NO,19,key_report_t)
#define CDFINGER_INIT_IRQ		 _IO(CDFINGER_IOCTL_MAGIC_NO,21)
#define CDFINGER_POWER_ON		 _IO(CDFINGER_IOCTL_MAGIC_NO,22)
#define CDFINGER_RESET		 	 _IO(CDFINGER_IOCTL_MAGIC_NO,23)
#define CDFINGER_RELEASE_DEVICE	 _IO(CDFINGER_IOCTL_MAGIC_NO,25)
#define CDFINGER_WAKE_LOCK	 _IOW(CDFINGER_IOCTL_MAGIC_NO,26,uint8_t)
#define CDFINGER_POLL_TRIGGER			 _IO(CDFINGER_IOCTL_MAGIC_NO,31)
#define CDFINGER_NEW_KEYMODE		_IOW(CDFINGER_IOCTL_MAGIC_NO, 37, uint8_t)
#define KEY_INTERRUPT                   KEY_F11

enum work_mode {
	CDFINGER_MODE_NONE       = 1<<0,
	CDFINGER_INTERRUPT_MODE  = 1<<1,
	CDFINGER_KEY_MODE        = 1<<2,
	CDFINGER_FINGER_UP_MODE  = 1<<3,
	CDFINGER_READ_IMAGE_MODE = 1<<4,
	CDFINGER_MODE_MAX
};

static struct cdfinger_data {
	struct platform_device *pdev;
	struct miscdevice *miscdev;
	struct mutex buf_lock;
	unsigned int irq;
	unsigned int irq_gpio;
	unsigned int rst_gpio;
	int irq_enabled;

	u32 vdd_ldo_enable;
	u32 vio_ldo_enable;
	u32 config_spi_pin;

	struct pinctrl *fps_pinctrl;
	struct pinctrl_state *fps_reset_high;
	struct pinctrl_state *fps_reset_low;
	struct pinctrl_state *fps_power_on;
	struct pinctrl_state *fps_power_off;
	struct pinctrl_state *fps_vio_on;
	struct pinctrl_state *fps_vio_off;
	struct pinctrl_state *cdfinger_spi_miso;
	struct pinctrl_state *cdfinger_spi_mosi;
	struct pinctrl_state *cdfinger_spi_sck;
	struct pinctrl_state *cdfinger_spi_cs;
	struct pinctrl_state *cdfinger_irq;

	int thread_wakeup;
	int process_interrupt;
	int key_report;
	enum work_mode device_mode;
	struct timer_list int_timer;
	struct input_dev *cdfinger_inputdev;
	struct wake_lock cdfinger_lock;
	struct task_struct *cdfinger_thread;
	struct fasync_struct *async_queue;
	uint8_t cdfinger_interrupt;
	struct notifier_block notifier;
	struct regulator *vdd;
}*g_cdfinger;

extern char *saved_command_line;

static DECLARE_WAIT_QUEUE_HEAD(waiter);
static DECLARE_WAIT_QUEUE_HEAD(cdfinger_waitqueue);
static irqreturn_t cdfinger_interrupt_handler(unsigned irq, void *arg);

#define PROC_NAME  "fp_info"
static struct proc_dir_entry *proc_entry=NULL;

// clk will follow platform... pls check this when you poarting
static void enable_clk(void)
{
#if (!defined(CONFIG_MT_SPI_FPGA_ENABLE))
//#if defined(CONFIG_MTK_CLKMGR)

        clkmux_sel(MT_CLKMUX_SPI_GFMUX_SEL, MT_CG_UPLL_D12, "spi");
        enable_clock(MT_CG_SPI_SW_CG, "spi");
//#endif
#endif
}

static void disable_clk(void)
{
#if (!defined(CONFIG_MT_SPI_FPGA_ENABLE))
//#if defined(CONFIG_MTK_CLKMGR)
        disable_clock(MT_CG_SPI_SW_CG, "spi");
        clkmux_sel(MT_CLKMUX_SPI_GFMUX_SEL, MT_CG_SYS_26M, "spi");
//#endif
#endif

}

static void cdfinger_disable_irq(struct cdfinger_data *cdfinger)
{
	if(cdfinger->irq_enabled == 1)
	{
		disable_irq_nosync(cdfinger->irq);
		cdfinger->irq_enabled = 0;
		CDFINGER_DBG("irq disable\n");
	}
}

static void cdfinger_enable_irq(struct cdfinger_data *cdfinger)
{
	if(cdfinger->irq_enabled == 0)
	{
		enable_irq(cdfinger->irq);
		cdfinger->irq_enabled =1;
		CDFINGER_DBG("irq enable\n");
	}
}
static int cdfinger_getirq_from_platform(struct cdfinger_data *cdfinger)
{
	u32 ints[2];
	if(!(cdfinger->pdev->dev.of_node)){
		CDFINGER_ERR("of node not exist!\n");
		return -1;
	}
	
	of_property_read_u32_array(cdfinger->pdev->dev.of_node, "debounce",ints,ARRAY_SIZE(ints));
	cdfinger->irq_gpio = ints[0];
	CDFINGER_DBG("irq_gpio[%d]  \n",cdfinger->irq_gpio);

	cdfinger->irq = irq_of_parse_and_map(cdfinger->pdev->dev.of_node, 0);
	if(cdfinger->irq < 0)
	{
		CDFINGER_ERR("parse irq failed! irq[%d]\n",cdfinger->irq);
		return -1;
	}
	CDFINGER_DBG("get irq success! irq[%d]\n",cdfinger->irq);
	//pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->cdfinger_irq);
	return 0;
}
static int cdfinger_parse_dts(struct cdfinger_data *cdfinger)
{
	int ret = -1;	
	u32 rst[2];
#ifndef DTS_PROBE
	cdfinger->pdev->dev.of_node = of_find_compatible_node(NULL,NULL,"cdfinger,fps1098");
#endif

	cdfinger->vdd_ldo_enable = 1;
	cdfinger->vio_ldo_enable = 1;
	cdfinger->config_spi_pin = 1;
	of_property_read_u32_array(cdfinger->pdev->dev.of_node, "resetgpio",rst,ARRAY_SIZE(rst));
	cdfinger->rst_gpio = rst[0];
	CDFINGER_DBG("rst_gpio[%d]  \n",cdfinger->rst_gpio);

	CDFINGER_DBG("vdd_ldo_enable[%d], vio_ldo_enable[%d], config_spi_pin[%d]\n",
		cdfinger->vdd_ldo_enable, cdfinger->vio_ldo_enable, cdfinger->config_spi_pin);

	cdfinger->fps_pinctrl = devm_pinctrl_get(&cdfinger->pdev->dev);
	if (IS_ERR(cdfinger->fps_pinctrl)) {
		ret = PTR_ERR(cdfinger->fps_pinctrl);
		CDFINGER_ERR("Cannot find fingerprint cdfinger->fps_pinctrl! ret=%d\n", ret);
		goto parse_err;
	}

	cdfinger->cdfinger_irq = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_irq");
	if (IS_ERR(cdfinger->cdfinger_irq))
	{
		ret = PTR_ERR(cdfinger->cdfinger_irq);
		CDFINGER_ERR("cdfinger->cdfinger_irq ret = %d\n",ret);
		goto parse_err;
	}
	cdfinger->fps_reset_low = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_reset_low");
	if (IS_ERR(cdfinger->fps_reset_low))
	{
		ret = PTR_ERR(cdfinger->fps_reset_low);
		CDFINGER_ERR("cdfinger->fps_reset_low ret = %d\n",ret);
		goto parse_err;
	}
	cdfinger->fps_reset_high = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_reset_high");
	if (IS_ERR(cdfinger->fps_reset_high))
	{
		ret = PTR_ERR(cdfinger->fps_reset_high);
		CDFINGER_ERR("cdfinger->fps_reset_high ret = %d\n",ret);
		goto parse_err;
	}

	if(cdfinger->config_spi_pin == 1)
	{
		cdfinger->cdfinger_spi_miso = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_spi_miso");
		if (IS_ERR(cdfinger->cdfinger_spi_miso))
		{
			ret = PTR_ERR(cdfinger->cdfinger_spi_miso);
			CDFINGER_ERR("cdfinger->cdfinger_spi_miso ret = %d\n",ret);
			goto parse_err;
		}
		cdfinger->cdfinger_spi_mosi = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_spi_mosi");
		if (IS_ERR(cdfinger->cdfinger_spi_mosi))
		{
			ret = PTR_ERR(cdfinger->cdfinger_spi_mosi);
			CDFINGER_ERR("cdfinger->cdfinger_spi_mosi ret = %d\n",ret);
			goto parse_err;
		}
		cdfinger->cdfinger_spi_sck = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_spi_sck");
		if (IS_ERR(cdfinger->cdfinger_spi_sck))
		{
			ret = PTR_ERR(cdfinger->cdfinger_spi_sck);
			CDFINGER_ERR("cdfinger->cdfinger_spi_sck ret = %d\n",ret);
			goto parse_err;
		}
		cdfinger->cdfinger_spi_cs = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_spi_cs");
		if (IS_ERR(cdfinger->cdfinger_spi_cs))
		{
			ret = PTR_ERR(cdfinger->cdfinger_spi_cs);
			CDFINGER_ERR("cdfinger->cdfinger_spi_cs ret = %d\n",ret);
			goto parse_err;
		}
	}

	if(cdfinger->vdd_ldo_enable == 1)
	{
		cdfinger->fps_power_on = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_power_high");
		if (IS_ERR(cdfinger->fps_power_on))
		{
			ret = PTR_ERR(cdfinger->fps_power_on);
			CDFINGER_ERR("cdfinger->fps_power_on ret = %d\n",ret);
			goto parse_err;
		}

		cdfinger->fps_power_off = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_power_low");
		if (IS_ERR(cdfinger->fps_power_off))
		{
			ret = PTR_ERR(cdfinger->fps_power_off);
			CDFINGER_ERR("cdfinger->fps_power_off ret = %d\n",ret);
			goto parse_err;
		}
	}

	if(cdfinger->vio_ldo_enable == 1)
	{
		cdfinger->fps_vio_on = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_vio_high");
		if (IS_ERR(cdfinger->fps_vio_on))
		{
			ret = PTR_ERR(cdfinger->fps_vio_on);
			CDFINGER_ERR("cdfinger->fps_vio_on ret = %d\n",ret);
			goto parse_err;
		}

		cdfinger->fps_vio_off = pinctrl_lookup_state(cdfinger->fps_pinctrl,"fingerprint_vio_low");
		if (IS_ERR(cdfinger->fps_vio_off))
		{
			ret = PTR_ERR(cdfinger->fps_vio_off);
			CDFINGER_ERR("cdfinger->fps_vio_off ret = %d\n",ret);
			goto parse_err;
		}
	}

	return 0;
parse_err:
	CDFINGER_ERR("parse dts failed!\n");

	return ret;
}


static void cdfinger_power_on(struct cdfinger_data *cdfinger)
{
	int ret = -100;
	if(cdfinger->config_spi_pin == 1)
	{
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->cdfinger_spi_miso);
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->cdfinger_spi_mosi);
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->cdfinger_spi_sck);
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->cdfinger_spi_cs);
	}

	if(cdfinger->vdd_ldo_enable == 1)
	{
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->fps_power_on);
	}

	if(cdfinger->vio_ldo_enable == 1)
	{
		pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->fps_vio_on);
	}
	cdfinger->vdd = regulator_get(NULL, "vio28");
	if (IS_ERR(cdfinger->vdd)) {
		CDFINGER_DBG("cdfinger: Didn't find vcc_core regulator\n");
		cdfinger->vdd = NULL;
	} else {
		CDFINGER_DBG("cdfinger: Found vcc_core regulator\n");
	}
	ret = regulator_set_voltage(cdfinger->vdd, 2800000, 2800000);
		CDFINGER_DBG("cdfinger regulator_set_voltage: vdd  ret = %d\n", ret);
	ret = regulator_enable(cdfinger->vdd);
		CDFINGER_DBG("cdfinger regulator_enable: vdd  ret = %d\n", ret);
}

#ifdef HAS_RESET_PIN
static void cdfinger_reset(int count)
{
	struct cdfinger_data *cdfinger = g_cdfinger;
	pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->fps_reset_low);
	mdelay(count);
	pinctrl_select_state(cdfinger->fps_pinctrl, cdfinger->fps_reset_high);
	mdelay(count);
}
#endif

static void cdfinger_release_wakelock(struct cdfinger_data *cdfinger)
{
	CDFINGER_FUNCTION("enter\n");
	wake_unlock(&cdfinger->cdfinger_lock);
	CDFINGER_FUNCTION("exit\n");
}

static int cdfinger_mode_init(struct cdfinger_data *cdfinger, uint8_t arg, enum work_mode mode)
{
	CDFINGER_DBG("mode=0x%x\n", mode);
	cdfinger->process_interrupt = 1;
	cdfinger->device_mode = mode;
	cdfinger->key_report = 0;

	return 0;
}

static void cdfinger_wake_lock(struct cdfinger_data *pdata,int arg)
{
	CDFINGER_DBG("cdfinger_wake_lock enter----------\n");
	if(arg)
	{
		wake_lock(&pdata->cdfinger_lock);
	}
	else
	{
		wake_unlock(&pdata->cdfinger_lock);
		wake_lock_timeout(&pdata->cdfinger_lock, msecs_to_jiffies(3000));
	}
}

int cdfinger_key_report(struct cdfinger_data *pdata, unsigned long arg)
{

	key_report_t __user *report = (key_report_t __user *)arg;
	printk("cdfinger key=0x%04x,value = %d \n",report->key,report->value);
	input_report_key(pdata->cdfinger_inputdev, report->key, !!report->value);
	input_sync(pdata->cdfinger_inputdev);
	return 0;
}

int cdfinger_report_key(struct cdfinger_data *cdfinger, unsigned long arg)
{
	key_report_t __user *report = (key_report_t __user *)arg;
	switch(report->key)
	{
	case KEY_UP:
		report->key=KEY_VOLUMEDOWN;
		break;
	case KEY_DOWN:
		report->key=KEY_VOLUMEUP;
		break;
	case KEY_RIGHT:
		report->key=KEY_PAGEUP;
		break;
	case KEY_LEFT:
		report->key=KEY_PAGEDOWN;
		break;
	default:
		break;
	}

	CDFINGER_FUNCTION("enter\n");
	input_report_key(cdfinger->cdfinger_inputdev, report->key, !!report->value);
	input_sync(cdfinger->cdfinger_inputdev);
	CDFINGER_FUNCTION("exit\n");

	return 0;
}

int cdfinger_report_key_legacy(struct cdfinger_data *cdfinger, uint8_t arg)
{
	CDFINGER_FUNCTION("enter\n");
	input_report_key(cdfinger->cdfinger_inputdev, KEY_INTERRUPT, !!arg);
	input_sync(cdfinger->cdfinger_inputdev);
	CDFINGER_FUNCTION("exit\n");

	return 0;
}

static unsigned int cdfinger_poll(struct file *filp, struct poll_table_struct *wait)
{
	int mask = 0;
	poll_wait(filp, &cdfinger_waitqueue, wait);
	if (sign_sync == 1)
	{
		mask |= POLLIN|POLLPRI;
	} else if (sign_sync == 2)
	{
		mask |= POLLOUT;
	}
	sign_sync = 0;
	CDFINGER_DBG("mask %u\n",mask);
	return mask;
}

static int cdfinger_eint_gpio_init(struct cdfinger_data *cdfinger)
{
	int error = 0;
	if(!irq_success)
	{
	
		if(cdfinger_getirq_from_platform(cdfinger)!=0)
		{
			CDFINGER_ERR("cdfinger_getirq_from_platform error\n");
			return -1;
		}

		error = request_threaded_irq(cdfinger->irq, (irq_handler_t)cdfinger_interrupt_handler, NULL,
						IRQF_TRIGGER_RISING | IRQF_ONESHOT, "cdfinger-irq", cdfinger);
		if(error){
			CDFINGER_ERR("request_irq error\n");
			return -1;
		}
		enable_irq_wake(cdfinger->irq);
		cdfinger->irq_enabled = 1;
		irq_success = 1;
		CDFINGER_DBG("%s(..) irq_success = %d.\n", __FUNCTION__,irq_success);
	}
	return error;
}

static int cdfinger_free_gpio(struct cdfinger_data *cdfinger)
{
	int err = 0;
	CDFINGER_DBG("%s(..) enter.\n", __FUNCTION__);

	if (gpio_is_valid(cdfinger->irq_gpio)) {
		gpio_free(cdfinger->irq_gpio);
	}
	
	if (gpio_is_valid(cdfinger->rst_gpio)) {
	CDFINGER_DBG("%s(..) cdfinger->rst_gpio %d.\n", __FUNCTION__, cdfinger->rst_gpio);
		pinctrl_free_gpio(cdfinger->rst_gpio);
		gpio_free(cdfinger->rst_gpio);
	}
	
#if 0
	if (gpio_is_valid(cdfinger->reset_num)) {
		gpio_free(cdfinger->reset_num);
	}

	if (gpio_is_valid(cdfinger->pwr_num)) {
		gpio_free(cdfinger->pwr_num);
	}
#endif

	CDFINGER_DBG("%s(..) ok! exit.\n", __FUNCTION__);

	return err;
}

// add for fingerprint check list start
static int proc_show_ver(struct seq_file *file,void *v)
{
// modify by zhoudan for fp checklist
	seq_printf(file,"[Vendor]Fps998se,cdfinger\n");
	return 0;
}

static int proc_open(struct inode *inode,struct file *file)
{
	printk("cdfinger proc_open\n");
	single_open(file,proc_show_ver,NULL);
	return 0;
}

static const struct file_operations proc_file_ops = {
	.owner = THIS_MODULE,
	.open = proc_open,
	.read = seq_read,
	.release = single_release,
};
// add for fingerprint check list end

static long cdfinger_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct cdfinger_data *cdfinger = filp->private_data;
	int ret = 0;

	CDFINGER_FUNCTION("enter cmd=%d\n", cmd);
	if(cdfinger == NULL)
	{
		CDFINGER_ERR("%s: fingerprint please open device first!\n", __func__);
		return -EIO;
	}

	mutex_lock(&cdfinger->buf_lock);
	switch (cmd) {
		case CDFINGER_INIT:
			break;
		case CDFINGER_GETIMAGE:
			break;
		case CDFINGER_INIT_IRQ:
			ret = cdfinger_eint_gpio_init(cdfinger);
		// add for fingerprint check list start
			if (NULL == proc_entry)
			{
				proc_entry = proc_create(PROC_NAME, 0777, NULL, &proc_file_ops);
				if (NULL == proc_entry)
				{
					printk("cdfinger Couldn't create proc entry!");
				}
				else
				{
					printk("cdfinger Create proc entry success!");
				}
			}
		// add for fingerprint check list end
			break;
		case CDFINGER_WAKE_LOCK:
			cdfinger_wake_lock(cdfinger,arg);
			break;
		case CDFINGER_INITERRUPT_MODE:
			sign_sync = 0;
			isInKeyMode = 1;  // not key mode
			cdfinger_reset(2);
			ret = cdfinger_mode_init(cdfinger,arg,CDFINGER_INTERRUPT_MODE);
			break;
		case CDFINGER_NEW_KEYMODE:
			isInKeyMode = 0;
			ret = cdfinger_mode_init(cdfinger,arg,CDFINGER_INTERRUPT_MODE);
			break;
		case CDFINGER_KEY_REPORT:
			ret = cdfinger_key_report(cdfinger,arg);
			break;
		case CDFINGER_INITERRUPT_FINGERUPMODE:
			ret = cdfinger_mode_init(cdfinger,arg,CDFINGER_FINGER_UP_MODE);
			break;
		case CDFINGER_RELEASE_WAKELOCK:
			cdfinger_release_wakelock(cdfinger);
			break;
		case CDFINGER_INITERRUPT_KEYMODE:
			ret = cdfinger_mode_init(cdfinger,arg,CDFINGER_KEY_MODE);
			break;
		case CDFINGER_CHECK_INTERRUPT:
			break;
		case CDFINGER_SET_SPI_SPEED:
			break;
		case CDFINGER_REPORT_KEY:
			ret = cdfinger_report_key(cdfinger, arg);
			break;
		case CDFINGER_REPORT_KEY_LEGACY:
			ret = cdfinger_report_key_legacy(cdfinger, arg);
			break;
		case CDFINGER_POWER_ON:
			cdfinger_power_on(cdfinger);
			break;
		case CDFINGER_POWERDOWN:
			break;
		case CDFINGER_RESET:
			cdfinger_reset(100);
			break;
		case CDFINGER_ENABLE_IRQ:
			cdfinger_enable_irq(cdfinger);
			break;
		case CDFINGER_DISABLE_IRQ:
			cdfinger_disable_irq(cdfinger);
			break;
		case CDFINGER_RELEASE_DEVICE:
			if (NULL != proc_entry)
				remove_proc_entry(PROC_NAME,NULL);
	CDFINGER_DBG(" %s(..) CDFINGER_RELEASE_DEVICE.\n", __FUNCTION__);
			cdfinger_free_gpio(cdfinger);
			if (cdfinger->cdfinger_inputdev != NULL) {
	input_unregister_device(cdfinger->cdfinger_inputdev);
			}
	CDFINGER_DBG(" %s(..) input_unregister_device.\n", __FUNCTION__);
			misc_deregister(cdfinger->miscdev);
			if(cdfinger->fps_pinctrl){
				devm_pinctrl_put(cdfinger->fps_pinctrl);
			}
	CDFINGER_DBG(" %s(..) devm_pinctrl_put.\n", __FUNCTION__);
			break;
		case CDFINGER_SPI_CLK:
			if (arg == 1)
				enable_clk();
			else if (arg == 0)
				disable_clk();
			break;
		case CDFINGER_HW_RESET:
			cdfinger_reset(arg);
			break;
		case CDFINGER_GET_STATUS:
			ret = screen_status;
			break;
		case CDFINGER_POLL_TRIGGER:
			sign_sync = 2;
			wake_up_interruptible(&cdfinger_waitqueue);
			ret = 0;
			break;
		default:
			ret = -ENOTTY;
			break;
	}
	mutex_unlock(&cdfinger->buf_lock);
	CDFINGER_FUNCTION("exit\n");

	return ret;
}

static int cdfinger_open(struct inode *inode, struct file *file)
{
	CDFINGER_FUNCTION("enter\n");
	file->private_data = g_cdfinger;
	CDFINGER_FUNCTION("exit\n");

	return 0;
}

static ssize_t cdfinger_write(struct file *file, const char *buff, size_t count, loff_t * ppos)
{
	return 0;
}

static int cdfinger_async_fasync(int fd, struct file *filp, int mode)
{
	struct cdfinger_data *cdfinger = g_cdfinger;

	CDFINGER_FUNCTION("enter\n");
	return fasync_helper(fd, filp, mode, &cdfinger->async_queue);
}

static ssize_t cdfinger_read(struct file *file, char *buff, size_t count, loff_t * ppos)
{
	return 0;
}

static int cdfinger_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;

	return 0;
}

static const struct file_operations cdfinger_fops = {
	.owner = THIS_MODULE,
	.open = cdfinger_open,
	.write = cdfinger_write,
	.read = cdfinger_read,
	.release = cdfinger_release,
	.fasync = cdfinger_async_fasync,
	.unlocked_ioctl = cdfinger_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = cdfinger_ioctl,
#endif
	.poll = cdfinger_poll,
};

static struct miscdevice cdfinger_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &cdfinger_fops,
};

static void cdfinger_async_Report(void)
{
	//disable by zhoudan for compile fail
	//struct cdfinger_data *cdfinger = g_cdfinger;

	CDFINGER_FUNCTION("enter\n");
	//disable by zhoudan for fingerprint fail
	//kill_fasync(&cdfinger->async_queue, SIGIO, POLL_IN);
	CDFINGER_FUNCTION("exit\n");
}

static void int_timer_handle(unsigned long arg)
{
	struct cdfinger_data *cdfinger = g_cdfinger;

	CDFINGER_DBG("enter\n");
	if ((cdfinger->device_mode == CDFINGER_KEY_MODE) && (cdfinger->key_report == 1)) {
		input_report_key(cdfinger->cdfinger_inputdev, KEY_INTERRUPT, 0);
		input_sync(cdfinger->cdfinger_inputdev);
		cdfinger->key_report = 0;
	}

	if (cdfinger->device_mode == CDFINGER_FINGER_UP_MODE){
		cdfinger->process_interrupt = 0;
		cdfinger_async_Report();
	}
	CDFINGER_DBG("exit\n");
}

static int cdfinger_thread_func(void *arg)
{
	struct cdfinger_data *cdfinger = (struct cdfinger_data *)arg;

	do {
		wait_event_interruptible(waiter, cdfinger->thread_wakeup != 0);
		CDFINGER_DBG("cdfinger:%s,thread wakeup\n",__func__);
		cdfinger->thread_wakeup = 0;
		wake_lock_timeout(&cdfinger->cdfinger_lock, 3*HZ);

		if (cdfinger->device_mode == CDFINGER_INTERRUPT_MODE) {
			cdfinger->process_interrupt = 0;
			sign_sync = 1;
			wake_up_interruptible(&cdfinger_waitqueue);
			cdfinger_async_Report();
			del_timer_sync(&cdfinger->int_timer);
			continue;
		} else if ((cdfinger->device_mode == CDFINGER_KEY_MODE) && (cdfinger->key_report == 0)) {
			input_report_key(cdfinger->cdfinger_inputdev, KEY_INTERRUPT, 1);
			input_sync(cdfinger->cdfinger_inputdev);
			cdfinger->key_report = 1;
		}

	}while(!kthread_should_stop());

	CDFINGER_ERR("thread exit\n");
	return -1;
}

static irqreturn_t cdfinger_interrupt_handler(unsigned irq, void *arg)
{
	struct cdfinger_data *cdfinger = (struct cdfinger_data *)arg;

	cdfinger->cdfinger_interrupt = 1;
	if (cdfinger->process_interrupt == 1)
	{
		mod_timer(&cdfinger->int_timer, jiffies + HZ / 10);
		cdfinger->thread_wakeup = 1;
		wake_up_interruptible(&waiter);
	}

	return IRQ_HANDLED;
}

static int cdfinger_create_inputdev(struct cdfinger_data *cdfinger)
{
	cdfinger->cdfinger_inputdev = input_allocate_device();
	if (!cdfinger->cdfinger_inputdev) {
		CDFINGER_ERR("cdfinger->cdfinger_inputdev create faile!\n");
		return -ENOMEM;
	}
	__set_bit(EV_KEY, cdfinger->cdfinger_inputdev->evbit);
	__set_bit(KEY_INTERRUPT, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_F1, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_F2, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_F3, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_F4, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_F5, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_F6, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_VOLUMEUP, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_VOLUMEDOWN, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_PAGEUP, cdfinger->cdfinger_inputdev->keybit);
    __set_bit(KEY_PAGEDOWN, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_UP, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_LEFT, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_RIGHT, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_DOWN, cdfinger->cdfinger_inputdev->keybit);
	__set_bit(KEY_ENTER, cdfinger->cdfinger_inputdev->keybit);

	cdfinger->cdfinger_inputdev->id.bustype = BUS_HOST;
	cdfinger->cdfinger_inputdev->name = "cdfinger_inputdev";
	if (input_register_device(cdfinger->cdfinger_inputdev)) {
		CDFINGER_ERR("register inputdev failed\n");
		input_free_device(cdfinger->cdfinger_inputdev);
		return -1;
	}

	return 0;
}

static int cdfinger_fb_notifier_callback(struct notifier_block* self,
                                        unsigned long event, void* data)
{
    struct fb_event* evdata = data;
    unsigned int blank;
    int retval = 0;
	
    if (event != FB_EVENT_BLANK /* FB_EARLY_EVENT_BLANK */) {
        return 0;
    }
    blank = *(int*)evdata->data;
    switch (blank) {
        case FB_BLANK_UNBLANK:
			CDFINGER_DBG("sunlin==FB_BLANK_UNBLANK==\n");
			mutex_lock(&g_cdfinger->buf_lock);
			screen_status = 1;
			if (isInKeyMode == 0) {
				sign_sync = 1;
				wake_up_interruptible(&cdfinger_waitqueue);
				cdfinger_async_Report();
			}
			mutex_unlock(&g_cdfinger->buf_lock);
            break;

        case FB_BLANK_POWERDOWN:
			CDFINGER_DBG("sunlin==FB_BLANK_POWERDOWN==\n");
			mutex_lock(&g_cdfinger->buf_lock);
			screen_status = 0;
			if (isInKeyMode == 0) {
				sign_sync = 1;
				wake_up_interruptible(&cdfinger_waitqueue);
				cdfinger_async_Report();
			}
			mutex_unlock(&g_cdfinger->buf_lock);
            break;
        default:
            break;
    }

    return retval;
}


static int cdfinger_probe(struct platform_device *pdev)
{
	char *pstr_sensor = NULL;
	struct cdfinger_data *cdfinger = NULL;
	int status = -ENODEV;
#ifdef PINBO_TEE_COMPATIBLE
	uint8_t chipid[4] = {0x74, 0x66, 0x66, 0x66};
#endif
	CDFINGER_DBG("enter\n");

	pstr_sensor = strstr(saved_command_line, "fp_sensor=");
	if (pstr_sensor == NULL)
		return -ENOMEM;
	pstr_sensor += strlen("fp_sensor=");
	if( simple_strtol(pstr_sensor, NULL, 10) != 2)
	{
		CDFINGER_ERR("fingerprint is not cdfinger! \n");
		return -ENOMEM;
	}
	
	cdfinger = kzalloc(sizeof(struct cdfinger_data), GFP_KERNEL);
	if (!cdfinger) {
		CDFINGER_ERR("alloc cdfinger failed!\n");
		return -ENOMEM;;
	}

	g_cdfinger = cdfinger;
	cdfinger->pdev = pdev;

	if(cdfinger_parse_dts(cdfinger))
        {
                CDFINGER_ERR("%s: parse dts failed!\n", __func__);
                goto free_cdfinger;
        }

	//cdfinger_power_on(cdfinger);
#ifdef HAS_RESET_PIN
	//cdfinger_reset(100);
#endif

#ifdef PINBO_TEE_COMPATIBLE
	status = tee_spi_transfer((void*)&chip_conf, sizeof(chip_conf), chipid, chipid, 4);
	if (status == 0) {
		if (chipid[3] == 0x80 || chipid[3] == 0x98 || chipid[3] == 0x56){
			CDFINGER_DBG("get id success(%x)\n",chipid[3]);		
		} else {
			CDFINGER_DBG("get id failed(%x)\n",chipid[3]);
			status = -1;			
			goto free_cdfinger;
		}
	} else {
		CDFINGER_ERR("spi invoke err()\n");
		status = -1;
		goto free_cdfinger;	
	}
#endif
	mutex_init(&cdfinger->buf_lock);
	wake_lock_init(&cdfinger->cdfinger_lock, WAKE_LOCK_SUSPEND, "cdfinger wakelock");

	status = misc_register(&cdfinger_dev);
	if (status < 0) {
		CDFINGER_ERR("%s: cdev register failed!\n", __func__);
		goto free_lock;
	}
	cdfinger->miscdev = &cdfinger_dev;

	if(cdfinger_create_inputdev(cdfinger) < 0)
	{
		CDFINGER_ERR("%s: inputdev register failed!\n", __func__);
		goto free_device;
	}

	init_timer(&cdfinger->int_timer);
	cdfinger->int_timer.function = int_timer_handle;
	add_timer(&cdfinger->int_timer);
	/*
	if(cdfinger_getirq_from_platform(cdfinger)!=0)
		goto free_work;
	status = request_threaded_irq(cdfinger->irq, (irq_handler_t)cdfinger_interrupt_handler, NULL,
					IRQF_TRIGGER_RISING | IRQF_ONESHOT, "cdfinger-irq", cdfinger);
	if(status){
		CDFINGER_ERR("request_irq error\n");
		goto free_work;
	}
	

	enable_irq_wake(cdfinger->irq);
	cdfinger->irq_enabled = 1;
	*/
	
	cdfinger->cdfinger_thread = kthread_run(cdfinger_thread_func, cdfinger, "cdfinger_thread");
	if (IS_ERR(cdfinger->cdfinger_thread)) {
		CDFINGER_ERR("kthread_run is failed\n");
		goto free_irq;
	}
	cdfinger->notifier.notifier_call = cdfinger_fb_notifier_callback;
    	fb_register_client(&cdfinger->notifier);
	//enable_clk();   //if doujia  we can open this to debug
	CDFINGER_DBG("exit\n");

	return 0;

free_irq:
	free_irq(cdfinger->irq, cdfinger);
//free_work:
	del_timer(&cdfinger->int_timer);
	input_unregister_device(cdfinger->cdfinger_inputdev);
	cdfinger->cdfinger_inputdev = NULL;
	input_free_device(cdfinger->cdfinger_inputdev);
free_device:
	misc_deregister(&cdfinger_dev);
free_lock:
	wake_lock_destroy(&cdfinger->cdfinger_lock);
	mutex_destroy(&cdfinger->buf_lock);
free_cdfinger:
	kfree(cdfinger);
	cdfinger = NULL;

	return -1;
}


static const struct of_device_id cdfinger_of_match[] = {
	{ .compatible = "cdfinger,fingerprint_tee", },
	{},
};

static const struct platform_device_id cdfinger_id[] = {
	{"cdfinger_fp", 0},
	{}
};

struct platform_device cdfinger_tee_device = {
	.name = "cdfinger_fp",
};


static struct platform_driver cdfinger_driver = {
	.driver = {
		.name = "cdfinger_fp",
		.of_match_table = cdfinger_of_match,
	},
	.id_table	= cdfinger_id,
	.probe = cdfinger_probe,
};

static int __init cdfinger_fp_init(void)
{
	int ret;
	ret = platform_device_register(&cdfinger_tee_device); 
	if(ret){ 
		CDFINGER_ERR("device register failed!\n");
	} 
	platform_driver_register(&cdfinger_driver);
	return 0;
}

static void __exit cdfinger_fp_exit(void)
{
	platform_driver_unregister(&cdfinger_driver);
}
late_initcall_sync(cdfinger_fp_init);
module_exit(cdfinger_fp_exit);

MODULE_DESCRIPTION("cdfinger tee Driver");
MODULE_AUTHOR("shuaitao@cdfinger.com");
MODULE_LICENSE("GPL");
MODULE_ALIAS("cdfinger");
