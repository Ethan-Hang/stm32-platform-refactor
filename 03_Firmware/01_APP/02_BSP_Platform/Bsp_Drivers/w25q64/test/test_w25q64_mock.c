/******************************************************************************
 * @file    test_w25q64_mock.c
 *
 * @par dependencies
 * - bsp_w25q64_driver.h
 * - bsp_w25q64_reg.h
 * - mock/Debug.h
 *
 * @author  Ethan-Hang
 *
 * @brief   Host-runnable mock test suite for the W25Q64 SPI NOR driver.
 *          Drives the driver through its injected SPI vtable using a
 *          fake on-host W25Q64 simulator. No target hardware needed.
 *
 * Build & run (host gcc):
 *     cd 02_BSP_Platform/Bsp_Drivers/w25q64/test
 *     make && ./test_w25q64_mock
 *
 * @version V1.0 2026-04-27
 *
 * @note    1 tab == 4 spaces!
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "bsp_w25q64_driver.h"
#include "bsp_w25q64_reg.h"

/******************************************************************************
 *                          Tiny test framework
 *****************************************************************************/

static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;
static const char *g_current_test = "";

#define TEST_CASE(name) static void name(void)

#define ASSERT_TRUE(cond)                                                      \
    do {                                                                       \
        if (!(cond))                                                           \
        {                                                                      \
            printf("  [FAIL] %s:%d  ASSERT_TRUE(" #cond ")\n",                 \
                   __FILE__, __LINE__);                                        \
            g_tests_failed++;                                                  \
            return;                                                            \
        }                                                                      \
    } while (0)

#define ASSERT_EQ(expected, actual)                                            \
    do {                                                                       \
        long _e = (long)(expected);                                            \
        long _a = (long)(actual);                                              \
        if (_e != _a)                                                          \
        {                                                                      \
            printf("  [FAIL] %s:%d  expected=%ld actual=%ld\n",                \
                   __FILE__, __LINE__, _e, _a);                                \
            g_tests_failed++;                                                  \
            return;                                                            \
        }                                                                      \
    } while (0)

#define ASSERT_MEM_EQ(p_expected, p_actual, len)                               \
    do {                                                                       \
        if (0 != memcmp((p_expected), (p_actual), (len)))                      \
        {                                                                      \
            printf("  [FAIL] %s:%d  memcmp mismatch over %u bytes\n",          \
                   __FILE__, __LINE__, (unsigned)(len));                       \
            g_tests_failed++;                                                  \
            return;                                                            \
        }                                                                      \
    } while (0)

