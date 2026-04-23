/******************************************************************************
 * @file    bsp_st7789_driver.c
 *
 * @par     dependencies
 * - bsp_st7789_driver.h
 * - Debug.h
 *
 * @author  Ethan-Hang
 *
 * @brief   ST7789 TFT-LCD controller driver implementation.
 *
 * Processing flow:
 * - Bind external interfaces via instantiation helper.
 * - Drive hardware reset, issue init command sequence, program addr window.
 * - Expose basic pixel / region / graphic / text primitives via vtable.
 *
 * @version V1.0 2026-4-23
 *
 * @note    1 tab == 4 spaces!
 *
 *****************************************************************************/

//******************************** Includes *********************************//
#include "bsp_st7789_driver.h"
#include "Debug.h"

//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define ST7789_IS_INITED                               true
#define ST7789_NOT_INITED                             false

/**
 * Sanity upper bound for the active window.  ST7789 controllers address a
 * 240 x 320 GRAM at most, so any panel larger than that is almost certainly
 * a caller mistake rather than a legitimate configuration.
 */
#define ST7789_MAX_PANEL_WIDTH                          240U
#define ST7789_MAX_PANEL_HEIGHT                         320U

//******************************** Defines **********************************//

//******************************* Declaring *********************************//

//******************************* Declaring *********************************//

//******************************* Functions *********************************//

/**
 * @brief  Run power-on reset and ST7789 init command sequence.
 *
 * @param[in] driver_instance : Driver object already populated by
 *                              bsp_st7789_driver_init().
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_init(bsp_st7789_driver_t *const driver_instance)
{
    /* TODO: implement hardware reset timing + command sequence. */
    (void)driver_instance;
    return ST7789_OK;
}

/**
 * @brief  Release hardware resources and mark the instance uninitialized.
 *
 * @param[in] driver_instance : Driver object.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_deinit(bsp_st7789_driver_t *const driver_instance)
{
    /* TODO: DISPOFF + SLPIN + SPI deinit. */
    (void)driver_instance;
    return ST7789_OK;
}

/**
 * @brief  Fill the entire active panel area with a single RGB565 color.
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] color           : RGB565 pixel value.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_fill_color(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t                  color)
{
    /* TODO: program full-screen addr window, stream color via DMA. */
    (void)driver_instance;
    (void)color;
    return ST7789_OK;
}

/**
 * @brief  Draw a single pixel at (x, y).
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] x               : Column in panel coordinates.
 * @param[in] y               : Row    in panel coordinates.
 * @param[in] color           : RGB565 pixel value.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_draw_pixel(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t                      x,
                                               uint16_t                      y,
                                               uint16_t                  color)
{
    /* TODO: set 1x1 addr window, write 2-byte color. */
    (void)driver_instance;
    (void)x;
    (void)y;
    (void)color;
    return ST7789_OK;
}

/**
 * @brief  Fill an inclusive rectangular region with a single color.
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] x_start, y_start: Top-left corner (inclusive).
 * @param[in] x_end,   y_end  : Bottom-right corner (inclusive).
 * @param[in] color           : RGB565 pixel value.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_fill_region(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t                x_start,
                                               uint16_t                y_start,
                                               uint16_t                  x_end,
                                               uint16_t                  y_end,
                                               uint16_t                  color)
{
    /* TODO: clamp to panel, set addr window, stream color. */
    (void)driver_instance;
    (void)x_start;
    (void)y_start;
    (void)x_end;
    (void)y_end;
    (void)color;
    return ST7789_OK;
}

/**
 * @brief  Draw a 2x2 pixel block at (x, y) for thick pixel / low-res UI.
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] x, y            : Top-left pixel of the 2x2 block.
 * @param[in] color           : RGB565 pixel value.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_draw_pixel_4px(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t                      x,
                                               uint16_t                      y,
                                               uint16_t                  color)
{
    /* TODO: delegate to st7789_fill_region(x, y, x+1, y+1, color). */
    (void)driver_instance;
    (void)x;
    (void)y;
    (void)color;
    return ST7789_OK;
}

