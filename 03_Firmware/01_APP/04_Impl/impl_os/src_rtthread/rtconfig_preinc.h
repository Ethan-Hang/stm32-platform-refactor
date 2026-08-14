/******************************************************************************
 * @file rtconfig_preinc.h
 *
 * @brief Force-included before every RT-Thread translation unit.
 *
 * @note Nothing #includes this file. cmake/os_kernel.cmake passes it via
 *       -include, which is how __RTTHREAD__ and HAVE_CCONFIG_H reach the
 *       kernel sources. HAVE_CCONFIG_H is what makes libc_signal.h consult
 *       cconfig.h; drop it and the newlib type collision comes back.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef RTCONFIG_PREINC_H__
#define RTCONFIG_PREINC_H__

#define HAVE_CCONFIG_H
#define __RTTHREAD__

#endif /* RTCONFIG_PREINC_H__ */
