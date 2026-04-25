/******************************************************************************
 * @file
 *
 * @par dependencies
 *
 * @author Ethan-Hang
 *
 * @brief
 *
 * Processing flow:
 *
 *
 * @version V1.0 2026--
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#pragma once
#ifndef __BSP_WRAPPER_DISPLAY_H__
#define __BSP_WRAPPER_DISPLAY_H__

//******************************** Includes *********************************//
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//******************************** Includes *********************************//

//******************************** Defines **********************************//
typedef enum
{
    WP_DISPLAY_OK               = 0,           /* Operation successful       */
    WP_DISPLAY_ERROR            = 1,           /* General error              */
    WP_DISPLAY_ERRORTIMEOUT     = 2,           /* Timeout error              */
    WP_DISPLAY_ERRORRESOURCE    = 3,           /* Resource unavailable       */
    WP_DISPLAY_ERRORPARAMETER   = 4,           /* Invalid parameter          */
    WP_DISPLAY_ERRORNOMEMORY    = 5,           /* Out of memory              */
    WP_DISPLAY_ERRORUNSUPPORTED = 6,           /* Unsupported feature        */
    WP_DISPLAY_ERRORISR         = 7,           /* ISR context error          */
    WP_DISPLAY_RESERVED         = 0xFF,        /* Reserved                   */
} wp_display_status_t;

typedef struct _drv_display_t
{
    uint32_t                       idx;
    uint32_t                    dev_id;
    void *                   user_data;

    void (*pf_display_drv_init  )(struct _drv_display_t *const dev);
    void (*pf_display_drv_deinit)(struct _drv_display_t *const dev);

    wp_display_status_t (*pf_display_fill_color     )(
                                 struct _drv_display_t *const driver_instance, 
                                              uint16_t                  color);
    wp_display_status_t (*pf_display_draw_pixel     )(
                                 struct _drv_display_t *const driver_instance, 
                                              uint16_t                      x,
                                              uint16_t                      y,
                                              uint16_t                  color);
    wp_display_status_t (*pf_display_fill_region    )(
                                 struct _drv_display_t *const driver_instance, 
                                              uint16_t                x_start,
                                              uint16_t                y_start,
                                              uint16_t                  x_end,
                                              uint16_t                  y_end,
                                              uint16_t                  color);
    wp_display_status_t (*pf_display_draw_line      )(
                                 struct _drv_display_t *const driver_instance,
                                              uint16_t                     x0,
                                              uint16_t                     y0,
                                              uint16_t                     x1,
                                              uint16_t                     y1,
                                              uint16_t                  color);
    wp_display_status_t (*pf_display_draw_rectangle )(
                                 struct _drv_display_t *const driver_instance,
                                              uint16_t                     x0,
                                              uint16_t                     y0,
                                              uint16_t                     x1,
                                              uint16_t                     y1,
                                              uint16_t                  color);
    wp_display_status_t (*pf_display_draw_circle    )(
                                 struct _drv_display_t *const driver_instance,
                                              uint16_t               x_center,
                                              uint16_t               y_center,
                                              uint16_t                 radius,
                                              uint16_t                  color);
    wp_display_status_t (*pf_display_draw_image     )(
                                 struct _drv_display_t *const driver_instance,
                                              uint16_t                x_start,
                                              uint16_t                y_start,
                                              uint16_t                      w,
                                              uint16_t                      h,
                                              uint16_t  const*         bitmap);
    wp_display_status_t (*pf_invert_colors         )(
                                 struct _drv_display_t *const driver_instance,
                                               uint8_t                 invert);
    wp_display_status_t (*pf_display_draw_char      )(
                                 struct _drv_display_t *const driver_instance,
                                              uint16_t                      x,
                                              uint16_t                      y,
                                                  char                     ch,
                                              uint16_t                  color, 
                                              uint16_t               bg_color);
    wp_display_status_t (*pf_display_draw_string    )(
                                 struct _drv_display_t *const driver_instance,
                                              uint16_t                      x,
                                              uint16_t                      y,
                                                  char  const*            str, 
                                              uint16_t                  color, 
                                              uint16_t               bg_color);
    wp_display_status_t (*pf_display_draw_filled_rectangle)(
                                 struct _drv_display_t *const driver_instance,
                                              uint16_t                     x0,
                                              uint16_t                     y0,
                                              uint16_t                     x1,
                                              uint16_t                     y1, 
                                              uint16_t                  color);
    wp_display_status_t (*pf_display_draw_filled_triangle )(
                                 struct _drv_display_t *const driver_instance,
                                              uint16_t                     x0,
                                              uint16_t                     y0,
                                              uint16_t                     x1,
                                              uint16_t                     y1, 
                                              uint16_t                     x2, 
                                              uint16_t                     y2, 
                                              uint16_t                  color);
    wp_display_status_t (*pf_display_draw_filled_circle   )(
                                 struct _drv_display_t *const driver_instance,
                                              uint16_t               x_center,
                                              uint16_t               y_center,
                                              uint16_t                 radius,
                                              uint16_t                  color);

    wp_display_status_t (*pf_display_tear_effect          )(
                                 struct _drv_display_t *const driver_instance,
                                               uint8_t                 enable);
} drv_display_t;

