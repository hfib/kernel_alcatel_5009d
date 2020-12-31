#
# Makefile for all kinds of Fingerprint
#

#obj-$(CONFIG_GOODIX_FINGERPRINT) += goodix/
#obj-$(CONFIG_FPC_FINGERPRINT) += fpc/

#jianggp 20180129 add ifneq --- endif, remove fingerprint for 5009D
ifneq ($(LCT_PRODUCT_MODEL), 5009D) 
ifneq ($(LCT_PRODUCT_MODEL), 5009D_RU) 
$(warning add fingerprint)
obj-$(CONFIG_FINGERPRINT_CDFINGER) += cdfinger/
obj-$(CONFIG_FINGERPRINT_CHIPS) += chips/
endif
endif
