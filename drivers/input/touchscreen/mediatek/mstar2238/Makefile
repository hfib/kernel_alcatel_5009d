# include $(srctree)/drivers/misc/mediatek/Makefile.custom

# Linux driver folder
#ccflags-y += -I$(srctree)/drivers/misc/mediatek/mach/$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)/touchpanel/mstar2238/
#ccflags-y += -I$(srctree)/arch/arm/mach-$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)/touchpanel/mstar2238/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/mstar2238/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hwmon/include

obj-y += ilitek_drv_mtk.o
obj-y += ilitek_drv_iliproc.o
obj-y += ilitek_drv_main.o
obj-y += ilitek_drv_mp_test.o
obj-y += ilitek_drv_update.o