//******************************** Defines **********************************//

//******************************* Declaring *********************************//

//******************************* Declaring *********************************//

//******************************* Functions *********************************//
bool drv_adapter_display_mount(uint32_t idx, drv_display_t *const drv);

/**
 * @brief Initialize the currently active display driver.
 *        Forwards to pf_display_drv_init in the registered vtable.
 */
void display_drv_init  (void);

/**
 * @brief Deinitialize the currently active display driver.
 *        Forwards to pf_display_drv_deinit in the registered vtable.
 */
void display_drv_deinit(void);

// external apis
wp_display_status_t display_draw_pixel    (uint16_t x,  uint16_t y,
                                           uint16_t color);
wp_display_status_t display_fill_color    (uint16_t color);
wp_display_status_t display_fill_region   (uint16_t x0, uint16_t y0,
                                           uint16_t x1, uint16_t y1,
                                           uint16_t color);
wp_display_status_t display_draw_line     (uint16_t x0, uint16_t y0,
                                           uint16_t x1, uint16_t y1,
                                           uint16_t color);
wp_display_status_t display_draw_rectangle(uint16_t x0, uint16_t y0,
                                           uint16_t x1, uint16_t y1,
                                           uint16_t color);
wp_display_status_t display_draw_circle   (uint16_t x,  uint16_t y,
                                           uint16_t radius,
                                           uint16_t color);
wp_display_status_t display_draw_image    (uint16_t x0, uint16_t y0,
                                           uint16_t  w, uint16_t  h,
                                           uint16_t const* bitmap);
wp_display_status_t display_invert_colors (bool invert);

wp_display_status_t display_draw_char            (uint16_t x, uint16_t y,
                                                      char c, 
                                                  uint16_t color, 
                                                  uint16_t bg_color);
wp_display_status_t display_draw_string          (uint16_t x, uint16_t y,
                                                const char* str, 
                                                  uint16_t color, 
                                                  uint16_t bg_color);
wp_display_status_t display_draw_filled_rectangle(uint16_t x0, uint16_t y0,
                                                  uint16_t x1, uint16_t y1, 
                                                  uint16_t color);
wp_display_status_t display_draw_filled_triangle (uint16_t x0, uint16_t y0,
                                                  uint16_t x1, uint16_t y1, 
                                                  uint16_t x2, uint16_t y2, 
                                                  uint16_t color);
wp_display_status_t display_draw_filled_circle   (uint16_t x,   uint16_t y,
                                                  uint16_t radius, 
                                                  uint16_t color);

wp_display_status_t display_tear_effect(bool enable);

//******************************* Functions *********************************//

#endif /* __BSP_WRAPPER_DISPLAY_H__ */
