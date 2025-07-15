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

#if defined(_WIN32)

#include "CNIOWindows.h"

#include <assert.h>

int CNIOWindows_sendmmsg(SOCKET s, CNIOWindows_mmsghdr *msgvec, unsigned int vlen,
                         int flags) {
  assert(!"sendmmsg not implemented");
  abort();
}

int CNIOWindows_recvmmsg(SOCKET s, CNIOWindows_mmsghdr *msgvec,
                         unsigned int vlen, int flags,
                         struct timespec *timeout) {
  assert(!"recvmmsg not implemented");
  abort();
}

const void *CNIOWindows_CMSG_DATA(const WSACMSGHDR *pcmsg) {
  return WSA_CMSG_DATA(pcmsg);
}

void *CNIOWindows_CMSG_DATA_MUTABLE(LPWSACMSGHDR pcmsg) {
  return WSA_CMSG_DATA(pcmsg);
}

WSACMSGHDR *CNIOWindows_CMSG_FIRSTHDR(const WSAMSG *msg) {
  return WSA_CMSG_FIRSTHDR(msg);
}

WSACMSGHDR *CNIOWindows_CMSG_NXTHDR(const WSAMSG *msg, LPWSACMSGHDR cmsg) {
  return WSA_CMSG_NXTHDR(msg, cmsg);
}

size_t CNIOWindows_CMSG_LEN(size_t length) {
  return WSA_CMSG_LEN(length);
}

size_t CNIOWindows_CMSG_SPACE(size_t length) {
  return WSA_CMSG_SPACE(length);
}

#endif
