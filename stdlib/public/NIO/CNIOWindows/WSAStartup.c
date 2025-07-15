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

#include <Winsock2.h>

#include <stdlib.h>

#pragma section(".CRT$XCU", read)

static void __cdecl
NIOWSAStartup(void) {
    WSADATA wsa;
    WORD wVersionRequested = MAKEWORD(2, 2);
    if (!WSAStartup(wVersionRequested, &wsa)) {
        _exit(EXIT_FAILURE);
    }
}

__declspec(allocate(".CRT$XCU"))
static void (*pNIOWSAStartup)(void) = &NIOWSAStartup;

#endif
