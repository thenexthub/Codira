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

// Support functions for liburing

// Check if this is needed, copied from shim.c to avoid possible problems due to:
// Xcode's Archive builds with Xcode's Package support struggle with empty .c files
// (https://bugs.swift.org/browse/SR-12939).
void CNIOLinux_i_do_nothing_just_working_around_a_darwin_toolchain_bug2(void) {}

#ifdef __linux__

#ifdef SWIFTNIO_USE_IO_URING

#define _GNU_SOURCE
#include <CNIOLinux.h>
#include <signal.h>
#include <sys/poll.h>

void CNIOLinux_io_uring_set_link_flag(struct io_uring_sqe *sqe)
{
    sqe->flags |= IOSQE_IO_LINK;
    return;
}

unsigned int CNIOLinux_POLLRDHUP()
{
    return POLLRDHUP;
}

#endif

#endif
