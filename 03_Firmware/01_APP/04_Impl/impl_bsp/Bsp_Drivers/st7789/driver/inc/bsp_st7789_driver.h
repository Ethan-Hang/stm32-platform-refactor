/******************************************************************************
 * @file bsp_st7789_driver.h
 *
 * @par dependencies
 * - stdint.h
 * - stdbool.h
 *
 * @author Ethan-Hang
 *
 * @brief ST7789 TFT-LCD controller driver interface definitions.
 *
 * Defines the SPI, timebase, and OS interface structures that must be
 * provided by the integration layer, together with the driver instance
 * struct and public API surface.
 *
 * Processing flow:
 *   1. Integration layer populates st7789_driver_input_arg_t.
 *   2. bsp_st7789_driver_inst() binds interfaces and initializes HW.
 *   3. Application draws pixels/shapes/text via vtable function pointers.
 *
 * @version V1.0 2026-04-23
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __BSP_ST7789_DRIVER_H__
#define __BSP_ST7789_DRIVER_H__

//******************************** Includes *********************************//
#include "board_types.h"

#include "Debug.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
typedef struct bsp_st7789_driver bsp_st7789_driver_t;

typedef enum
{
    ST7789_OK               = 0,       /* Operation successful               */
    ST7789_ERROR            = 1,       /* General error                      */
    ST7789_ERRORTIMEOUT     = 2,       /* Timeout error                      */
    ST7789_ERRORRESOURCE    = 3,       /* Resource unavailable               */
    ST7789_ERRORPARAMETER   = 4,       /* Invalid parameter                  */
    ST7789_ERRORNOMEMORY    = 5,       /* Out of memory                      */
    ST7789_ERRORUNSUPPORTED = 6,       /* Unsupported feature                */
    ST7789_ERRORISR         = 7,       /* ISR context error                  */
    ST7789_RESERVED         = 0xFF,    /* ST7789 Reserved                    */
} st7789_status_t;

/**
 * Completion callback for pf_st7789_flush_async.  Fires from the SPI TX-DMA
 * complete interrupt: ISR-safe work only (flag setters, *_from_isr OSAL).
 **/
typedef void (*st7789_flush_done_cb_t)(void *arg);

typedef struct
{
    st7789_status_t (*pf_spi_init             )(void);
    st7789_status_t (*pf_spi_deinit           )(void);
    st7789_status_t (*pf_spi_transmit         )( UINT8_t const *p_data, 
                                                UINT32_t   data_length);
    st7789_status_t (*pf_spi_transmit_dma     )( UINT8_t const *p_data, 
                                                UINT32_t   data_length);
    st7789_status_t (*pf_spi_wait_dma_complete)(UINT32_t    timeout_ms);
    st7789_status_t (*pf_spi_write_cs_pin     )( UINT8_t         state);
    st7789_status_t (*pf_spi_write_dc_pin     )( UINT8_t         state);
    st7789_status_t (*pf_spi_write_rst_pin    )( UINT8_t         state);
} st7789_spi_interface_t;

typedef struct 
{
    void     (*pf_delay_ms   )(UINT32_t ms);
} st7789_timebase_interface_t;

typedef struct
{
    UINT16_t    width;        /*  Active width  in current orientation (px)  */
    UINT16_t   height;        /*  Active height in current orientation (px)  */
    UINT16_t x_offset;        /*  CASET offset for the specific panel        */
    UINT16_t y_offset;        /*  RASET offset for the specific panel        */
} st7789_panel_config_t;

typedef struct
{
    st7789_panel_config_t        panel;
    st7789_spi_interface_t      *p_spi_interface;
    st7789_timebase_interface_t *p_timebase_interface;
} st7789_driver_input_arg_t;

struct bsp_st7789_driver
{
    /************* Target of Internal Status *************/
    BOOL                       is_init;

    /************ Panel geometry / orientation ***********/
    st7789_panel_config_t                            panel;
    st7789_spi_interface_t        *        p_spi_interface;
    st7789_timebase_interface_t   *   p_timebase_interface;

    // basic funcionts
    st7789_status_t (*pf_st7789_init           )(
                                   bsp_st7789_driver_t *const driver_instance);
    st7789_status_t (*pf_st7789_deinit         )(
                                   bsp_st7789_driver_t *const driver_instance);
    st7789_status_t (*pf_st7789_fill_color     )(
                                   bsp_st7789_driver_t *const driver_instance, 
                                              UINT16_t                  color);
    st7789_status_t (*pf_st7789_draw_pixel     )(
                                   bsp_st7789_driver_t *const driver_instance, 
                                              UINT16_t                      x,
                                              UINT16_t                      y,
                                              UINT16_t                  color);
    st7789_status_t (*pf_st7789_fill_region    )(
                                   bsp_st7789_driver_t *const driver_instance, 
                                              UINT16_t                x_start,
                                              UINT16_t                y_start,
                                              UINT16_t                  x_end,
                                              UINT16_t                  y_end,
                                              UINT16_t                  color);
    st7789_status_t (*pf_st7789_draw_pixel_4px )(
                                   bsp_st7789_driver_t *const driver_instance,
                                              UINT16_t                      x,
                                              UINT16_t                      y,
                                              UINT16_t                  color);

