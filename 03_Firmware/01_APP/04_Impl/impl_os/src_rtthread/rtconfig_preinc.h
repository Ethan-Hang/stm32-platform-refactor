/******************************************************************************
 * @file rtconfig_preinc.h
 *
 * @brief Force-included before every RT-Thread translation unit.
 *
 * @note Nothing #includes this file. 07_Toolchain/os_kernel.cmake passes it via
 *       -include, which is how __RTTHREAD__ and HAVE_CCONFIG_H reach the
 *       kernel sources. HAVE_CCONFIG_H is what makes libc_signal.h consult
 *       cconfig.h; drop it and the newlib type collision comes back.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef RTCONFIG_PREINC_H__
#define RTCONFIG_PREINC_H__

/* Guarded because 07_Toolchain/os_kernel.cmake also defines HAVE_CCONFIG_H on the
 * firmware target: project files that include <rtthread.h> directly need it
 * too, and those get both this force-include and the -D. */
#ifndef HAVE_CCONFIG_H
#define HAVE_CCONFIG_H
#endif

#define __RTTHREAD__

#endif /* RTCONFIG_PREINC_H__ */
