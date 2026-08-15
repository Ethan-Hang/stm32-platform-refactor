/******************************************************************************
 * @file lv_port_mem_pool.h
 *
 * @par dependencies
 * - stddef.h / stdint.h
 *
 * @author Ethan-Hang
 *
 * @brief Owner of the LVGL heap pool storage, with MPU guard bands.
 *
 *        LVGL's default pool is a bare `static work_mem_int[]` inside
 *        lv_mem.c: it lands wherever the linker puts it, flanked by
 *        unrelated .bss objects.  A write that runs off either end of the
 *        pool therefore corrupts a neighbour silently, and only surfaces
 *        later as an unattributable HardFault.
 *
 *        This module takes ownership of that storage instead, wrapping the
 *        pool in one struct between two 32 B guard bands.  Struct member
 *        order is guaranteed by the language, so the guards are provably
 *        adjacent to the pool without any linker-script cooperation.
 *        mpu_protect_init() then marks both bands MPU_REGION_NO_ACCESS, so
 *        an overrun traps in MemManage_Handler at the instruction that did
 *        it, with the address in SCB->MMFAR.
 *
 *        LVGL reaches this through its LV_MEM_POOL_ALLOC hook (wired in
 *        lv_conf.h), so no LVGL source is patched.
 *
 * Processing flow:
 *   lv_init -> lv_mem_init -> LV_MEM_POOL_ALLOC(LV_MEM_SIZE)
 *                          -> lv_port_mem_pool_alloc -> &s_block.pool
 *
 * @note  Uses raw stdint rather than the platform_type.h `_T` vocabulary:
 *        this header is included from inside LVGL (lv_mem.c) and must stay
 *        free of project-layer dependencies, matching the 04_Impl
 *        convention.
 *
 * @note  The MPU only checks CPU accesses.  A DMA engine writing past the
 *        pool would not be trapped -- no LVGL path does that here (the SPI
 *        TX DMA reads the draw buffers in lv_port_disp.c, not this pool).
 *
 * @version V1.0 2026-08-14
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __LV_PORT_MEM_POOL_H__
#define __LV_PORT_MEM_POOL_H__

//******************************** Includes *********************************//
#include <stddef.h>
#include <stdint.h>
//******************************** Includes *********************************//

#ifdef __cplusplus
extern "C" {
#endif

//******************************** Defines **********************************//
/**
 * Guard band width, in bytes, on each side of the pool.
 *
 * 32 B is the ARMv7-M PMSAv7 minimum region size, and a region's base must
 * be aligned to its own size -- so this value doubles as the alignment of
 * the whole block.  Keeping it at the minimum costs only 64 B of RAM total,
 * which matters at 95% RAM utilisation.
 */
#define LV_PORT_MEM_GUARD_SIZE      32U
//******************************** Defines **********************************//

//******************************* Functions *********************************//
/**
 * @brief      LVGL's LV_MEM_POOL_ALLOC hook: hand LVGL the guarded pool.
 *
 * @param[in]  size : Pool size LVGL asks for; must equal LV_MEM_SIZE.
 *
 * @return     Pool base address, or NULL if @p size disagrees with the
 *             compiled-in pool (which would mean lv_conf.h changed without
 *             this translation unit being rebuilt).
 */
void *lv_port_mem_pool_alloc(size_t size);

/**
 * @brief      Base address of the guard band below the pool.
 * @return     32 B-aligned address, suitable as an MPU region base.
 */
const void *lv_port_mem_pool_guard_lo(void);

/**
 * @brief      Base address of the guard band above the pool.
 * @return     32 B-aligned address, suitable as an MPU region base.
 */
const void *lv_port_mem_pool_guard_hi(void);

/**
 * @brief      First address of the pool itself, for fault classification.
 * @return     Pool base address.
 */
const void *lv_port_mem_pool_base(void);

/**
 * @brief      Pool size in bytes (mirrors LV_MEM_SIZE).
 * @return     Byte count.
 */
uint32_t lv_port_mem_pool_size(void);
//******************************* Functions *********************************//

#ifdef __cplusplus
}
#endif

#endif /* __LV_PORT_MEM_POOL_H__ */