#define RUN_TEST(fn)                                                           \
    do {                                                                       \
        g_current_test = #fn;                                                  \
        int _failed_before = g_tests_failed;                                   \
        printf("[RUN ] %s\n", #fn);                                            \
        fake_w25q64_reset();                                                   \
        g_tests_run++;                                                         \
        fn();                                                                  \
        if (g_tests_failed == _failed_before)                                  \
        {                                                                      \
            g_tests_passed++;                                                  \
            printf("[ OK ] %s\n", #fn);                                        \
        }                                                                      \
    } while (0)


/******************************************************************************
 *                       Fake W25Q64 SPI simulator
 *
 * Models the device as a CS-bracketed transaction state machine.
 * Each pf_spi_write_cs_pin transition resets / commits the current
 * transaction. Commands consume the first transmit byte; reads return
 * data based on the latched command + address.
 *****************************************************************************/

#define FAKE_FLASH_SIZE      W25Q64_MAX_SIZE   /* 8 MiB */

typedef enum
{
    FAKE_CS_IDLE = 0,
    FAKE_CS_ACTIVE,
} fake_cs_state_t;

static struct
{
    /* Storage. */
    uint8_t  flash[FAKE_FLASH_SIZE];

    /* Transaction state. */
    fake_cs_state_t cs_state;
    uint8_t         current_cmd;
    bool            cmd_latched;
    uint8_t         addr_bytes[3];
    uint32_t        addr_collected;     /* number of addr bytes received */
    uint32_t        addr_value;
    uint8_t         data_in[W25Q64_PAGE_SIZE * 2];
    uint32_t        data_in_len;

    /* Device flags. */
    bool     wel;                       /* write enable latch */
    bool     sleeping;
    uint32_t busy_polls_remaining;      /* simulates BUSY bit decay */

    /* Faulting / instrumentation hooks. */
    bool     force_jedec_mismatch;      /* return wrong JEDEC ID */
    bool     force_busy_forever;        /* never clear BUSY */
    bool     transmit_should_fail;      /* next transmit returns error */

    /* Counters for assertions. */
    uint32_t init_call_count;
    uint32_t deinit_call_count;
    uint32_t cs_low_count;
    uint32_t cs_high_count;
    uint32_t delay_call_count;
    uint32_t last_delay_ms;
} g_fake;

static void fake_w25q64_reset(void)
{
    memset(&g_fake, 0, sizeof(g_fake));
    memset(g_fake.flash, 0xFF, sizeof(g_fake.flash));
    g_fake.cs_state = FAKE_CS_IDLE;
}

/* Commit pending write/erase work when CS rises. */
static void fake_w25q64_commit_transaction(void)
{
    switch (g_fake.current_cmd)
    {
    case W25Q64_CMD_WRITE_ENABLE:
        g_fake.wel = true;
        break;

    case W25Q64_CMD_ERASE_SECTOR:
        if (g_fake.wel && g_fake.addr_collected == 3)
        {
            uint32_t base = g_fake.addr_value
                            & ~(W25Q64_SECTOR_SIZE - 1U);
            if (base + W25Q64_SECTOR_SIZE <= FAKE_FLASH_SIZE)
            {
                memset(&g_fake.flash[base], 0xFF, W25Q64_SECTOR_SIZE);
            }
            g_fake.busy_polls_remaining = 2;
            g_fake.wel = false;
        }
        break;

    case W25Q64_CMD_WRITE_DATA:
        if (g_fake.wel
            && g_fake.addr_collected == 3
            && g_fake.data_in_len > 0
            && g_fake.addr_value + g_fake.data_in_len <= FAKE_FLASH_SIZE)
        {
            /* Page Program is bitwise-AND (only 1->0). */
            for (uint32_t i = 0; i < g_fake.data_in_len; i++)
            {
                g_fake.flash[g_fake.addr_value + i] &= g_fake.data_in[i];
            }
            g_fake.busy_polls_remaining = 2;
            g_fake.wel = false;
        }
        break;

    case W25Q64_CMD_CHIP_ERASE:
        if (g_fake.wel)
        {
            memset(g_fake.flash, 0xFF, FAKE_FLASH_SIZE);
            g_fake.busy_polls_remaining = 2;
            g_fake.wel = false;
        }
        break;

    case W25Q64_CMD_SLEEP:
        g_fake.sleeping = true;
        break;

    case W25Q64_CMD_WAKEUP:
        g_fake.sleeping = false;
        break;

    default:
        break;
    }
}

/* SPI mock — CS pin handler. */
static w25q64_status_t mock_spi_write_cs_pin(uint8_t state)
{
    if (state == 0U)
    {
        g_fake.cs_low_count++;
        g_fake.cs_state       = FAKE_CS_ACTIVE;
        g_fake.cmd_latched    = false;
        g_fake.current_cmd    = 0;
        g_fake.addr_collected = 0;
        g_fake.addr_value     = 0;
        g_fake.data_in_len    = 0;
    }
    else
    {
        g_fake.cs_high_count++;
        if (g_fake.cs_state == FAKE_CS_ACTIVE)
        {
            fake_w25q64_commit_transaction();
        }
        g_fake.cs_state = FAKE_CS_IDLE;
    }
    return W25Q64_OK;
}

/* SPI mock — transmit. Drives the device state machine. */
static w25q64_status_t mock_spi_transmit(const uint8_t *p_data,
                                         uint32_t       data_length)
{
    if (g_fake.transmit_should_fail)
    {
        g_fake.transmit_should_fail = false;
        return W25Q64_ERROR;
    }
    if (p_data == NULL || data_length == 0
        || g_fake.cs_state != FAKE_CS_ACTIVE)
    {
        return W25Q64_ERROR;
    }

    uint32_t i = 0;
    if (!g_fake.cmd_latched)
    {
        g_fake.current_cmd = p_data[0];
        g_fake.cmd_latched = true;
        i = 1;
    }

    /* Commands that consume 3-byte address: 0x90, 0x03, 0x20, 0x02. */
    bool needs_addr =
        (g_fake.current_cmd == W25Q64_CMD_READ_ID)      ||
        (g_fake.current_cmd == W25Q64_CMD_READ_DATA)    ||
        (g_fake.current_cmd == W25Q64_CMD_ERASE_SECTOR) ||
        (g_fake.current_cmd == W25Q64_CMD_WRITE_DATA);

    while (i < data_length && needs_addr && g_fake.addr_collected < 3)
    {
        g_fake.addr_bytes[g_fake.addr_collected] = p_data[i];
        g_fake.addr_value =
            (g_fake.addr_value << 8) | p_data[i];
        g_fake.addr_collected++;
        i++;
    }

    /* Remaining bytes are data payload (Page Program). */
    while (i < data_length
           && g_fake.data_in_len < sizeof(g_fake.data_in))
    {
        g_fake.data_in[g_fake.data_in_len++] = p_data[i++];
    }

    return W25Q64_OK;
}

/* SPI mock — receive. Returns data based on latched command. */
static w25q64_status_t mock_spi_read(uint8_t *p_buffer,
                                     uint32_t buffer_length)
{
    if (p_buffer == NULL || buffer_length == 0
        || g_fake.cs_state != FAKE_CS_ACTIVE
        || !g_fake.cmd_latched)
    {
        return W25Q64_ERROR;
    }

    switch (g_fake.current_cmd)
    {
    case W25Q64_CMD_JEDEC_ID:
    {
        uint8_t id[3];
        if (g_fake.force_jedec_mismatch)
        {
            id[0] = 0xAA; id[1] = 0xBB; id[2] = 0xCC;
        }
        else
        {
            id[0] = 0xEF; id[1] = 0x40; id[2] = 0x17;
        }
        for (uint32_t i = 0; i < buffer_length; i++)
        {
            p_buffer[i] = id[i % 3];
        }
        return W25Q64_OK;
    }

    case W25Q64_CMD_READ_ID:
    {
        uint8_t id[2] = { 0xEF, 0x16 };
        for (uint32_t i = 0; i < buffer_length; i++)
        {
            p_buffer[i] = id[i % 2];
        }
        return W25Q64_OK;
    }

    case W25Q64_CMD_READ_REG:
    {
        uint8_t status = 0x00;
        if (g_fake.force_busy_forever)
        {
            status = W25Q64_STATUS_BUSY;
        }
        else if (g_fake.busy_polls_remaining > 0)
        {
            status = W25Q64_STATUS_BUSY;
            g_fake.busy_polls_remaining--;
        }
        for (uint32_t i = 0; i < buffer_length; i++)
        {
            p_buffer[i] = status;
        }
        return W25Q64_OK;
    }

    case W25Q64_CMD_READ_DATA:
    {
        uint32_t base = g_fake.addr_value;
        for (uint32_t i = 0; i < buffer_length; i++)
        {
            uint32_t a = base + i;
            p_buffer[i] = (a < FAKE_FLASH_SIZE) ? g_fake.flash[a] : 0xFF;
        }
        return W25Q64_OK;
    }

    default:
        memset(p_buffer, 0, buffer_length);
        return W25Q64_OK;
    }
}

static w25q64_status_t mock_spi_init(void)
{
    g_fake.init_call_count++;
    return W25Q64_OK;
}

static w25q64_status_t mock_spi_deinit(void)
{
    g_fake.deinit_call_count++;
    return W25Q64_OK;
}

static w25q64_status_t mock_spi_transmit_dma(const uint8_t *p_data,
                                             uint32_t       data_length)
{
    (void)p_data; (void)data_length;
    return W25Q64_OK;
}

static w25q64_status_t mock_spi_wait_dma_complete(uint32_t timeout_ms)
{
    (void)timeout_ms;
    return W25Q64_OK;
}

static w25q64_status_t mock_spi_write_dc_pin(uint8_t state)
{
    (void)state;
    return W25Q64_OK;
}

/* Timebase mock. */
static uint32_t g_mock_tick_ms = 0;
static uint32_t mock_get_tick_ms(void) { return g_mock_tick_ms; }
static void mock_delay_ms(uint32_t ms)
{
    g_fake.delay_call_count++;
    g_fake.last_delay_ms = ms;
    g_mock_tick_ms += ms;
}

/* OS delay mock. */
static void mock_os_delay_ms(uint32_t ms)
{
    g_mock_tick_ms += ms;
}

/* Build fully populated vtables. */
static w25q64_spi_interface_t make_spi_vtable(void)
{
    w25q64_spi_interface_t v = {
        .pf_spi_init              = mock_spi_init,
        .pf_spi_deinit            = mock_spi_deinit,
        .pf_spi_transmit          = mock_spi_transmit,
        .pf_spi_read              = mock_spi_read,
        .pf_spi_transmit_dma      = mock_spi_transmit_dma,
        .pf_spi_wait_dma_complete = mock_spi_wait_dma_complete,
        .pf_spi_write_cs_pin      = mock_spi_write_cs_pin,
        .pf_spi_write_dc_pin      = mock_spi_write_dc_pin,
    };
    return v;
}

static w25q64_timebase_interface_t make_tb_vtable(void)
{
    w25q64_timebase_interface_t v = {
        .pf_get_tick_ms = mock_get_tick_ms,
        .pf_delay_ms    = mock_delay_ms,
    };
    return v;
}

static w25q64_os_delay_t make_os_vtable(void)
{
    w25q64_os_delay_t v = { .pf_os_delay_ms = mock_os_delay_ms };
    return v;
}

/* Helper: instantiate + run init successfully. */
static void make_initialized_driver(bsp_w25q64_driver_t        *drv,
                                    w25q64_spi_interface_t      *spi,
                                    w25q64_timebase_interface_t *tb,
                                    w25q64_os_delay_t           *os)
{
    *spi = make_spi_vtable();
    *tb  = make_tb_vtable();
    *os  = make_os_vtable();
    memset(drv, 0, sizeof(*drv));
    w25q64_status_t r = w25q64_driver_inst(drv, spi, tb, os);
    (void)r;
}


/******************************************************************************
 *                              Test cases
 *****************************************************************************/

/* ------- w25q64_driver_inst ------- */

TEST_CASE(test_inst_null_args_returns_param_error)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi = make_spi_vtable();
    w25q64_timebase_interface_t tb  = make_tb_vtable();
    w25q64_os_delay_t           os  = make_os_vtable();

    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              w25q64_driver_inst(NULL, &spi, &tb, &os));
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              w25q64_driver_inst(&drv, NULL, &tb, &os));
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              w25q64_driver_inst(&drv, &spi, NULL, &os));
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              w25q64_driver_inst(&drv, &spi, &tb, NULL));
}

