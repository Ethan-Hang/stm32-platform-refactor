/******************************************************************************
 * @file lv_port_mem_pool.c
 *
 * @par dependencies
 * - lv_port_mem_pool.h
 * - lvgl.h (for LV_MEM_SIZE)
 *
 * @author Ethan-Hang
 *
 * @brief Guarded storage for the LVGL heap pool.  See the header for the
 *        rationale; this file is just the storage definition plus accessors.
 *
 * @version V1.0 2026-08-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "lv_port_mem_pool.h"
#include "lvgl.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
/*
 * The three members must be laid out guard_lo | pool | guard_hi with no
 * padding between them.  All are uint8_t arrays, so the compiler inserts
 * none, and C guarantees increasing member addresses in declaration order.
 * Aligning the struct to LV_PORT_MEM_GUARD_SIZE puts guard_lo on a 32 B
 * boundary; guard_hi then lands on one too as long as the pool size is a
 * multiple of 32, which the static assert below enforces.
 */
typedef struct
{
    uint8_t guard_lo[LV_PORT_MEM_GUARD_SIZE];
    uint8_t pool[LV_MEM_SIZE];
    uint8_t guard_hi[LV_PORT_MEM_GUARD_SIZE];
} lv_port_mem_block_t;

/* ARMv7-M PMSAv7: a region's base address must be aligned to its size. */
_Static_assert((LV_MEM_SIZE % LV_PORT_MEM_GUARD_SIZE) == 0U,
               "LV_MEM_SIZE must be a multiple of the guard size, otherwise "
               "guard_hi is not aligned for an MPU region base");
//******************************** Defines **********************************//

//******************************* Declaring *********************************//
/*
 * Not static const -- LVGL writes the pool, and the guards are never
 * touched by design (the MPU makes any touch a MemManage fault).  Lives in
 * .bss, so startup zero-fills it; the MPU must therefore be armed after
 * that fill, which mpu_protect_init() from main() satisfies.
 */
static lv_port_mem_block_t s_block
    __attribute__((aligned(LV_PORT_MEM_GUARD_SIZE)));
//******************************* Declaring *********************************//

//******************************* Functions *********************************//
void *lv_port_mem_pool_alloc(size_t size)
{
    /*
     * LVGL passes LV_MEM_SIZE.  A mismatch means lv_conf.h moved without
     * this file being recompiled -- returning NULL makes lv_tlsf fail
     * loudly at init instead of handing LVGL a wrong-sized pool.
     */
    if (sizeof(s_block.pool) != size)
    {
        return NULL;
    }
    return s_block.pool;
}

const void *lv_port_mem_pool_guard_lo(void)
{
    return s_block.guard_lo;
}

const void *lv_port_mem_pool_guard_hi(void)
{
    return s_block.guard_hi;
}

const void *lv_port_mem_pool_base(void)
{
    return s_block.pool;
}

uint32_t lv_port_mem_pool_size(void)
{
    return (uint32_t)sizeof(s_block.pool);
}
//******************************* Functions *********************************//
