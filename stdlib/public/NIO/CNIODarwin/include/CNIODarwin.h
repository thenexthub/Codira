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
#ifndef C_NIO_DARWIN_H
#define C_NIO_DARWIN_H

#ifdef __APPLE__
#include <sys/socket.h>
#include <sys/vsock.h>
#include <sys/ioctl.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>

#ifdef __cplusplus
extern "C" {
#endif

// Darwin platforms do not have a sendmmsg implementation available to them. This C module
// provides a shim that implements sendmmsg on top of sendmsg. It also provides a shim for
// recvmmsg, but does not actually implement that shim, instantly throwing errors if called.
//
// On Linux sendmmsg will error immediately in many cases if it knows any of the messages
// cannot be sent. This is not something we can easily achieve from userspace on Darwin,
// so instead if we encounter an error on any message but the first we will return "short".

typedef struct {
    struct msghdr msg_hdr;
    unsigned int msg_len;
} CNIODarwin_mmsghdr;

extern const int CNIODarwin_IPTOS_ECN_NOTECT;
extern const int CNIODarwin_IPTOS_ECN_MASK;
extern const int CNIODarwin_IPTOS_ECN_ECT0;
extern const int CNIODarwin_IPTOS_ECN_ECT1;
extern const int CNIODarwin_IPTOS_ECN_CE;
extern const int CNIODarwin_IPV6_RECVPKTINFO;
extern const int CNIODarwin_IPV6_PKTINFO;

int CNIODarwin_sendmmsg(int sockfd, CNIODarwin_mmsghdr *msgvec, unsigned int vlen, int flags);
int CNIODarwin_recvmmsg(int sockfd, CNIODarwin_mmsghdr *msgvec, unsigned int vlen, int flags, struct timespec *timeout);

// cmsghdr handling
struct cmsghdr *CNIODarwin_CMSG_FIRSTHDR(const struct msghdr *);
struct cmsghdr *CNIODarwin_CMSG_NXTHDR(const struct msghdr *, const struct cmsghdr *);
const void *CNIODarwin_CMSG_DATA(const struct cmsghdr *);
void *CNIODarwin_CMSG_DATA_MUTABLE(struct cmsghdr *);
size_t CNIODarwin_CMSG_LEN(size_t);
size_t CNIODarwin_CMSG_SPACE(size_t);

extern const unsigned long CNIODarwin_IOCTL_VM_SOCKETS_GET_LOCAL_CID;

const char* CNIODarwin_dirent_dname(struct dirent* ent);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  // __APPLE__
#endif  // C_NIO_DARWIN_H
