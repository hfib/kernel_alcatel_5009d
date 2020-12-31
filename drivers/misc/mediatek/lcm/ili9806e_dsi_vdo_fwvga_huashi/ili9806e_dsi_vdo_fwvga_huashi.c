

#ifdef BUILD_LK
#include <string.h>
#include <mt_gpio.h>
#include <platform/mt_pmic.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <linux/string.h>
//#include <mach/mt_gpio.h>
//#include <mach/mt_pm_ldo.h>
#include <mt-plat/mt_gpio.h>
#include <mach/gpio_const.h>
#endif
//#include <cust_gpio_usage.h>
#include "lcm_drv.h"



#if defined(BUILD_LK)
#define LCM_PRINT printf
#elif defined(BUILD_UBOOT)
#define LCM_PRINT printf
#else
#define LCM_PRINT printk
#endif

#define GPIO_LCM_RST         (GPIO70 | 0x80000000)
#define GPIO_LCM_RST_M_GPIO  GPIO_MODE_00
#define GPIO_LCM_RST_M_EINT  GPIO_MODE_06
#define GPIO_LCM_RST_M_LCM_RST   GPIO_MODE_01
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
// pixel
#define FRAME_WIDTH  			(480)
#define FRAME_HEIGHT 			(960)
// physical dimension
#define PHYSICAL_WIDTH        (68)
#define PHYSICAL_HIGHT         (121)


#define LCM_ID       (0x40)
#define LCM_DSI_CMD_MODE		0

#define REGFLAG_DELAY 0xBB
#define REGFLAG_END_OF_TABLE 0xEE // END OF REGISTERS MARKER


// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))
#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V3(para_tbl, size, force_update)   	lcm_util.dsi_set_cmdq_V3(para_tbl, size, force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    


struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[128];
};

static struct LCM_setting_table lcm_initialization_setting_V3[] = {
	{0XFF, 5, {0XFF, 0X98, 0X06, 0X04, 0X01}},	//EXTC Command Set enable register (Page 1)
	{0X22, 1, {0X00}},	//Interface Mode Control 1
	{0X21, 1, {0X01}},	//Display Function Control 2
	
	{0X30, 1, {0X07}},	//Resolution Control (480*960)
	{0X2E, 1, {0X77}},    //xiaopei res 480*960
	
	{0X31, 1, {0X00}},	//2 Dot
	
	{0X60, 1, {0X07}},	//Source Timing Adjust1 (SDTI)
	{0X61, 1, {0X03}},	//Source Timing Adjust2 (CRTI)
	{0X62, 1, {0X06}},	//Source Timing Adjust3 (EQTI)
	{0X63, 1, {0X07}},	//Source Timing Adjust4 (EQTI)		
	
	{0X40, 1, {0X10}},
	{0X41, 1, {0X22}},
    {0X42, 1, {0X01}},
	{0X43, 1, {0X09}},
	{0X44, 1, {0X86}},

    {0X50, 1, {0X78}},
	{0X51, 1, {0X78}},
	{0X52, 1, {0X00}},
	{0X53, 1, {0X48}},//xiaopei vcom adjust 2018-1-22		
	{0X56, 1, {0X00}},
	
	//Begin xiaopei Positive Gamma Control
	{0XA0, 1, {0X00}},	//Positive Gamma Control (01h~16h)
	{0XA1, 1, {0X05}},
	{0XA2, 1, {0X0F}},
	{0XA3, 1, {0X11}},
	{0XA4, 1, {0X0B}},
	{0XA5, 1, {0X1E}},
	{0XA6, 1, {0X0C}},
	{0XA7, 1, {0X0C}},
	{0XA8, 1, {0X00}},
	{0XA9, 1, {0X05}},
	{0XAA, 1, {0X00}},
	{0XAB, 1, {0X00}},
	{0XAC, 1, {0X00}},
	{0XAD, 1, {0X3A}},
	{0XAE, 1, {0X37}},
	{0XAF, 1, {0X00}},
	