TEST_CASE(test_inst_missing_spi_callback_returns_resource_error)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi = make_spi_vtable();
    w25q64_timebase_interface_t tb  = make_tb_vtable();
    w25q64_os_delay_t           os  = make_os_vtable();

    spi.pf_spi_transmit = NULL; /* required slot missing */

    ASSERT_EQ(W25Q64_ERRORRESOURCE,
              w25q64_driver_inst(&drv, &spi, &tb, &os));
}

TEST_CASE(test_inst_missing_timebase_callback_returns_resource_error)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi = make_spi_vtable();
    w25q64_timebase_interface_t tb  = make_tb_vtable();
    w25q64_os_delay_t           os  = make_os_vtable();

    tb.pf_delay_ms = NULL;

    ASSERT_EQ(W25Q64_ERRORRESOURCE,
              w25q64_driver_inst(&drv, &spi, &tb, &os));
}

TEST_CASE(test_inst_success_binds_vtables_and_api)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi = make_spi_vtable();
    w25q64_timebase_interface_t tb  = make_tb_vtable();
    w25q64_os_delay_t           os  = make_os_vtable();
    memset(&drv, 0, sizeof(drv));

    ASSERT_EQ(W25Q64_OK,
              w25q64_driver_inst(&drv, &spi, &tb, &os));

    ASSERT_TRUE(drv.p_spi_interface       == &spi);
    ASSERT_TRUE(drv.p_timebase_interface  == &tb);
    ASSERT_TRUE(drv.p_os_interface        == &os);
    ASSERT_TRUE(drv.pf_w25q64_init        != NULL);
    ASSERT_TRUE(drv.pf_w25q64_deinit      != NULL);
    ASSERT_TRUE(drv.pf_w25q64_read_id     != NULL);
    ASSERT_TRUE(drv.pf_w25q64_read_data   != NULL);
    ASSERT_TRUE(drv.pf_w25q64_write_data_noerase != NULL);
    ASSERT_TRUE(drv.pf_w25q64_write_data_erase   != NULL);
    ASSERT_TRUE(drv.pf_w25q64_erase_chip  != NULL);
    ASSERT_TRUE(drv.pf_w25q64_sleep       != NULL);
    ASSERT_TRUE(drv.pf_w25q64_wakeup      != NULL);
}

