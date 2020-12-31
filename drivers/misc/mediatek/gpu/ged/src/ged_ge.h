
#ifndef __GED_GE_H__
#define __GED_GE_H__

#include <linux/types.h>

#include <ged_type.h>

#define GE_POOL_ENTRY_SHIFT     (10)
#define GE_POOL_ENTRY_SIZE      (1 << GE_POOL_ENTRY_SHIFT)
#define GE_INVALID_GEHND        0
#define GE_GEHND2IDX(gehnd)     ((gehnd) & (GE_POOL_ENTRY_SIZE - 1))
#define GE_GEHND2VER(gehnd)     ((gehnd) >> GE_POOL_ENTRY_SHIFT)

int ged_ge_init(void);
int ged_ge_exit(void);

int ged_ge_init_context(void **pp_priv);

void ged_ge_context_ref(GED_FILE_PRIVATE_BASE *base, uint32_t ge_hnd);
void ged_ge_context_deref(GED_FILE_PRIVATE_BASE *base, uint32_t ge_hnd);

uint32_t ged_ge_alloc(int region_num, uint32_t *region_sizes);
int32_t ged_ge_retain(uint32_t ge_hnd);
int32_t ged_ge_release(uint32_t ge_hnd);

int ged_ge_get(uint32_t ge_hnd, int region_id, int u32_offset, int u32_size, uint32_t *output_data);
int ged_ge_set(uint32_t ge_hnd, int region_id, int u32_offset, int u32_size, uint32_t *input_data);

#endif