	{0XC0, 1, {0X00}},	//Negative Gamma Control (01h~16h)
	{0XC1, 1, {0X03}},
	{0XC2, 1, {0X0A}},
	{0XC3, 1, {0X0E}},
	{0XC4, 1, {0X08}},
	{0XC5, 1, {0X13}},
	{0XC6, 1, {0X0A}},
	{0XC7, 1, {0X06}},
	{0XC8, 1, {0X06}},
	{0XC9, 1, {0X0C}},
	{0XCA, 1, {0X0A}},
	{0XCB, 1, {0X08}},
	{0XCC, 1, {0X13}},
	{0XCD, 1, {0X2B}},
	{0XCE, 1, {0X24}},
	{0XCF, 1, {0X00}},
	//end xiaopei Negative Gamma Control
	{0XFF, 5, {0XFF, 0X98, 0X06, 0X04, 0X06}},	//Change to page 6
	{0X00, 1, {0X21}},	//External Power Setting
	{0X01, 1, {0X04}},
	{0X02, 1, {0X00}},
	{0X03, 1, {0X00}},
	{0X04, 1, {0X16}},
	{0X05, 1, {0X16}},
	{0X06, 1, {0X80}},
	{0X07, 1, {0X02}},
	{0X08, 1, {0X07}},
	{0X09, 1, {0X90}},
	{0X0A, 1, {0X01}},
	{0X0B, 1, {0X06}},
	{0X0C, 1, {0X15}},
	{0X0D, 1, {0X15}},
	{0X0E, 1, {0X00}},
	{0X0F, 1, {0X00}},
	{0X10, 1, {0X77}},
	
	{0X11, 1, {0Xf0}},
	{0X12, 1, {0X00}},
	{0X13, 1, {0X87}},
	{0X14, 1, {0X87}},
	{0X15, 1, {0XC0}},
	{0X16, 1, {0X08}},
	{0X17, 1, {0X00}},
	{0X18, 1, {0X00}},
	{0X19, 1, {0X00}},
	{0X1A, 1, {0X00}},
	{0X1B, 1, {0X00}},
	{0X1C, 1, {0X00}},
	{0X1D, 1, {0X00}},
	{0X20, 1, {0X01}},
	{0X21, 1, {0X23}},
	{0X22, 1, {0X45}},
	{0X23, 1, {0X67}},
	{0X24, 1, {0X01}},
	{0X25, 1, {0X23}},
	{0X26, 1, {0X45}},
	{0X27, 1, {0X67}},
	{0X30, 1, {0X11}},
	{0X31, 1, {0X22}},
	{0X32, 1, {0X11}},
	{0X33, 1, {0X00}},
	{0X34, 1, {0X86}},
	{0X35, 1, {0X68}},
	{0X36, 1, {0X22}},
	{0X37, 1, {0X22}},
	{0X38, 1, {0Xda}},
	{0X39, 1, {0Xbc}},
	{0X3A, 1, {0Xcb}},
	{0X3B, 1, {0Xad}},
	{0X3C, 1, {0X22}},
	{0X3D, 1, {0X22}},
	{0X3E, 1, {0X22}},
	{0X3F, 1, {0X22}},
	{0X40, 1, {0X22}},
	{0X52, 1, {0X10}},
	{0X53, 1, {0X10}},
	{0X54, 1, {0X13}},
	
	
	{0XFF, 5, {0XFF, 0X98, 0X06, 0X04, 0X07}},	//Change to page 7
	//{0X17, 1, {0X22}},
	//{0X02, 1, {0X77}},
	//{0Xe1, 1, {0X79}},
	//{0X26, 1, {0Xb2}},
	{0X06, 1, {0X13}},
	{0X18, 1, {0X1d}},
	
	
	{0XFF, 5, {0XFF, 0X98, 0X06, 0X04, 0X00}},	//Change to page 0
	//begin xiaopei modify resume white splash problem 12-11
	{0X11, 1, {0X00}},
	{REGFLAG_DELAY,120,{}},
	{0X29, 1, {0X00}},
	{REGFLAG_DELAY,20,{}},
	//xiaopei  2018-2-5 reduce powerup time
	{REGFLAG_END_OF_TABLE, 0x00,{}}

};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

	for(i = 0; i < count; i++) {
		unsigned cmd;
		
		cmd = table[i].cmd;

		switch (cmd) {
		case REGFLAG_DELAY:
			MDELAY(table[i].count);
			break;

		case REGFLAG_END_OF_TABLE:
			break;

		default:
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
	LCM_PRINT("[LCD] push_table \n");
}
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy((void*)&lcm_util, (void*)util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS * params) 

{ 
	memset(params, 0, sizeof(LCM_PARAMS)); 

	params->type   = LCM_TYPE_DSI;
	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

   params->physical_width=PHYSICAL_WIDTH;
   params->physical_height=PHYSICAL_HIGHT;

	// enable tearing-free
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
#else
	params->dsi.mode   = SYNC_PULSE_VDO_MODE;//SYNC_EVENT_VDO_MODE;
#endif

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine. 

	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;	
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST; 
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB; 
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888; 

	// Highly depends on LCD driver capability. 
	params->dsi.packet_size = 256; 
	// Video mode setting 
	params->dsi.intermediat_buffer_num = 2; 
	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888; 

	params->dsi.vertical_sync_active = 2; 
	params->dsi.vertical_backporch = 20; 
	params->dsi.vertical_frontporch = 20; 
	params->dsi.vertical_active_line = FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 10;
	params->dsi.horizontal_backporch				= 20;
	params->dsi.horizontal_frontporch				= 80;
	params->dsi.horizontal_active_pixel 			= FRAME_WIDTH;
    //Beign xiaopei add ESD 12-19
	params->dsi.esd_check_enable = 1; 
	params->dsi.customization_esd_check_enable = 1; 
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A; 
	params->dsi.lcm_esd_check_table[0].count = 1; 
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C; 
	//End add ESD 
	// Bit rate calculation
	params->dsi.PLL_CLOCK = 228;

	LCM_PRINT("[LCD] lcm_get_params \n");

}

static void init_lcm_registers(void)
{
	push_table(lcm_initialization_setting_V3, sizeof(lcm_initialization_setting_V3) / sizeof(LCM_setting_table_V3), 1);
	LCM_PRINT("[LCD] init_lcm_registers \n");
}

static void reset_lcd_module(unsigned char reset)
{
	mt_set_gpio_mode(GPIO_LCM_RST, GPIO_LCM_RST_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_LCM_RST, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_LCM_RST, GPIO_DIR_OUT);

   if(reset){
   	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ONE);	
   }else{
   	mt_set_gpio_out(GPIO_LCM_RST, GPIO_OUT_ZERO);
   }
}
   