/**
 * @brief  Program CASET / RASET so subsequent RAMWR writes land in the
 *         given inclusive rectangle.
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] x_start, y_start: Top-left corner (inclusive).
 * @param[in] x_end,   y_end  : Bottom-right corner (inclusive).
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_set_addr_window(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t                x_start,
                                               uint16_t                y_start,
                                               uint16_t                  x_end,
                                               uint16_t                  y_end)
{
    /* TODO: apply panel.x_offset / y_offset, CASET + RASET + RAMWR. */
    (void)driver_instance;
    (void)x_start;
    (void)y_start;
    (void)x_end;
    (void)y_end;
    return ST7789_OK;
}

/**
 * @brief  Draw a straight line between two endpoints.
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] x0, y0          : First endpoint.
 * @param[in] x1, y1          : Second endpoint.
 * @param[in] color           : RGB565 pixel value.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_draw_line(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t                     x0,
                                               uint16_t                     y0,
                                               uint16_t                     x1,
                                               uint16_t                     y1,
                                               uint16_t                  color)
{
    /* TODO: Bresenham line algorithm. */
    (void)driver_instance;
    (void)x0;
    (void)y0;
    (void)x1;
    (void)y1;
    (void)color;
    return ST7789_OK;
}

/**
 * @brief  Draw a hollow rectangle outline.
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] x0, y0          : Top-left corner.
 * @param[in] x1, y1          : Bottom-right corner.
 * @param[in] color           : RGB565 pixel value.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_draw_rectangle(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t                     x0,
                                               uint16_t                     y0,
                                               uint16_t                     x1,
                                               uint16_t                     y1,
                                               uint16_t                  color)
{
    /* TODO: 4x draw_line or 2x horizontal + 2x vertical fill_region. */
    (void)driver_instance;
    (void)x0;
    (void)y0;
    (void)x1;
    (void)y1;
    (void)color;
    return ST7789_OK;
}

/**
 * @brief  Draw a hollow circle centered at (x_center, y_center).
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] x_center, y_center: Circle center.
 * @param[in] radius          : Radius in pixels.
 * @param[in] color           : RGB565 pixel value.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_draw_circle(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t               x_center,
                                               uint16_t               y_center,
                                               uint16_t                 radius,
                                               uint16_t                  color)
{
    /* TODO: midpoint (Bresenham) circle algorithm. */
    (void)driver_instance;
    (void)x_center;
    (void)y_center;
    (void)radius;
    (void)color;
    return ST7789_OK;
}

/**
 * @brief  Blit a caller-supplied RGB565 bitmap to the given region.
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] x_start, y_start: Top-left paste coordinate.
 * @param[in] w, h            : Bitmap width / height in pixels.
 * @param[in] bitmap          : Pointer to w*h RGB565 pixels (row-major).
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_draw_image(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t                x_start,
                                               uint16_t                y_start,
                                               uint16_t                      w,
                                               uint16_t                      h,
                                               uint16_t  const*         bitmap)
{
    /* TODO: set addr window (w*h), stream bitmap via DMA. */
    (void)driver_instance;
    (void)x_start;
    (void)y_start;
    (void)w;
    (void)h;
    (void)bitmap;
    return ST7789_OK;
}

/**
 * @brief  Enable / disable display color inversion (INVON / INVOFF).
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] invert          : Non-zero enables inversion.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t invert_colors(
                                    bsp_st7789_driver_t *const driver_instance,
                                                uint8_t                 invert)
{
    /* TODO: write_command(INVON / INVOFF). */
    (void)driver_instance;
    (void)invert;
    return ST7789_OK;
}

