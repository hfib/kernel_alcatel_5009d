
#ifndef _MTK_AFE_UTILITY_H_
#define _MTK_AFE_UTILITY_H_

struct mtk_afe;

int mtk_afe_enable_top_cg(struct mtk_afe *afe, unsigned int cg_type);

int mtk_afe_disable_top_cg(struct mtk_afe *afe, unsigned int cg_type);

int mtk_afe_enable_main_clk(struct mtk_afe *afe);

int mtk_afe_disable_main_clk(struct mtk_afe *afe);

int mtk_afe_emi_clk_on(struct mtk_afe *afe);

int mtk_afe_emi_clk_off(struct mtk_afe *afe);

int mtk_afe_enable_afe_on(struct mtk_afe *afe);

int mtk_afe_disable_afe_on(struct mtk_afe *afe);

#endif