/* ------- pf_w25q64_init ------- */

TEST_CASE(test_init_success_runs_power_on_sequence)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    ASSERT_EQ(W25Q64_OK, drv.pf_w25q64_init(&drv));
    ASSERT_EQ(1, g_fake.init_call_count);     /* spi_init called once */
    ASSERT_TRUE(g_fake.delay_call_count >= 1);/* power-on delay */
}

TEST_CASE(test_init_jedec_mismatch_returns_resource_error)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    g_fake.force_jedec_mismatch = true;
    ASSERT_EQ(W25Q64_ERRORRESOURCE, drv.pf_w25q64_init(&drv));
}

TEST_CASE(test_init_null_driver_returns_param_error)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    ASSERT_EQ(W25Q64_ERRORPARAMETER, drv.pf_w25q64_init(NULL));
}

/* ------- pf_w25q64_read_id ------- */

TEST_CASE(test_read_id_invalid_param)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    uint8_t buf[2];
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_read_id(NULL, buf, 2));
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_read_id(&drv, NULL, 2));
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_read_id(&drv, buf, 1));
}

TEST_CASE(test_read_id_returns_manufacturer_and_device)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    uint8_t buf[2] = {0};
    ASSERT_EQ(W25Q64_OK, drv.pf_w25q64_read_id(&drv, buf, 2));
    ASSERT_EQ(0xEF, buf[0]);
    ASSERT_EQ(0x16, buf[1]);
}

