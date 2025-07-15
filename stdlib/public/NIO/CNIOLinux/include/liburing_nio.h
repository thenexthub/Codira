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

#ifndef LIBURING_NIO_H
#define LIBURING_NIO_H

#ifdef __linux__

#ifdef SWIFTNIO_USE_IO_URING

#if __has_include(<liburing.h>)
#include <liburing.h>
#else
#error "SWIFTNIO_USE_IO_URING specified but liburing.h not available. You need to install https://github.com/axboe/liburing."
#endif

// OR in the IOSQE_IO_LINK flag, couldn't access the define from Swift
void CNIOLinux_io_uring_set_link_flag(struct io_uring_sqe *sqe);

// No way I managed to get this even when defining _GNU_SOURCE properly. Argh.
unsigned int CNIOLinux_POLLRDHUP();

#endif

#endif /* __linux__ */

#endif /* LIBURING_NIO_H */