static void lcm_init(void)
{
	/*
	/	Power IOVCC > VCI On
	/ 	vgp2 1.8v LDO enable
	/   gpio pin for external LDO enable
	/	delay 1ms
	*/
	//ldo_1v8io_on();
	
	//External LDO 3.0v on
	//ldo_ext_3v0_on();
	//MDELAY(5);
	//ldo_ext_3v0_off();
	//MDELAY(5);
	//ldo_ext_3v0_on();
	/* 
	/ reset pin High > delay 1ms > Low > delay 10ms > High > delay 120ms
	/ LCD RESET PIN 
	*/
	MDELAY(1);
	reset_lcd_module(1);
	MDELAY(1);
	reset_lcd_module(0);
	MDELAY(10);
	reset_lcd_module(1);
	MDELAY(120);

	init_lcm_registers();	
//	MDELAY(20);//xiaopei reduce powerup time 2018-2-5 


	LCM_PRINT("[LCD] lcm_init \n");
}

static struct LCM_setting_table lcm_sleep_in_setting[] = {
    // Display off sequence
	//xiaopei reduce powerup tine 2018-2-5
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 40, {}},

    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
	//xiaopei reduce powerup tine 2018-2-5
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void lcm_suspend(void)
{
	push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);
   // SET_RESET_PIN(1);
   // MDELAY(20 );
    SET_RESET_PIN(0);
    MDELAY(50);
    //SET_RESET_PIN(1);
    //MDELAY(120);
}

static void lcm_resume(void)
{
	lcm_init();
}

static unsigned int lcm_compare_id(void)
{
   //begin xiaopei  2017-12-06 second  LCM supply  ID pin is HIGH  
    SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(50);
   
    mt_set_gpio_mode(GPIO11 | 0x80000000 , GPIO_MODE_00);
	mt_set_gpio_pull_enable(GPIO11, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO11 , GPIO_DIR_IN);
   if( mt_get_gpio_in(GPIO11) ) 
      return 1;
    else 
	  return 0 ;	
  //end
}
// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER ili9806e_dsi_vdo_fwvga_drv_huashi = {
	.name = "ili9806e_dsi_vdo_huashi",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
	//.update = lcm_update,
};