/* ------- pf_w25q64_read_data ------- */

TEST_CASE(test_read_data_invalid_param)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    uint8_t buf[16];
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_read_data(NULL, 0, buf, 16));
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_read_data(&drv, 0, NULL, 16));
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_read_data(&drv, 0, buf, 0));
}

TEST_CASE(test_read_data_address_out_of_range)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    uint8_t buf[16];
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_read_data(&drv, W25Q64_MAX_SIZE, buf, 1));
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_read_data(&drv, W25Q64_MAX_SIZE - 8U, buf, 16));
}

TEST_CASE(test_read_data_returns_underlying_flash)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    /* Seed fake flash directly. */
    for (int i = 0; i < 32; i++)
    {
        g_fake.flash[0x1000 + i] = (uint8_t)(0xA0 + i);
    }

    uint8_t buf[32] = {0};
    ASSERT_EQ(W25Q64_OK,
              drv.pf_w25q64_read_data(&drv, 0x1000, buf, 32));
    for (int i = 0; i < 32; i++)
    {
        ASSERT_EQ((uint8_t)(0xA0 + i), buf[i]);
    }
}

/* ------- pf_w25q64_write_data_noerase ------- */

TEST_CASE(test_write_noerase_invalid_param)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    uint8_t data[4] = {1, 2, 3, 4};
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_write_data_noerase(NULL, 0, data, 4));
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_write_data_noerase(&drv, 0, NULL, 4));
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_write_data_noerase(&drv, 0, data, 0));
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_write_data_noerase(&drv,
                                               W25Q64_MAX_SIZE, data, 1));
}