/**
 * @brief  Draw a single ASCII character using the embedded glyph table.
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] x, y            : Top-left glyph cell.
 * @param[in] ch              : ASCII code point.
 * @param[in] color, bg_color : Foreground / background RGB565.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_draw_char(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t                      x,
                                               uint16_t                      y,
                                                   char                     ch,
                                               uint16_t                  color,
                                               uint16_t               bg_color)
{
    /* TODO: resolve glyph, stream pixel rows into fill_region. */
    (void)driver_instance;
    (void)x;
    (void)y;
    (void)ch;
    (void)color;
    (void)bg_color;
    return ST7789_OK;
}

/**
 * @brief  Draw a null-terminated ASCII string.
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] x, y            : Start coordinate of the first glyph.
 * @param[in] str             : NUL-terminated ASCII string.
 * @param[in] color, bg_color : Foreground / background RGB565.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_draw_string(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t                      x,
                                               uint16_t                      y,
                                                   char  const*            str,
                                               uint16_t                  color,
                                               uint16_t               bg_color)
{
    /* TODO: iterate str, advance cursor by glyph width per character. */
    (void)driver_instance;
    (void)x;
    (void)y;
    (void)str;
    (void)color;
    (void)bg_color;
    return ST7789_OK;
}

/**
 * @brief  Draw a solid-filled rectangle.
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] x0, y0, x1, y1  : Inclusive corner coordinates.
 * @param[in] color           : RGB565 pixel value.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_draw_filled_rectangle(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t                     x0,
                                               uint16_t                     y0,
                                               uint16_t                     x1,
                                               uint16_t                     y1,
                                               uint16_t                  color)
{
    /* TODO: delegate to st7789_fill_region. */
    (void)driver_instance;
    (void)x0;
    (void)y0;
    (void)x1;
    (void)y1;
    (void)color;
    return ST7789_OK;
}

/**
 * @brief  Draw a solid-filled triangle.
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] x0..y2          : Three vertices.
 * @param[in] color           : RGB565 pixel value.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_draw_filled_triangle(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t                     x0,
                                               uint16_t                     y0,
                                               uint16_t                     x1,
                                               uint16_t                     y1,
                                               uint16_t                     x2,
                                               uint16_t                     y2,
                                               uint16_t                  color)
{
    /* TODO: scanline / edge-function rasterization. */
    (void)driver_instance;
    (void)x0;
    (void)y0;
    (void)x1;
    (void)y1;
    (void)x2;
    (void)y2;
    (void)color;
    return ST7789_OK;
}

/**
 * @brief  Draw a solid-filled circle.
 *
 * @param[in] driver_instance    : Driver object.
 * @param[in] x_center, y_center : Circle center.
 * @param[in] radius             : Radius in pixels.
 * @param[in] color              : RGB565 pixel value.
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_draw_filled_circle(
                                    bsp_st7789_driver_t *const driver_instance,
                                               uint16_t               x_center,
                                               uint16_t               y_center,
                                               uint16_t                 radius,
                                               uint16_t                  color)
{
    /* TODO: midpoint circle + horizontal fill per scanline. */
    (void)driver_instance;
    (void)x_center;
    (void)y_center;
    (void)radius;
    (void)color;
    return ST7789_OK;
}

/**
 * @brief  Enable / disable tearing-effect (TE) output on the TE pin.
 *
 * @param[in] driver_instance : Driver object.
 * @param[in] enable          : Non-zero enables TE (TEON), zero disables (TEOFF).
 *
 * @return ST7789_OK on success, error code otherwise.
 * */
static st7789_status_t st7789_tear_effect(
                                    bsp_st7789_driver_t *const driver_instance,
                                                uint8_t                 enable)
{
    /* TODO: write_command(TEON / TEOFF), default V-blank only. */
    (void)driver_instance;
    (void)enable;
    return ST7789_OK;
}

