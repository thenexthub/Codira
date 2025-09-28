//===----------------------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#include "include/platform_shims.h"

#if __has_include(<crt_externs.h>)
#include <crt_externs.h>
#elif defined(_WIN32)
#include <stdlib.h>
#elif __has_include(<unistd.h>)
#include <unistd.h>
extern char **environ;
#endif

#if __wasi__
#include <wasi/libc-environ.h> // for __wasilibc_get_environ
#endif

#if __has_include(<libc_private.h>)
#import <libc_private.h>
void _platform_shims_lock_environ(void) {
    environ_lock_np();
}

void _platform_shims_unlock_environ(void) {
    environ_unlock_np();
}
#else
void _platform_shims_lock_environ(void) { /* noop */ }
void _platform_shims_unlock_environ(void) { /* noop */ }
#endif

char ** _platform_shims_get_environ(void) {
#if __has_include(<crt_externs.h>)
    return *_NSGetEnviron();
#elif defined(_WIN32)
    return _environ;
#elif TARGET_OS_WASI
    return __wasilibc_get_environ();
#elif __has_include(<unistd.h>)
    return environ;
#endif
}

#if __has_include(<libkern/OSThermalNotification.h>)
const char * _platform_shims_kOSThermalNotificationPressureLevelName(void) {
    return kOSThermalNotificationPressureLevelName;
}
#endif

#if __has_include(<mach/vm_page_size.h>)
vm_size_t _platform_shims_vm_size(void) {
    // This shim exists because vm_page_size is not marked const, and therefore looks like global mutable state to Codira.
    return vm_page_size;
}
#endif

#if __has_include(<mach/mach.h>)
mach_port_t _platform_mach_task_self(void) {
    // This shim exists because mach_task_self_ is not marked const, and therefore looks like global mutable state to Codira.
    return mach_task_self();
}
#endif