TEST_CASE(test_write_noerase_persists_to_fake_flash)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    uint8_t pattern[16];
    for (int i = 0; i < 16; i++) pattern[i] = (uint8_t)(0x10 + i);

    /* Pre-erased region (0xFF) is required for Page Program semantics. */
    ASSERT_EQ(W25Q64_OK,
              drv.pf_w25q64_write_data_noerase(&drv, 0x2000,
                                               pattern, 16));
    ASSERT_MEM_EQ(pattern, &g_fake.flash[0x2000], 16);
}

/* ------- pf_w25q64_write_data_erase ------- */

TEST_CASE(test_write_erase_invalid_param)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    uint8_t data[1] = {0xAA};
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_write_data_erase(NULL, 0, data, 1));
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_write_data_erase(&drv, 0, NULL, 1));
    ASSERT_EQ(W25Q64_ERRORPARAMETER,
              drv.pf_w25q64_write_data_erase(&drv, 0, data, 0));
}

TEST_CASE(test_write_erase_overwrites_dirty_data_within_one_sector)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    /* Dirty the destination so a noerase write would fail to set 1s. */
    memset(&g_fake.flash[0x3000], 0x00, 32);

    uint8_t pattern[16];
    for (int i = 0; i < 16; i++) pattern[i] = (uint8_t)(0xC0 | i);

    ASSERT_EQ(W25Q64_OK,
              drv.pf_w25q64_write_data_erase(&drv, 0x3000, pattern, 16));

    ASSERT_MEM_EQ(pattern, &g_fake.flash[0x3000], 16);
    /* Trailing bytes in the same sector are now erased to 0xFF. */
    for (int i = 16; i < 32; i++)
    {
        ASSERT_EQ(0xFF, g_fake.flash[0x3000 + i]);
    }
}

TEST_CASE(test_write_erase_spans_two_sectors)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    const uint32_t addr = W25Q64_SECTOR_SIZE - 8U; /* end of sector 0 */
    uint8_t pattern[16];
    for (int i = 0; i < 16; i++) pattern[i] = (uint8_t)(0x80 + i);

    ASSERT_EQ(W25Q64_OK,
              drv.pf_w25q64_write_data_erase(&drv, addr, pattern, 16));
    ASSERT_MEM_EQ(pattern, &g_fake.flash[addr], 16);
}

/* ------- pf_w25q64_erase_chip ------- */

TEST_CASE(test_erase_chip_null_param)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    ASSERT_EQ(W25Q64_ERRORPARAMETER, drv.pf_w25q64_erase_chip(NULL));
}

TEST_CASE(test_erase_chip_wipes_to_0xff)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    g_fake.flash[0]                      = 0x00;
    g_fake.flash[FAKE_FLASH_SIZE / 2]    = 0x55;
    g_fake.flash[FAKE_FLASH_SIZE - 1]    = 0xAA;

    ASSERT_EQ(W25Q64_OK, drv.pf_w25q64_erase_chip(&drv));

    ASSERT_EQ(0xFF, g_fake.flash[0]);
    ASSERT_EQ(0xFF, g_fake.flash[FAKE_FLASH_SIZE / 2]);
    ASSERT_EQ(0xFF, g_fake.flash[FAKE_FLASH_SIZE - 1]);
}

TEST_CASE(test_busy_forever_returns_timeout)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    g_fake.force_busy_forever = true;
    ASSERT_EQ(W25Q64_ERRORTIMEOUT, drv.pf_w25q64_erase_chip(&drv));
}

/* ------- pf_w25q64_sleep / pf_w25q64_wakeup ------- */

TEST_CASE(test_sleep_null_param)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    ASSERT_EQ(W25Q64_ERRORPARAMETER, drv.pf_w25q64_sleep(NULL));
}

TEST_CASE(test_sleep_then_wakeup_round_trip)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    ASSERT_EQ(W25Q64_OK, drv.pf_w25q64_sleep(&drv));
    ASSERT_TRUE(g_fake.sleeping);

    uint32_t delays_before = g_fake.delay_call_count;
    ASSERT_EQ(W25Q64_OK, drv.pf_w25q64_wakeup(&drv));
    ASSERT_TRUE(!g_fake.sleeping);
    ASSERT_TRUE(g_fake.delay_call_count > delays_before);
}

