#include <stdio.h>
#include <string.h>

#include "platform_device.h"
#include "platform_object.h"

static UINT32_T s_start_calls;
static UINT32_T s_stop_calls;
static UINT32_T s_sleep_calls;

static platform_err_t test_start(void *p_self)
{
    (void)p_self;
    s_start_calls++;
    return PLATFORM_OK;
}

static platform_err_t test_stop(void *p_self)
{
    (void)p_self;
    s_stop_calls++;
    return PLATFORM_OK;
}

static platform_err_t test_sleep(void *p_self)
{
    (void)p_self;
    s_sleep_calls++;
    return PLATFORM_OK;
}

static int expect_true(BOOL_T condition, const char *p_message)
{
    if (!condition)
    {
        (void)fprintf(stderr, "FAIL: %s\n", p_message);
        return 1;
    }

    return 0;
}

static int test_device_init_stores_metadata(void)
{
    platform_device_t device;
    platform_err_t ret;

    (void)memset(&device, 0xA5, sizeof(device));
    ret = platform_device_init(&device,
                               "display",
                               PLATFORM_DEVICE_CLASS_DISPLAY,
                               PLATFORM_DEVICE_CAP_READ |
                               PLATFORM_DEVICE_CAP_WRITE,
                               NULL,
                               NULL);

    return expect_true((PLATFORM_OK == ret) &&
                       (PLATFORM_DEVICE_CLASS_DISPLAY == device.device_class) &&
                       ((PLATFORM_DEVICE_CAP_READ |
                         PLATFORM_DEVICE_CAP_WRITE) == device.cap),
                       "platform_device_init must store class and capabilities");
}

static int test_stop_invokes_callback(void)
{
    platform_object_t object;
    platform_lifecycle_ops_t lifecycle = {0};
    platform_err_t ret;

    lifecycle.stop = test_stop;
    s_stop_calls = 0u;
    (void)platform_object_init(&object, "object", PLATFORM_OBJECT_DEVICE,
                               NULL, NULL, &lifecycle);
    (void)platform_object_set_state(&object, PLATFORM_OBJECT_STARTED);

    ret = platform_object_lifecycle_stop(&object);

    return expect_true((PLATFORM_OK == ret) &&
                       (1u == s_stop_calls) &&
                       (PLATFORM_OBJECT_STOPPED == object.state),
                       "stop must invoke callback and enter stopped state");
}

static int test_sleep_invokes_callback(void)
{
    platform_object_t object;
    platform_lifecycle_ops_t lifecycle = {0};
    platform_err_t ret;

    lifecycle.sleep = test_sleep;
    s_sleep_calls = 0u;
    (void)platform_object_init(&object, "object", PLATFORM_OBJECT_DEVICE,
                               NULL, NULL, &lifecycle);
    (void)platform_object_set_state(&object, PLATFORM_OBJECT_STARTED);

    ret = platform_object_lifecycle_sleep(&object);

    return expect_true((PLATFORM_OK == ret) && (1u == s_sleep_calls),
                       "sleep must invoke callback and return its status");
}

static int test_start_rejects_uninitialized_object(void)
{
    platform_object_t object;
    platform_lifecycle_ops_t lifecycle = {0};
    platform_err_t ret;

    lifecycle.start = test_start;
    s_start_calls = 0u;
    (void)platform_object_init(&object, "object", PLATFORM_OBJECT_DEVICE,
                               NULL, NULL, &lifecycle);

    ret = platform_object_lifecycle_start(&object);

    return expect_true((PLATFORM_OK != ret) &&
                       (0u == s_start_calls) &&
                       (PLATFORM_OBJECT_CREATED == object.state),
                       "start must reject an object that was never initialized");
}

int main(void)
{
    int failures = 0;

    failures += test_device_init_stores_metadata();
    failures += test_stop_invokes_callback();
    failures += test_sleep_invokes_callback();
    failures += test_start_rejects_uninitialized_object();

    if (0 == failures)
    {
        (void)printf("platform_common regression tests passed\n");
    }

    return failures;
}
