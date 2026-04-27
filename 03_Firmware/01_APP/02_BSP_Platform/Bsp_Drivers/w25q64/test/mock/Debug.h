/******************************************************************************
 * @file    Debug.h (mock)
 * @brief   Host-side stub of the firmware Debug.h, used only by the
 *          w25q64 mock test harness. Routes DEBUG_OUT to stderr when
 *          W25Q64_TEST_VERBOSE is defined, and silences it otherwise.
 *****************************************************************************/

#pragma once
#ifndef __DEBUG_H_TEST_STUB__
#define __DEBUG_H_TEST_STUB__

#include <stdio.h>

#ifdef W25Q64_TEST_VERBOSE
#define DEBUG_OUT(LEVEL, TAG, fmt, ...)                                        \
    do {                                                                       \
        fprintf(stderr, "[%s][%s] " fmt "\n", #LEVEL, TAG, ##__VA_ARGS__);     \
    } while (0)
#else
#define DEBUG_OUT(LEVEL, TAG, fmt, ...) ((void)0)
#endif

#endif /* __DEBUG_H_TEST_STUB__ */