    // graphic functions
    st7789_status_t (*pf_st7789_draw_line      )(
                                   bsp_st7789_driver_t *const driver_instance,
                                              UINT16_t                     x0,
                                              UINT16_t                     y0,
                                              UINT16_t                     x1,
                                              UINT16_t                     y1,
                                              UINT16_t                  color);
    st7789_status_t (*pf_st7789_draw_rectangle )(
                                   bsp_st7789_driver_t *const driver_instance,
                                              UINT16_t                     x0,
                                              UINT16_t                     y0,
                                              UINT16_t                     x1,
                                              UINT16_t                     y1,
                                              UINT16_t                  color);
    st7789_status_t (*pf_st7789_draw_circle    )(
                                   bsp_st7789_driver_t *const driver_instance,
                                              UINT16_t               x_center,
                                              UINT16_t               y_center,
                                              UINT16_t                 radius,
                                              UINT16_t                  color);
    st7789_status_t (*pf_st7789_draw_image     )(
                                   bsp_st7789_driver_t *const driver_instance,
                                              UINT16_t                x_start,
                                              UINT16_t                y_start,
                                              UINT16_t                      w,
                                              UINT16_t                      h,
                                              UINT16_t  const*         bitmap);
    /* Non-blocking blit: single DMA straight from `bitmap`, returns after
     * dispatch.  `bitmap` must be in PANEL byte order (big-endian RGB565,
     * e.g. LV_COLOR_16_SWAP=1 output) and stay stable until `done_cb`
     * fires from the DMA-complete ISR.  Single in-flight transfer; one
     * producer task only (LVGL).  Max w*h*2 = 65535 bytes per call. */
    st7789_status_t (*pf_st7789_flush_async    )(
                                   bsp_st7789_driver_t *const driver_instance,
                                              UINT16_t                x_start,
                                              UINT16_t                y_start,
                                              UINT16_t                      w,
                                              UINT16_t                      h,
                                              UINT16_t  const*         bitmap,
                                st7789_flush_done_cb_t           done_cb,
                                                  void *      done_cb_arg);
    st7789_status_t (*pf_invert_colors         )(
                                   bsp_st7789_driver_t *const driver_instance,
                                               UINT8_t                 invert);

    // text functions
    st7789_status_t (*pf_st7789_draw_char      )(
                                   bsp_st7789_driver_t *const driver_instance,
                                              UINT16_t                      x,
                                              UINT16_t                      y,
                                                  char                     ch,
                                              UINT16_t                  color, 
                                              UINT16_t               bg_color);
    st7789_status_t (*pf_st7789_draw_string    )(
                                   bsp_st7789_driver_t *const driver_instance,
                                              UINT16_t                      x,
                                              UINT16_t                      y,
                                                  char  const*            str, 
                                              UINT16_t                  color, 
                                              UINT16_t               bg_color);

    // extended functions
    st7789_status_t (*pf_st7789_draw_filled_rectangle)(
                                   bsp_st7789_driver_t *const driver_instance,
                                              UINT16_t                     x0,
                                              UINT16_t                     y0,
                                              UINT16_t                     x1,
                                              UINT16_t                     y1, 
                                              UINT16_t                  color);
    st7789_status_t (*pf_st7789_draw_filled_triangle )(
                                   bsp_st7789_driver_t *const driver_instance,
                                              UINT16_t                     x0,
                                              UINT16_t                     y0,
                                              UINT16_t                     x1,
                                              UINT16_t                     y1, 
                                              UINT16_t                     x2, 
                                              UINT16_t                     y2, 
                                              UINT16_t                  color);
    st7789_status_t (*pf_st7789_draw_filled_circle   )(
                                   bsp_st7789_driver_t *const driver_instance,
                                              UINT16_t               x_center,
                                              UINT16_t               y_center,
                                              UINT16_t                 radius,
                                              UINT16_t                  color);

    st7789_status_t (*pf_st7789_tear_effect          )(
                                   bsp_st7789_driver_t *const driver_instance,
                                               UINT8_t                 enable);
};

//******************************** Defines **********************************//

//******************************* Declaring *********************************//

//******************************* Declaring *********************************//

//******************************* Functions *********************************//
/**
 * @brief  Assemble a st7789 driver instance with caller-supplied interfaces
 *         and panel geometry.
 *
 * @param[out] driver_instance       Driver instance to populate.
 * @param[in]  p_spi_interface       Raw SPI / CS / DC / RST / DMA-wait vtable.
 * @param[in]  p_spi_driver_interface  ST7789 SPI framing vtable
 *                                     (write_cmd / write_data wrappers).
 * @param[in]  p_timebase_interface  ms tick / busy-wait delay vtable.
 * @param[in]  p_panel               Panel geometry config (width/height/offsets).
 *
 * @return ST7789_OK on success, error code otherwise.
 */
st7789_status_t bsp_st7789_driver_inst(
                                   bsp_st7789_driver_t * const driver_instance,
                                st7789_spi_interface_t *       p_spi_interface,
                           st7789_timebase_interface_t *  p_timebase_interface,
                                 st7789_panel_config_t const *         p_panel
                                        );

/**
 * @brief  ISR-side completion handler for pf_st7789_flush_async.
 *
 *         The integration layer must call this from its SPI TX-DMA-complete
 *         hook (HAL_SPI_TxCpltCallback path).  Releases the panel CS and
 *         invokes the caller's done_cb.  No-op when no async flush is armed
 *         (e.g. the TC interrupt belongs to a synchronous DMA write).
 *
 * @return None.
 * */
void bsp_st7789_driver_async_txcplt_isr(void);
//******************************* Functions *********************************//

#endif /* __BSP_ST7789_DRIVER_H__ */