TEST_CASE(test_wakeup_null_param)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    ASSERT_EQ(W25Q64_ERRORPARAMETER, drv.pf_w25q64_wakeup(NULL));
}

/* ------- pf_w25q64_deinit ------- */

TEST_CASE(test_deinit_releases_bus)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    ASSERT_EQ(W25Q64_OK, drv.pf_w25q64_deinit(&drv));
    ASSERT_EQ(1, g_fake.deinit_call_count);
    ASSERT_EQ(W25Q64_ERRORPARAMETER, drv.pf_w25q64_deinit(NULL));
}

/* ------- end-to-end ------- */

TEST_CASE(test_end_to_end_write_then_readback)
{
    bsp_w25q64_driver_t drv;
    w25q64_spi_interface_t      spi;
    w25q64_timebase_interface_t tb;
    w25q64_os_delay_t           os;
    make_initialized_driver(&drv, &spi, &tb, &os);

    ASSERT_EQ(W25Q64_OK, drv.pf_w25q64_init(&drv));

    uint8_t tx[64];
    for (int i = 0; i < 64; i++) tx[i] = (uint8_t)(i ^ 0x5A);

    ASSERT_EQ(W25Q64_OK,
              drv.pf_w25q64_write_data_erase(&drv, 0x4000, tx, 64));

    uint8_t rx[64] = {0};
    ASSERT_EQ(W25Q64_OK,
              drv.pf_w25q64_read_data(&drv, 0x4000, rx, 64));
    ASSERT_MEM_EQ(tx, rx, 64);
}


/******************************************************************************
 *                                main
 *****************************************************************************/

int main(void)
{
    printf("== W25Q64 driver mock test suite ==\n\n");

    /* w25q64_driver_inst */
    RUN_TEST(test_inst_null_args_returns_param_error);
    RUN_TEST(test_inst_missing_spi_callback_returns_resource_error);
    RUN_TEST(test_inst_missing_timebase_callback_returns_resource_error);
    RUN_TEST(test_inst_success_binds_vtables_and_api);

    /* init */
    RUN_TEST(test_init_success_runs_power_on_sequence);
    RUN_TEST(test_init_jedec_mismatch_returns_resource_error);
    RUN_TEST(test_init_null_driver_returns_param_error);

    /* read_id */
    RUN_TEST(test_read_id_invalid_param);
    RUN_TEST(test_read_id_returns_manufacturer_and_device);

    /* read_data */
    RUN_TEST(test_read_data_invalid_param);
    RUN_TEST(test_read_data_address_out_of_range);
    RUN_TEST(test_read_data_returns_underlying_flash);

    /* write_data_noerase */
    RUN_TEST(test_write_noerase_invalid_param);
    RUN_TEST(test_write_noerase_persists_to_fake_flash);

    /* write_data_erase */
    RUN_TEST(test_write_erase_invalid_param);
    RUN_TEST(test_write_erase_overwrites_dirty_data_within_one_sector);
    RUN_TEST(test_write_erase_spans_two_sectors);

    /* erase_chip */
    RUN_TEST(test_erase_chip_null_param);
    RUN_TEST(test_erase_chip_wipes_to_0xff);
    RUN_TEST(test_busy_forever_returns_timeout);

    /* sleep / wakeup */
    RUN_TEST(test_sleep_null_param);
    RUN_TEST(test_sleep_then_wakeup_round_trip);
    RUN_TEST(test_wakeup_null_param);

    /* deinit */
    RUN_TEST(test_deinit_releases_bus);

    /* end-to-end */
    RUN_TEST(test_end_to_end_write_then_readback);

    printf("\n== summary ==\n");
    printf("  run    : %d\n", g_tests_run);
    printf("  passed : %d\n", g_tests_passed);
    printf("  failed : %d\n", g_tests_failed);

    return (g_tests_failed == 0) ? 0 : 1;
}
