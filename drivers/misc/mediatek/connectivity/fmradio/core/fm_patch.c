
#include <linux/fs.h>
#include <linux/firmware.h>
#include <asm/uaccess.h>

#include "fm_typedef.h"
#include "fm_dbg.h"
#include "fm_err.h"

fm_s32 fm_file_read(const fm_s8 *filename, fm_u8 *dst, fm_s32 len, fm_s32 position)
{
	const struct firmware *fw = NULL;
	fm_s32 ret = 0;

	ret = request_firmware(&fw, filename, NULL);
	if (ret) {
		WCN_DBG(FM_ERR | CHIP, "Failed to load firmware \"%s\"\n", filename);
		return -FM_EPATCH;
	} else {
		WCN_DBG(FM_NTC | CHIP, "load firmware \"%s\" ok\n", filename);
	}

	if (len >= fw->size) {
		memcpy(dst, fw->data, fw->size);
		WCN_DBG(FM_NTC | CHIP, "Copy file data(%p) size(%zu)\n", fw->data, fw->size);
		ret = fw->size;
	} else {
		WCN_DBG(FM_NTC | CHIP, "Copy file data failed fw->size(%zu) > bufsize(%d)\n", fw->size, len);
		ret = -FM_EPATCH;
	}
	release_firmware(fw);
	return ret;
}

fm_s32 fm_file_write(const fm_s8 *filename, fm_u8 *dst, fm_s32 len, fm_s32 *ppos)
{
	fm_s32 ret = 0;
	return ret;
}
