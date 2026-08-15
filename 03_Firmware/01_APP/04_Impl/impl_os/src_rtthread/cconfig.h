/******************************************************************************
 * @file cconfig.h
 *
 * @brief Toolchain capability description for the RT-Thread kernel.
 *
 * @note RT-Thread normally generates this by probing the compiler. The copy
 *       shipped with the 411test reference project describes newlib 2.4.0 /
 *       GCC 5.4.1, which is what RT-Thread 4.0.3 was released against.
 *
 *       This project builds with arm-none-eabi-gcc 15.x, whose newlib already
 *       defines union sigval, struct sigevent and siginfo_t in <sys/signal.h>.
 *       rt-thread/inc/libc/libc_signal.h defines them too, guarded by the
 *       HAVE_SIG* macros below, and rtdef.h includes rtlibc.h -> libc_signal.h
 *       unconditionally. Without these three macros every kernel translation
 *       unit fails with "redefinition of 'union sigval'".
 *
 *       Declaring them is the whole fix: RT-Thread 4.0.3 then builds cleanly
 *       on a modern toolchain, and the GCC 5.4.1 requirement in the transplant
 *       notes does not apply to this project.
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef CCONFIG_H__
#define CCONFIG_H__

#define HAVE_NEWLIB_H       1
#define LIBC_VERSION        "newlib 4.x"

#define HAVE_SYS_SIGNAL_H   1
#define HAVE_SYS_SELECT_H   1

#define HAVE_FDSET          1
#define HAVE_SIGACTION      1

/* The three that matter: newlib supplies these types, so libc_signal.h must
 * not define its own. */
#define HAVE_SIGVAL         1
#define HAVE_SIGEVENT       1
#define HAVE_SIGINFO        1

#define STDC                "2011"

#endif /* CCONFIG_H__ */
