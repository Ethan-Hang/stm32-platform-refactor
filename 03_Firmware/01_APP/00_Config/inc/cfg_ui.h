/******************************************************************************
 * @file cfg_ui.h
 *
 * @par dependencies
 * - none
 *
 * @author Ethan-Hang
 *
 * @brief Project-level UI behaviour switches.
 *
 *        Kept in 00_Config rather than inside the GUI Guider tree so a UI
 *        re-export cannot silently revert them; the generated layer only
 *        includes this header and reads the macros.
 *
 * @version V1.0 2026-08-23
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __CFG_UI_H__
#define __CFG_UI_H__

/**
 * Screen-transition animations (OVER_* / MOVE_* / FADE_ON).
 *
 * Every one of these animation types redraws BOTH the outgoing and the
 * incoming screen on EVERY animation frame.  On this board one full-screen
 * redraw costs, at minimum:
 *
 *   - 240x280 background read from W25Q64 over SPI2 @ 25 MHz
 *     (LV_IMG_CF_TRUE_COLOR_ALPHA, 3 B/px = 201600 B)      ~64 ms
 *   - 240x284 RGB565 pushed to ST7789 over SPI1 @ 50 MHz   ~22 ms
 *
 * so a nominally 200 ms transition becomes several hundred ms of
 * back-to-back full-screen repaints.  The panel exposes no TE line (the
 * FPC does not route one) and SPI1 is half-duplex with no MISO, so neither
 * TE-interrupt nor GETSCANLINE sync is available and a write can never be
 * phase-locked to the scan.  Every one of those repaints therefore crosses
 * the 16.7 ms scanout period and shows up as tearing.
 *
 * Disabling the animation collapses a transition to ONE repaint instead of
 * several -- the largest tearing reduction available without reworking the
 * asset pipeline.
 *
 * Set to 1 to restore the generated animations.
 */
#ifndef CFG_UI_SCR_ANIM_ENABLE
#define CFG_UI_SCR_ANIM_ENABLE      (0)
#endif

#endif /* __CFG_UI_H__ */