/**
 * @brief  Instantiate a ST7789 driver object: validate caller-supplied
 *         interfaces and panel geometry, bind them into the driver instance,
 *         and mount the public API vtable.  Does NOT touch the hardware -
 *         the caller must invoke pf_st7789_init() afterwards to run the
 *         power-on reset and register-configuration sequence.
 *
 * @param[out] driver_instance         Driver object to populate.
 * @param[in]  p_spi_interface         Raw SPI / CS / DC / RST / DMA vtable.
 * @param[in]  p_spi_driver_interface  ST7789 framing vtable
 *                                     (write_cmd / write_data wrappers).
 * @param[in]  p_timebase_interface    ms-tick / busy-wait delay vtable.
 * @param[in]  p_os_interface          OS-aware delay vtable.
 * @param[in]  p_panel                 Panel geometry (width/height/offsets).
 *
 * @return  ST7789_OK                  - Success.
 *          ST7789_ERRORPARAMETER      - NULL pointer or out-of-range geometry.
 *          ST7789_ERRORRESOURCE       - Instance already initialized, or a
 *                                       required vtable slot is NULL.
 * */
st7789_status_t bsp_st7789_driver_inst(
                                   bsp_st7789_driver_t * const driver_instance,
                                st7789_spi_interface_t *       p_spi_interface,
                         st7789_spi_driver_interface_t *p_spi_driver_interface,
                           st7789_timebase_interface_t *  p_timebase_interface,
                                 st7789_os_interface_t *        p_os_interface,
                           const st7789_panel_config_t *               p_panel
                                        )
{
    DEBUG_OUT(i, ST7789_LOG_TAG, "bsp_st7789_driver_inst start");

    st7789_status_t ret = ST7789_OK;

    /************ 1.Checking input parameters ************/
    /**
     * All top-level pointers are mandatory.  p_os_interface is required as
     * well because the project always links against FreeRTOS; a bare-metal
     * variant would gate it with OS_SUPPORTING, which this driver does not
     * expose.
     **/
    if (NULL == driver_instance                                 ||
        NULL == p_spi_interface                                 ||
        NULL == p_spi_driver_interface                          ||
        NULL == p_timebase_interface                            ||
        NULL == p_os_interface                                  ||
        NULL == p_panel)
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG,
                  "bsp_st7789_driver_inst input error parameter");
        ret = ST7789_ERRORPARAMETER;
        return ret;
    }

    /**
     * Panel geometry sanity check.  width/height == 0 would make every draw
     * call a no-op and any fill_region compute wrong addr windows, so treat
     * it as a caller error rather than silently accepting it.  Upper bound
     * follows ST7789 GRAM limits.
     **/
    if ((0U == p_panel->width)                                  ||
        (0U == p_panel->height)                                 ||
        (p_panel->width  > ST7789_MAX_PANEL_WIDTH)              ||
        (p_panel->height > ST7789_MAX_PANEL_HEIGHT))
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG,
                  "bsp_st7789_driver_inst panel geometry out of range");
        ret = ST7789_ERRORPARAMETER;
        return ret;
    }

    /************* 2.Checking the Resources **************/
    /**
     * Reject re-instantiation of the same driver object: re-entering this
     * helper after a successful bind would overwrite bookkeeping and could
     * double-init the underlying SPI bus.
     **/
    if (ST7789_IS_INITED == driver_instance->is_init)
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG,
                  "bsp_st7789_driver_inst instance already initialized");
        ret = ST7789_ERRORRESOURCE;
        return ret;
    }

    /**
     * Verify every vtable slot the driver will actually call.  Do this on
     * the caller-supplied pointers directly, BEFORE mounting them onto the
     * driver instance, so a partial bind never leaks into the instance on
     * failure.
     **/
    if (NULL == p_spi_interface->pf_spi_init                    ||
        NULL == p_spi_interface->pf_spi_deinit                  ||
        NULL == p_spi_interface->pf_spi_transmit                ||
        NULL == p_spi_interface->pf_spi_transmit_dma            ||
        NULL == p_spi_interface->pf_spi_wait_dma_complete       ||
        NULL == p_spi_interface->pf_spi_write_cs_pin            ||
        NULL == p_spi_interface->pf_spi_write_dc_pin            ||
        NULL == p_spi_interface->pf_spi_write_rst_pin)
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG,
                  "st7789 spi_interface has NULL callback");
        ret = ST7789_ERRORRESOURCE;
        return ret;
    }

    if (NULL == p_spi_driver_interface->pf_st7789_write_data        ||
        NULL == p_spi_driver_interface->pf_st7789_write_single_data ||
        NULL == p_spi_driver_interface->pf_st7789_write_command)
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG,
                  "st7789 spi_driver_interface has NULL callback");
        ret = ST7789_ERRORRESOURCE;
        return ret;
    }

    if (NULL == p_timebase_interface->pf_get_tick_ms            ||
        NULL == p_timebase_interface->pf_delay_ms)
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG,
                  "st7789 timebase_interface has NULL callback");
        ret = ST7789_ERRORRESOURCE;
        return ret;
    }

    if (NULL == p_os_interface->pf_os_delay_ms)
    {
        DEBUG_OUT(e, ST7789_ERR_LOG_TAG,
                  "st7789 os_interface has NULL callback");
        ret = ST7789_ERRORRESOURCE;
        return ret;
    }

    /************** 3.Mount the Interfaces ***************/
    /**
     * 3.1 Mount external interfaces.
     * Bind all caller-supplied vtables and geometry onto the driver
     * instance so subsequent API calls can dereference them via
     * driver_instance->p_xxx and driver_instance->panel.
     **/
    driver_instance->p_spi_interface         =        p_spi_interface;
    driver_instance->p_spi_driver_interface  = p_spi_driver_interface;
    driver_instance->p_timebase_interface    =   p_timebase_interface;
    driver_instance->p_os_interface          =         p_os_interface;
    driver_instance->panel                   =               *p_panel;

    /**
     * 3.2 Mount internal API vtable.
     * Each slot points to a file-local stub; call-sites dispatch via
     * driver_instance->pf_xxx(driver_instance, ...).  Stubs currently
     * only acknowledge the call and return ST7789_OK; real command
     * sequences will fill them in incrementally.
     **/
    driver_instance->pf_st7789_init                  =             st7789_init;
    driver_instance->pf_st7789_deinit                =           st7789_deinit;
    driver_instance->pf_st7789_fill_color            =       st7789_fill_color;
    driver_instance->pf_st7789_draw_pixel            =       st7789_draw_pixel;
    driver_instance->pf_st7789_fill_region           =      st7789_fill_region;
    driver_instance->pf_st7789_draw_pixel_4px        =   st7789_draw_pixel_4px;
    driver_instance->pf_st7789_set_addr_window       =  st7789_set_addr_window;
    driver_instance->pf_st7789_draw_line             =        st7789_draw_line;
    driver_instance->pf_st7789_draw_rectangle        =   st7789_draw_rectangle;
    driver_instance->pf_st7789_draw_circle           =      st7789_draw_circle;
    driver_instance->pf_st7789_draw_image            =       st7789_draw_image;
    driver_instance->pf_invert_colors                =           invert_colors;
    driver_instance->pf_st7789_draw_char             =        st7789_draw_char;
    driver_instance->pf_st7789_draw_string           =      st7789_draw_string;
    driver_instance->pf_st7789_draw_filled_rectangle =
                                                  st7789_draw_filled_rectangle;
    driver_instance->pf_st7789_draw_filled_triangle  =
                                                   st7789_draw_filled_triangle;
    driver_instance->pf_st7789_draw_filled_circle    =
                                                     st7789_draw_filled_circle;
    driver_instance->pf_st7789_tear_effect           =      st7789_tear_effect;

    DEBUG_OUT(d, ST7789_LOG_TAG, "bsp_st7789_driver_inst success");
    return ret;
}

//******************************* Functions *********************************//
