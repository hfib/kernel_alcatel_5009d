
#include <linux/kernel.h>

#define STA_POWER_DOWN  0
#define STA_POWER_ON    1

extern int spm_mtcmos_cpu_init(void);
extern void spm_mtcmos_cpu_lock(unsigned long *flags);
extern void spm_mtcmos_cpu_unlock(unsigned long *flags);
extern int spm_mtcmos_ctrl_cpu(unsigned int cpu, int state, int chkWfiBeforePdn);
extern int spm_mtcmos_ctrl_cpu0(int state, int chkWfiBeforePdn);
extern int spm_mtcmos_ctrl_cpu1(int state, int chkWfiBeforePdn);
extern int spm_mtcmos_ctrl_cpu2(int state, int chkWfiBeforePdn);
extern int spm_mtcmos_ctrl_cpu3(int state, int chkWfiBeforePdn);
extern int spm_mtcmos_ctrl_cpu4(int state, int chkWfiBeforePdn);
extern int spm_mtcmos_ctrl_cpu5(int state, int chkWfiBeforePdn);
extern int spm_mtcmos_ctrl_cpu6(int state, int chkWfiBeforePdn);
extern int spm_mtcmos_ctrl_cpu7(int state, int chkWfiBeforePdn);
extern int spm_mtcmos_ctrl_dbg0(int state);
extern int spm_mtcmos_ctrl_cpusys0(int state, int chkWfiBeforePdn);
extern bool spm_cpusys0_can_power_down(void);



extern int spm_mtcmos_ctrl_isp(int state);
extern int spm_mtcmos_ctrl_disp(int state);
extern int spm_mtcmos_ctrl_mfg(int state);
extern int spm_mtcmos_ctrl_mdsys1(int state);
extern int spm_mtcmos_ctrl_connsys(int state);
